#include "kek.h"

const char* AstNodeTypeNames[] = {
    "File",
    "Statement",
    "Block",
    "Group",
    "Index",
    "Token",
};

static struct AstNode* CreateAstNode(enum AstNodeType type, struct SourceLocation location) {
    struct AstNode* node = calloc(1, sizeof(struct AstNode));
    if (!node) {
        fprintf(stderr, "Error: Could not allocate AST node.\n");
        exit(1);
    }
    node->type = type;
    node->location = location;
    return node;
}

static void AddChild(struct AstNode* parent, struct AstNode* child) {
    if (parent->childCount >= parent->childCapacity) {
        size_t capacity = parent->childCapacity == 0 ? 8 : parent->childCapacity * 2;
        struct AstNode** children = realloc(parent->children, capacity * sizeof(struct AstNode*));
        if (!children) {
            fprintf(stderr, "Error: Could not grow AST children.\n");
            exit(1);
        }
        parent->children = children;
        parent->childCapacity = capacity;
    }
    parent->children[parent->childCount++] = child;
}

static int IsPunctuationToken(struct Token* token, enum PunctuationType punctuation) {
    return token->type == TOKEN_PUNCTUATION && token->value.punctuation == punctuation;
}

static int IsClosingPunctuation(struct Token* token) {
    return IsPunctuationToken(token, PUNCTUATION_RIGHT_PAREN)
        || IsPunctuationToken(token, PUNCTUATION_RIGHT_BRACE)
        || IsPunctuationToken(token, PUNCTUATION_RIGHT_BRACKET);
}

static const char* PunctuationName(enum PunctuationType punctuation) {
    return punctuation < PUNCTUATION_COUNT ? PunctuationNames[punctuation] : "<end of file>";
}

static void ReportParseError(struct Parser* parser, struct Token* token, const char* message) {
    fprintf(stderr, "Syntax error at line %zu, column %zu: %s\n", token->location.line, token->location.column, message);
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

static void FinishLocationFromChildren(struct AstNode* node) {
    if (node->childCount == 0) {
        return;
    }

    struct SourceLocation first = node->children[0]->location;
    struct SourceLocation last = node->children[node->childCount - 1]->location;
    node->location = first;
    if (last.offset + last.length >= first.offset) {
        node->location.length = (last.offset + last.length) - first.offset;
    }
}

static struct AstNode* ParseList(struct Parser* parser, enum AstNodeType listType, enum PunctuationType closePunctuation);

static struct AstNode* ParseTokenNode(struct Parser* parser) {
    struct Token token = parser->tokens[parser->position++];
    struct AstNode* node = CreateAstNode(AST_TOKEN, token.location);
    node->token = token;
    return node;
}

static struct AstNode* ParseDelimited(struct Parser* parser, enum AstNodeType type, enum PunctuationType closePunctuation) {
    struct Token open = parser->tokens[parser->position++];
    struct AstNode* node = CreateAstNode(type, open.location);

    struct AstNode* body = ParseList(parser, type, closePunctuation);
    for (size_t i = 0; i < body->childCount; i++) {
        AddChild(node, body->children[i]);
    }
    free(body->children);
    free(body);

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

static struct AstNode* ParseStatement(struct Parser* parser, enum PunctuationType closePunctuation) {
    struct AstNode* statement = CreateAstNode(AST_STATEMENT, parser->tokens[parser->position].location);

    while (parser->position < parser->count && !IsAstTerminator(&parser->tokens[parser->position], closePunctuation)) {
        struct Token* token = &parser->tokens[parser->position];

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

        AddChild(statement, ParseTokenNode(parser));
    }

    FinishLocationFromChildren(statement);
    return statement;
}

static struct AstNode* ParseList(struct Parser* parser, enum AstNodeType listType, enum PunctuationType closePunctuation) {
    struct AstNode* list = CreateAstNode(listType, parser->tokens[parser->position].location);

    while (parser->position < parser->count && !IsAstTerminator(&parser->tokens[parser->position], closePunctuation)) {
        struct AstNode* statement = ParseStatement(parser, closePunctuation);
        if (statement->childCount > 0) {
            AddChild(list, statement);
        } else {
            free(statement);
            break;
        }
    }

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
    struct AstNode* root = CreateAstNode(AST_FILE, location);
    struct AstNode* list = ParseList(parser, AST_FILE, PUNCTUATION_COUNT);
    for (size_t i = 0; i < list->childCount; i++) {
        AddChild(root, list->children[i]);
    }
    free(list->children);
    free(list);
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

    for (size_t i = 0; i < node->childCount; i++) {
        PrintAst(node->children[i], file, indent + 1);
    }
}

void FreeAst(struct AstNode* node) {
    for (size_t i = 0; i < node->childCount; i++) {
        FreeAst(node->children[i]);
    }
    free(node->children);
    free(node);
}
