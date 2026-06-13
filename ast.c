#include "kek_internal.h"

const char* AstNodeTypeNames[] = {
    "File",
    "Statement",
    "Block",
    "Group",
    "Index",
    "Generic",
    "Token",
};

static struct AstNode* CreateAstNode(struct Parser* parser, enum AstNodeType type, struct SourceLocation location) {
    if (parser->astNodeCount >= parser->astNodeCapacity) {
        KekAddDiagnostic(parser->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_PARSE,
            parser->file ? parser->file->fileIndex : -1, location, "AST node storage capacity exceeded");
        parser->errorCount++;
        if (parser->astNodeCapacity > 0) {
            return &parser->astNodes[parser->astNodeCapacity - 1];
        }
        return NULL;
    }

    struct AstNode* node = &parser->astNodes[parser->astNodeCount++];
    memset(node, 0, sizeof(*node));
    node->type = type;
    node->location = location;
    return node;
}

static void AddChild(struct AstNode* parent, struct AstNode* child) {
    child->nextSibling = NULL;
    if (parent->lastChild) {
        parent->lastChild->nextSibling = child;
    } else {
        parent->firstChild = child;
    }
    parent->lastChild = child;
    parent->childCount++;
}

static int IsPunctuationToken(struct Token* token, enum PunctuationType punctuation) {
    return token->type == TOKEN_PUNCTUATION && token->value.punctuation == punctuation;
}

static int IsOperatorToken(struct Token* token, enum OperatorType operator) {
    return token->type == TOKEN_OPERATOR && token->value.operator == operator;
}

static int TokenTextEquals(struct Parser* parser, struct Token* token, const char* text) {
    size_t length = strlen(text);
    return (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_NUMBER || token->type == TOKEN_STRING)
        && token->location.length == length
        && strncmp(parser->file->content + token->location.offset, text, length) == 0;
}

static int IsClosingPunctuation(struct Token* token) {
    return IsPunctuationToken(token, PUNCTUATION_RIGHT_PAREN)
        || IsPunctuationToken(token, PUNCTUATION_RIGHT_BRACE)
        || IsPunctuationToken(token, PUNCTUATION_RIGHT_BRACKET);
}

static int IsTriviaToken(struct Token* token) {
    return token->type == TOKEN_COMMENT || token->type == TOKEN_DOC_COMMENT;
}

static const char* PunctuationName(enum PunctuationType punctuation) {
    return punctuation < PUNCTUATION_COUNT ? PunctuationNames[punctuation] : "<end of file>";
}

static void ReportParseError(struct Parser* parser, struct Token* token, const char* message) {
    KekAddDiagnostic(parser->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_PARSE,
        parser->file ? parser->file->fileIndex : -1, token->location, message);
    parser->errorCount++;
}

static int IsAstTerminator(struct Token* token, enum PunctuationType closePunctuation) {
    if (token->type == TOKEN_EOF) {
        return 1;
    }
    if (closePunctuation < PUNCTUATION_COUNT && IsPunctuationToken(token, closePunctuation)) {
        return 1;
    }
    if (IsClosingPunctuation(token)) {
        return 1;
    }
    return 0;
}

static int IsGenericTerminator(struct Token* token) {
    return token->type == TOKEN_EOF || IsOperatorToken(token, OPERATOR_GREATER);
}

static void FinishLocationFromChildren(struct AstNode* node) {
    if (node->childCount == 0) {
        return;
    }

    struct SourceLocation first = node->firstChild->location;
    struct SourceLocation last = node->lastChild->location;
    node->location = first;
    if (last.offset + last.length >= first.offset) {
        node->location.length = (last.offset + last.length) - first.offset;
    }
}

static struct AstNode* ParseStatement(struct Parser* parser, enum PunctuationType closePunctuation);
static int ShouldParseGenericList(struct Parser* parser, struct AstNode* previousChild);

static struct AstNode* ParseTokenNode(struct Parser* parser) {
    struct Token token = parser->tokens[parser->position++];
    struct AstNode* node = CreateAstNode(parser, AST_TOKEN, token.location);
    node->token = token;
    return node;
}

static void ParseChildrenInto(struct Parser* parser, struct AstNode* parent, enum PunctuationType closePunctuation) {
    while (parser->position < parser->count && !IsAstTerminator(&parser->tokens[parser->position], closePunctuation)) {
        if (IsTriviaToken(&parser->tokens[parser->position])) {
            parser->position++;
            continue;
        }
        size_t previousPosition = parser->position;
        struct AstNode* statement = ParseStatement(parser, closePunctuation);
        if (statement->childCount > 0) {
            AddChild(parent, statement);
        } else if (parser->position == previousPosition) {
            break;
        } else {
            continue;
        }
    }
}

static struct AstNode* ParseDelimited(struct Parser* parser, enum AstNodeType type, enum PunctuationType closePunctuation) {
    struct Token open = parser->tokens[parser->position++];
    struct AstNode* node = CreateAstNode(parser, type, open.location);

    ParseChildrenInto(parser, node, closePunctuation);

    if (parser->position < parser->count && IsPunctuationToken(&parser->tokens[parser->position], closePunctuation)) {
        struct Token close = parser->tokens[parser->position++];
        node->location = open.location;
        node->location.length = (close.location.offset + close.location.length) - open.location.offset;
    } else if (parser->position < parser->count) {
        char message[128];
        snprintf(message, sizeof(message), "expected '%s'", PunctuationName(closePunctuation));
        ReportParseError(parser, &parser->tokens[parser->position], message);
        if (IsClosingPunctuation(&parser->tokens[parser->position])) {
            parser->position++;
        }
    } else {
        ReportParseError(parser, &open, "unterminated delimiter");
    }

    if (node->childCount > 0 && node->location.length == open.location.length) {
        FinishLocationFromChildren(node);
        node->location.offset = open.location.offset;
        node->location.line = open.location.line;
        node->location.column = open.location.column;
    }
    return node;
}

static struct AstNode* ParseGenericDelimited(struct Parser* parser) {
    struct Token open = parser->tokens[parser->position++];
    struct AstNode* node = CreateAstNode(parser, AST_GENERIC, open.location);

    while (parser->position < parser->count && !IsGenericTerminator(&parser->tokens[parser->position])) {
        if (IsTriviaToken(&parser->tokens[parser->position])) {
            parser->position++;
            continue;
        }
        struct AstNode* statement = CreateAstNode(parser, AST_STATEMENT, parser->tokens[parser->position].location);
        while (parser->position < parser->count && !IsGenericTerminator(&parser->tokens[parser->position])) {
            struct Token* token = &parser->tokens[parser->position];
            if (IsTriviaToken(token)) {
                parser->position++;
                continue;
            }
            if (IsPunctuationToken(token, PUNCTUATION_COMMA)) {
                parser->position++;
                break;
            }
            if (IsPunctuationToken(token, PUNCTUATION_LEFT_PAREN)) {
                AddChild(statement, ParseDelimited(parser, AST_GROUP, PUNCTUATION_RIGHT_PAREN));
                continue;
            }
            if (IsPunctuationToken(token, PUNCTUATION_LEFT_BRACKET)) {
                AddChild(statement, ParseDelimited(parser, AST_INDEX, PUNCTUATION_RIGHT_BRACKET));
                continue;
            }
            if (ShouldParseGenericList(parser, statement->lastChild)) {
                AddChild(statement, ParseGenericDelimited(parser));
                continue;
            }
            AddChild(statement, ParseTokenNode(parser));
        }
        FinishLocationFromChildren(statement);
        if (statement->childCount > 0) {
            AddChild(node, statement);
        }
    }

    if (parser->position < parser->count && IsOperatorToken(&parser->tokens[parser->position], OPERATOR_GREATER)) {
        struct Token close = parser->tokens[parser->position++];
        node->location = open.location;
        node->location.length = (close.location.offset + close.location.length) - open.location.offset;
    } else if (parser->position < parser->count) {
        ReportParseError(parser, &parser->tokens[parser->position], "expected '>'");
    } else {
        ReportParseError(parser, &open, "unterminated generic list");
    }

    if (node->childCount > 0 && node->location.length == open.location.length) {
        FinishLocationFromChildren(node);
        node->location.offset = open.location.offset;
        node->location.line = open.location.line;
        node->location.column = open.location.column;
    }
    return node;
}

static int ShouldParseGenericList(struct Parser* parser, struct AstNode* previousChild) {
    if (!previousChild || previousChild->type != AST_TOKEN || previousChild->token.type != TOKEN_IDENTIFIER) {
        return 0;
    }
    if (TokenTextEquals(parser, &previousChild->token, "cast")) {
        return 0;
    }
    if (parser->position >= parser->count || !IsOperatorToken(&parser->tokens[parser->position], OPERATOR_LESS)) {
        return 0;
    }

    int depth = 0;
    for (size_t i = parser->position; i < parser->count; i++) {
        struct Token* token = &parser->tokens[i];
        if (IsOperatorToken(token, OPERATOR_LESS)) {
            depth++;
            continue;
        }
        if (IsOperatorToken(token, OPERATOR_GREATER)) {
            depth--;
            if (depth == 0) {
                return 1;
            }
            continue;
        }
        if (depth == 1
            && token->type == TOKEN_PUNCTUATION
            && token->value.punctuation == PUNCTUATION_DOT) {
            return 0;
        }
        if (token->type == TOKEN_EOF
            || IsPunctuationToken(token, PUNCTUATION_SEMICOLON)
            || IsPunctuationToken(token, PUNCTUATION_RIGHT_PAREN)
            || IsPunctuationToken(token, PUNCTUATION_LEFT_BRACE)
            || IsPunctuationToken(token, PUNCTUATION_RIGHT_BRACE)) {
            return 0;
        }
    }
    return 0;
}

static struct AstNode* ParseStatement(struct Parser* parser, enum PunctuationType closePunctuation) {
    struct AstNode* statement = CreateAstNode(parser, AST_STATEMENT, parser->tokens[parser->position].location);

    while (parser->position < parser->count && !IsAstTerminator(&parser->tokens[parser->position], closePunctuation)) {
        struct Token* token = &parser->tokens[parser->position];

        if (IsTriviaToken(token)) {
            parser->position++;
            continue;
        }

        if (IsPunctuationToken(token, PUNCTUATION_SEMICOLON) || IsPunctuationToken(token, PUNCTUATION_COMMA)) {
            parser->position++;
            break;
        }

        if (IsClosingPunctuation(token)) {
            break;
        }

        if (IsPunctuationToken(token, PUNCTUATION_LEFT_BRACE)) {
            AddChild(statement, ParseDelimited(parser, AST_BLOCK, PUNCTUATION_RIGHT_BRACE));
            break;
        }

        if (IsPunctuationToken(token, PUNCTUATION_LEFT_PAREN)) {
            AddChild(statement, ParseDelimited(parser, AST_GROUP, PUNCTUATION_RIGHT_PAREN));
            continue;
        }

        if (IsPunctuationToken(token, PUNCTUATION_LEFT_BRACKET)) {
            AddChild(statement, ParseDelimited(parser, AST_INDEX, PUNCTUATION_RIGHT_BRACKET));
            continue;
        }

        if (ShouldParseGenericList(parser, statement->lastChild)) {
            AddChild(statement, ParseGenericDelimited(parser));
            continue;
        }

        AddChild(statement, ParseTokenNode(parser));
    }

    FinishLocationFromChildren(statement);
    return statement;
}

static struct AstNode* ParseList(struct Parser* parser, enum AstNodeType listType, enum PunctuationType closePunctuation) {
    struct AstNode* list = CreateAstNode(parser, listType, parser->tokens[parser->position].location);

    ParseChildrenInto(parser, list, closePunctuation);

    if (closePunctuation == PUNCTUATION_COUNT
        && parser->position < parser->count
        && IsClosingPunctuation(&parser->tokens[parser->position])) {
        ReportParseError(parser, &parser->tokens[parser->position], "unexpected closing delimiter");
        parser->position++;
    }

    FinishLocationFromChildren(list);
    return list;
}

struct AstNode* ParseAst(struct Parser* parser) {
    struct SourceLocation location = {1, 1, 0, parser->file->length};
    struct AstNode* root = ParseList(parser, AST_FILE, PUNCTUATION_COUNT);
    root->location = location;
    return root;
}

static void PrintIndent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void PrintAst(struct AstNode* node, struct SourceFile* file, int indent) {
    PrintIndent(indent);
    printf("%s", AstNodeTypeNames[node->type]);
    if (node->type == AST_TOKEN) {
        printf(" %s", TokenTypeNames[node->token.type]);
        if (node->token.type == TOKEN_IDENTIFIER || node->token.type == TOKEN_NUMBER || node->token.type == TOKEN_STRING) {
            printf(" '%.*s'", (int)node->token.location.length, file->content + node->token.location.offset);
        } else if (node->token.type != TOKEN_EOF) {
            printf(" '%s'", TokenLexeme(&node->token, file));
        }
    }
    printf(" @ %zu:%zu len=%zu\n", node->location.line, node->location.column, node->location.length);

    for (struct AstNode* child = node->firstChild; child; child = child->nextSibling) {
        PrintAst(child, file, indent + 1);
    }
}

void FreeAst(struct AstNode* node) {
    (void)node;
}
