#include "kek_internal.h"

const char* KekDeclKindNames[] = {
    "Import",
    "Using",
    "Alias",
    "ExternC",
    "Struct",
    "Enum",
    "Union",
    "Function",
    "Variable",
    "Unknown",
};

const char* KekStmtKindNames[] = {
    "Block",
    "Decl",
    "Expr",
    "If",
    "Else",
    "While",
    "DoWhile",
    "For",
    "Switch",
    "Case",
    "Default",
    "Defer",
    "Return",
    "Break",
    "Continue",
    "Unknown",
};

const char* KekExprKindNames[] = {
    "Name",
    "Number",
    "String",
    "Bool",
    "Call",
    "Field",
    "Scope",
    "Index",
    "Group",
    "StructLiteral",
    "Unary",
    "Binary",
    "Assign",
    "Cast",
    "Unknown",
};

const char* KekTypeKindNames[] = {
    "Builtin",
    "Name",
    "Pointer",
    "Array",
    "Function",
    "Unknown",
};

static int IsTokenNode(struct AstNode* node) {
    return node && node->type == AST_TOKEN;
}

static int IsKeywordNode(struct AstNode* node, enum KeywordType keyword) {
    return IsTokenNode(node)
        && node->token.type == TOKEN_KEYWORD
        && node->token.value.keyword == keyword;
}

static int IsPunctuationNode(struct AstNode* node, enum PunctuationType punctuation) {
    return IsTokenNode(node)
        && node->token.type == TOKEN_PUNCTUATION
        && node->token.value.punctuation == punctuation;
}

static int IsGenericNode(struct AstNode* node) {
    return node && node->type == AST_GENERIC;
}

static int IsOperatorNode(struct AstNode* node, enum OperatorType operator) {
    return IsTokenNode(node)
        && node->token.type == TOKEN_OPERATOR
        && node->token.value.operator == operator;
}

static int IsAnyOverloadableOperatorNode(struct AstNode* node) {
    return IsTokenNode(node)
        && node->token.type == TOKEN_OPERATOR
        && node->token.value.operator != OPERATOR_SCOPE
        && node->token.value.operator != OPERATOR_ASSIGN;
}

static int TokenTextEquals(struct AstNode* node, struct SourceFile* file, const char* text) {
    size_t length = strlen(text);
    return IsTokenNode(node)
        && (node->token.type == TOKEN_IDENTIFIER || node->token.type == TOKEN_NUMBER || node->token.type == TOKEN_STRING)
        && node->token.location.length == length
        && strncmp(file->content + node->token.location.offset, text, length) == 0;
}

static struct AstNode* Next(struct AstNode* node) {
    return node ? node->nextSibling : NULL;
}

static void* AddFrontendNode(struct KekFrontend* frontend, struct KekModule* module, struct KekNodePool* pool, struct AstNode* source, const char* message) {
    if (pool->count >= pool->capacity) {
        KekAddDiagnostic(frontend->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_TYPED_PARSE,
            module->file ? module->file->fileIndex : -1,
            source ? source->location : (struct SourceLocation){0},
            message);
        frontend->errorCount++;
        module->errorCount++;
        return NULL;
    }

    char* items = pool->items;
    void* node = items + pool->itemSize * pool->count++;
    memset(node, 0, pool->itemSize);
    return node;
}

static struct KekType* AddType(struct KekFrontend* frontend, struct KekModule* module, enum KekTypeKind kind, struct AstNode* source) {
    struct KekType* type = AddFrontendNode(frontend, module, &frontend->types, source, "typed type storage capacity exceeded");
    if (!type) {
        return NULL;
    }
    type->kind = kind;
    type->source = source;
    type->location = source ? source->location : (struct SourceLocation){0};
    if (kind < KEK_TYPE_COUNT) {
        module->typeKindCounts[kind]++;
    }
    module->typedTypeCount++;
    return type;
}

static struct KekExpr* AddExpr(struct KekFrontend* frontend, struct KekModule* module, enum KekExprKind kind, struct AstNode* source) {
    struct KekExpr* expr = AddFrontendNode(frontend, module, &frontend->exprs, source, "typed expression storage capacity exceeded");
    if (!expr) {
        return NULL;
    }
    expr->kind = kind;
    expr->source = source;
    expr->location = source ? source->location : (struct SourceLocation){0};
    if (IsTokenNode(source)) {
        expr->token = source;
    }
    if (kind < KEK_EXPR_COUNT) {
        module->exprKindCounts[kind]++;
    }
    module->typedExprCount++;
    return expr;
}

static struct KekStmt* AddStmt(struct KekFrontend* frontend, struct KekModule* module, enum KekStmtKind kind, struct AstNode* source) {
    struct KekStmt* stmt = AddFrontendNode(frontend, module, &frontend->stmts, source, "typed statement storage capacity exceeded");
    if (!stmt) {
        return NULL;
    }
    stmt->kind = kind;
    stmt->source = source;
    stmt->location = source ? source->location : (struct SourceLocation){0};
    if (kind < KEK_STMT_COUNT) {
        module->stmtKindCounts[kind]++;
    }
    module->typedStmtCount++;
    return stmt;
}

static struct KekParam* AddParam(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* source) {
    struct KekParam* param = AddFrontendNode(frontend, module, &frontend->params, source, "typed parameter storage capacity exceeded");
    if (!param) {
        return NULL;
    }
    param->source = source;
    param->location = source ? source->location : (struct SourceLocation){0};
    module->paramCount++;
    return param;
}

static struct KekField* AddField(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* source) {
    struct KekField* field = AddFrontendNode(frontend, module, &frontend->fields, source, "typed field storage capacity exceeded");
    if (!field) {
        return NULL;
    }
    field->source = source;
    field->location = source ? source->location : (struct SourceLocation){0};
    module->fieldCount++;
    return field;
}

static struct KekVariant* AddVariant(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* source) {
    struct KekVariant* variant = AddFrontendNode(frontend, module, &frontend->variants, source, "typed variant storage capacity exceeded");
    if (!variant) {
        return NULL;
    }
    variant->source = source;
    variant->location = source ? source->location : (struct SourceLocation){0};
    module->variantCount++;
    return variant;
}

static struct KekDecl* AddDecl(struct KekFrontend* frontend, struct KekModule* module, enum KekDeclKind kind, struct AstNode* source) {
    struct KekDecl* decl = AddFrontendNode(frontend, module, &frontend->decls, source, "typed declaration storage capacity exceeded");
    if (!decl) {
        return NULL;
    }
    decl->kind = kind;
    decl->source = source;
    decl->location = source ? source->location : (struct SourceLocation){0};

    if (module->lastDecl) {
        module->lastDecl->next = decl;
    } else {
        module->firstDecl = decl;
    }
    module->lastDecl = decl;
    module->declCount++;
    if (kind < KEK_DECL_COUNT) {
        module->declKindCounts[kind]++;
    }
    return decl;
}

static struct KekExpr* ParseExprUntil(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* first, struct AstNode* stop);
static struct KekType* ParseType(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* typeNode, struct AstNode* afterName);

static void AddTypedParseDiagnostic(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* source, const char* message) {
    KekAddDiagnostic(frontend->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_TYPED_PARSE,
        module->file ? module->file->fileIndex : -1,
        source ? source->location : (struct SourceLocation){0},
        message);
    frontend->errorCount++;
    module->errorCount++;
}

static struct KekType* ParsePointerElementType(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* genericArgs) {
    if (!genericArgs) {
        return NULL;
    }
    if (!genericArgs->firstChild || genericArgs->childCount != 1) {
        AddTypedParseDiagnostic(frontend, module, genericArgs, "ptr<T> requires exactly one type argument");
        return AddType(frontend, module, KEK_TYPE_UNKNOWN, genericArgs);
    }

    struct AstNode* arg = genericArgs->firstChild;
    if (!arg || arg->type != AST_STATEMENT || !arg->firstChild) {
        AddTypedParseDiagnostic(frontend, module, genericArgs, "ptr<T> requires a type argument");
        return AddType(frontend, module, KEK_TYPE_UNKNOWN, genericArgs);
    }

    return ParseType(frontend, module, arg->firstChild, NULL);
}

static void AddExprArg(struct KekExpr* call, struct KekExpr* arg) {
    if (!call || !arg) {
        return;
    }
    if (call->lastArg) {
        call->lastArg->next = arg;
    } else {
        call->firstArg = arg;
    }
    call->lastArg = arg;
}

static void AddStructLiteralFields(struct KekFrontend* frontend, struct KekModule* module, struct KekExpr* literal, struct AstNode* block) {
    if (!literal || !block || block->type != AST_BLOCK) {
        return;
    }

    for (struct AstNode* field = block->firstChild; field; field = field->nextSibling) {
        if (field->firstChild) {
            AddExprArg(literal, ParseExprUntil(frontend, module, field->firstChild, NULL));
        }
    }
}

static struct KekExpr* ParseStructLiteralExpr(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* typeNode, struct AstNode* block) {
    struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_STRUCT_LITERAL, block ? block : typeNode);
    if (expr) {
        if (typeNode) {
            expr->type = ParseType(frontend, module, typeNode, NULL);
        }
        AddStructLiteralFields(frontend, module, expr, block);
    }
    return expr;
}

static struct KekExpr* ParseGroupExpr(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* group) {
    if (!group || (group->type != AST_GROUP && group->type != AST_INDEX) || !group->firstChild) {
        return AddExpr(frontend, module, KEK_EXPR_UNKNOWN, group);
    }
    struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_GROUP, group);
    if (expr) {
        expr->right = ParseExprUntil(frontend, module, group->firstChild->firstChild, NULL);
    }
    return expr;
}

static int IsExpressionEnd(struct AstNode* node, struct AstNode* stop) {
    return !node || node == stop;
}

static int IsAssignmentExprOperator(struct AstNode* node) {
    return IsOperatorNode(node, OPERATOR_ASSIGN)
        || IsOperatorNode(node, OPERATOR_PLUS_ASSIGN)
        || IsOperatorNode(node, OPERATOR_MINUS_ASSIGN);
}

static int ExprOperatorPrecedence(struct AstNode* node) {
    if (IsAssignmentExprOperator(node)) {
        return 1;
    }
    if (IsOperatorNode(node, OPERATOR_LOGICAL_OR)) {
        return 2;
    }
    if (IsOperatorNode(node, OPERATOR_LOGICAL_AND)) {
        return 3;
    }
    if (IsOperatorNode(node, OPERATOR_BITWISE_OR)) {
        return 4;
    }
    if (IsOperatorNode(node, OPERATOR_BITWISE_AND)) {
        return 5;
    }
    if (IsOperatorNode(node, OPERATOR_EQUAL) || IsOperatorNode(node, OPERATOR_NOT_EQUAL)) {
        return 6;
    }
    if (IsOperatorNode(node, OPERATOR_LESS)
        || IsOperatorNode(node, OPERATOR_LESS_EQUAL)
        || IsOperatorNode(node, OPERATOR_GREATER)
        || IsOperatorNode(node, OPERATOR_GREATER_EQUAL)) {
        return 7;
    }
    if (IsOperatorNode(node, OPERATOR_PLUS) || IsOperatorNode(node, OPERATOR_MINUS)) {
        return 8;
    }
    if (IsOperatorNode(node, OPERATOR_MULTIPLY)
        || IsOperatorNode(node, OPERATOR_DIVIDE)
        || IsOperatorNode(node, OPERATOR_MODULO)) {
        return 9;
    }
    return 0;
}

static int IsRightAssociativeExprOperator(struct AstNode* node) {
    return IsAssignmentExprOperator(node);
}

static int IsUnaryExprOperator(struct AstNode* node) {
    return IsOperatorNode(node, OPERATOR_LOGICAL_NOT)
        || IsOperatorNode(node, OPERATOR_BITWISE_NOT)
        || IsOperatorNode(node, OPERATOR_MINUS)
        || IsOperatorNode(node, OPERATOR_BITWISE_AND)
        || IsOperatorNode(node, OPERATOR_MULTIPLY);
}

static struct KekExpr* ParseExprPrecedence(struct KekFrontend* frontend, struct KekModule* module, struct AstNode** current, struct AstNode* stop, int minPrecedence);

static void AddCallArgs(struct KekFrontend* frontend, struct KekModule* module, struct KekExpr* call, struct AstNode* group) {
    if (!call || !group || group->type != AST_GROUP) {
        return;
    }

    for (struct AstNode* arg = group->firstChild; arg; arg = arg->nextSibling) {
        if (arg->firstChild) {
            AddExprArg(call, ParseExprUntil(frontend, module, arg->firstChild, NULL));
        }
    }
}

static struct KekExpr* ParsePrimaryExpr(struct KekFrontend* frontend, struct KekModule* module, struct AstNode** current, struct AstNode* stop) {
    struct AstNode* node = *current;
    if (IsExpressionEnd(node, stop)) {
        return AddExpr(frontend, module, KEK_EXPR_UNKNOWN, node);
    }

    if (node->type == AST_GROUP) {
        *current = Next(node);
        return ParseGroupExpr(frontend, module, node);
    }

    if (node->type == AST_BLOCK) {
        *current = Next(node);
        return ParseStructLiteralExpr(frontend, module, NULL, node);
    }

    if (IsTokenNode(node)
        && node->token.type == TOKEN_IDENTIFIER
        && IsPunctuationNode(Next(node), PUNCTUATION_COLON)
        && Next(Next(node))
        && Next(Next(node))->type == AST_BLOCK) {
        struct AstNode* block = Next(Next(node));
        *current = Next(block);
        return ParseStructLiteralExpr(frontend, module, node, block);
    }

    if (IsOperatorNode(node, OPERATOR_SCOPE)) {
        struct AstNode* name = Next(node);
        struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_SCOPE, node);
        if (expr) {
            expr->right = AddExpr(frontend, module, KEK_EXPR_NAME, name);
        }
        *current = name ? Next(name) : Next(node);
        return expr;
    }

    if (IsTokenNode(node) && TokenTextEquals(node, module->file, "cast")) {
        struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_CAST, node);
        struct AstNode* less = Next(node);
        struct AstNode* typeNode = Next(less);
        struct AstNode* greater = Next(typeNode);
        struct AstNode* valueGroup = Next(greater);
        if (expr
            && IsOperatorNode(less, OPERATOR_LESS)
            && typeNode
            && IsOperatorNode(greater, OPERATOR_GREATER)
            && valueGroup
            && valueGroup->type == AST_GROUP) {
            expr->type = ParseType(frontend, module, typeNode, NULL);
            expr->right = ParseGroupExpr(frontend, module, valueGroup);
            *current = Next(valueGroup);
        } else {
            *current = Next(node);
        }
        return expr;
    }

    if (IsUnaryExprOperator(node)) {
        struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_UNARY, node);
        *current = Next(node);
        if (expr) {
            expr->right = ParseExprPrecedence(frontend, module, current, stop, 10);
        }
        return expr;
    }

    enum KekExprKind kind = KEK_EXPR_UNKNOWN;
    if (IsTokenNode(node) && node->token.type == TOKEN_IDENTIFIER) {
        kind = KEK_EXPR_NAME;
    } else if (IsTokenNode(node) && node->token.type == TOKEN_NUMBER) {
        kind = KEK_EXPR_NUMBER;
    } else if (IsTokenNode(node) && node->token.type == TOKEN_STRING) {
        kind = KEK_EXPR_STRING;
    } else if (IsKeywordNode(node, KEYWORD_TRUE) || IsKeywordNode(node, KEYWORD_FALSE)) {
        kind = KEK_EXPR_BOOL;
    }

    struct KekExpr* expr = AddExpr(frontend, module, kind, node);
    *current = Next(node);
    return expr;
}

static struct KekExpr* ParsePostfixExpr(struct KekFrontend* frontend, struct KekModule* module, struct AstNode** current, struct AstNode* stop) {
    struct KekExpr* expr = ParsePrimaryExpr(frontend, module, current, stop);

    while (!IsExpressionEnd(*current, stop)) {
        struct AstNode* node = *current;

        if (node->type == AST_GROUP) {
            struct KekExpr* call = AddExpr(frontend, module, KEK_EXPR_CALL, node);
            if (call) {
                call->callee = expr;
                AddCallArgs(frontend, module, call, node);
            }
            expr = call;
            *current = Next(node);
            continue;
        }

        if (node->type == AST_INDEX) {
            struct KekExpr* index = AddExpr(frontend, module, KEK_EXPR_INDEX, node);
            if (index) {
                index->left = expr;
                index->right = ParseGroupExpr(frontend, module, node);
            }
            expr = index;
            *current = Next(node);
            continue;
        }

        if (IsGenericNode(node)) {
            if (expr) {
                expr->genericArgs = node;
            }
            *current = Next(node);
            continue;
        }

        if (IsPunctuationNode(node, PUNCTUATION_DOT)) {
            struct AstNode* name = Next(node);
            struct KekExpr* field = AddExpr(frontend, module, KEK_EXPR_FIELD, node);
            if (field) {
                field->left = expr;
                field->right = AddExpr(frontend, module, KEK_EXPR_NAME, name);
            }
            expr = field;
            *current = name ? Next(name) : Next(node);
            continue;
        }

        if (IsOperatorNode(node, OPERATOR_SCOPE)) {
            struct AstNode* name = Next(node);
            struct KekExpr* scoped = AddExpr(frontend, module, KEK_EXPR_SCOPE, node);
            if (scoped) {
                scoped->left = expr;
                scoped->right = AddExpr(frontend, module, KEK_EXPR_NAME, name);
            }
            expr = scoped;
            *current = name ? Next(name) : Next(node);
            continue;
        }

        break;
    }

    return expr;
}

static struct KekExpr* ParseExprPrecedence(struct KekFrontend* frontend, struct KekModule* module, struct AstNode** current, struct AstNode* stop, int minPrecedence) {
    struct KekExpr* left = ParsePostfixExpr(frontend, module, current, stop);

    while (!IsExpressionEnd(*current, stop)) {
        struct AstNode* op = *current;
        int precedence = ExprOperatorPrecedence(op);
        if (precedence == 0 || precedence < minPrecedence) {
            break;
        }

        *current = Next(op);
        struct KekExpr* right = ParseExprPrecedence(frontend, module, current, stop,
            precedence + (IsRightAssociativeExprOperator(op) ? 0 : 1));
        struct KekExpr* combined = AddExpr(frontend, module,
            IsAssignmentExprOperator(op) ? KEK_EXPR_ASSIGN : KEK_EXPR_BINARY, op);
        if (combined) {
            combined->left = left;
            combined->right = right;
        }
        left = combined;
    }

    return left;
}

static struct KekExpr* ParseExprUntil(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* first, struct AstNode* stop) {
    struct AstNode* current = first;
    if (!current) {
        return AddExpr(frontend, module, KEK_EXPR_UNKNOWN, NULL);
    }

    return ParseExprPrecedence(frontend, module, &current, stop, 1);
}

static int IsBuiltinTypeName(struct AstNode* typeNode, struct SourceFile* file) {
    return TokenTextEquals(typeNode, file, "void")
        || TokenTextEquals(typeNode, file, "bool")
        || TokenTextEquals(typeNode, file, "char")
        || TokenTextEquals(typeNode, file, "int")
        || TokenTextEquals(typeNode, file, "u8")
        || TokenTextEquals(typeNode, file, "u16")
        || TokenTextEquals(typeNode, file, "u32")
        || TokenTextEquals(typeNode, file, "u64")
        || TokenTextEquals(typeNode, file, "i8")
        || TokenTextEquals(typeNode, file, "i16")
        || TokenTextEquals(typeNode, file, "i32")
        || TokenTextEquals(typeNode, file, "i64")
        || TokenTextEquals(typeNode, file, "f32")
        || TokenTextEquals(typeNode, file, "f64");
}

static struct KekType* ParseType(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* typeNode, struct AstNode* afterName) {
    if (!typeNode) {
        return AddType(frontend, module, KEK_TYPE_UNKNOWN, NULL);
    }

    enum KekTypeKind baseKind = KEK_TYPE_NAME;
    if (TokenTextEquals(typeNode, module->file, "ptr")) {
        baseKind = KEK_TYPE_POINTER;
    } else if (IsBuiltinTypeName(typeNode, module->file)) {
        baseKind = KEK_TYPE_BUILTIN;
    }

    struct KekType* base = AddType(frontend, module, baseKind, typeNode);
    if (!base) {
        return NULL;
    }
    base->name = typeNode;
    if (baseKind == KEK_TYPE_POINTER && IsGenericNode(Next(typeNode))) {
        base->element = ParsePointerElementType(frontend, module, Next(typeNode));
    } else if (IsGenericNode(Next(typeNode))) {
        base->genericArgs = Next(typeNode);
    }

    struct AstNode* arrayNode = NULL;
    if (afterName && afterName->type == AST_INDEX) {
        arrayNode = afterName;
    } else if (afterName && Next(afterName) && Next(afterName)->type == AST_INDEX) {
        arrayNode = Next(afterName);
    }

    while (arrayNode) {
        struct KekType* array = AddType(frontend, module, KEK_TYPE_ARRAY, arrayNode);
        if (array) {
            array->element = base;
            array->arraySize = ParseGroupExpr(frontend, module, arrayNode);
            base = array;
        }
        arrayNode = Next(arrayNode) && Next(arrayNode)->type == AST_INDEX ? Next(arrayNode) : NULL;
    }

    return base;
}

static void AddChildStmt(struct KekStmt* parent, struct KekStmt* child) {
    if (!parent || !child) {
        return;
    }
    if (parent->lastChild) {
        parent->lastChild->next = child;
    } else {
        parent->firstChild = child;
    }
    parent->lastChild = child;
}

static int LooksLikeDecl(struct AstNode* first) {
    struct AstNode* colon = Next(first);
    if (IsGenericNode(colon)) {
        colon = Next(colon);
    }
    return IsTokenNode(first)
        && colon
        && IsPunctuationNode(colon, PUNCTUATION_COLON)
        && Next(colon)
        && IsTokenNode(Next(colon));
}

static struct AstNode* DeclColonAfterType(struct AstNode* type) {
    struct AstNode* colon = Next(type);
    if (IsGenericNode(colon)) {
        colon = Next(colon);
    }
    return colon;
}

static struct AstNode* DeclNameAfterType(struct AstNode* type) {
    struct AstNode* colon = DeclColonAfterType(type);
    return colon ? Next(colon) : NULL;
}

static struct AstNode* AfterNameAndArraySuffixes(struct AstNode* name) {
    struct AstNode* node = Next(name);
    if (IsGenericNode(node)) {
        node = Next(node);
    }
    while (node && node->type == AST_INDEX) {
        node = Next(node);
    }
    return node;
}

static struct KekExpr* ParseFirstGroupStatementExpr(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* group) {
    if (!group || group->type != AST_GROUP || !group->firstChild) {
        return AddExpr(frontend, module, KEK_EXPR_UNKNOWN, group);
    }
    return ParseExprUntil(frontend, module, group->firstChild->firstChild, NULL);
}

static enum KekStmtKind ClassifyStatementKind(struct AstNode* first) {
    if (IsKeywordNode(first, KEYWORD_IF)) {
        return KEK_STMT_IF;
    }
    if (IsKeywordNode(first, KEYWORD_ELSE)) {
        return KEK_STMT_ELSE;
    }
    if (IsKeywordNode(first, KEYWORD_WHILE)) {
        return KEK_STMT_WHILE;
    }
    if (IsKeywordNode(first, KEYWORD_DO)) {
        return KEK_STMT_DO_WHILE;
    }
    if (IsKeywordNode(first, KEYWORD_FOR)) {
        return KEK_STMT_FOR;
    }
    if (IsKeywordNode(first, KEYWORD_SWITCH)) {
        return KEK_STMT_SWITCH;
    }
    if (IsKeywordNode(first, KEYWORD_CASE)) {
        return KEK_STMT_CASE;
    }
    if (IsKeywordNode(first, KEYWORD_DEFAULT)) {
        return KEK_STMT_DEFAULT;
    }
    if (IsKeywordNode(first, KEYWORD_DEFER)) {
        return KEK_STMT_DEFER;
    }
    if (IsKeywordNode(first, KEYWORD_RETURN)) {
        return KEK_STMT_RETURN;
    }
    if (IsKeywordNode(first, KEYWORD_BREAK)) {
        return KEK_STMT_BREAK;
    }
    if (IsKeywordNode(first, KEYWORD_CONTINUE)) {
        return KEK_STMT_CONTINUE;
    }
    if (LooksLikeDecl(first)) {
        return KEK_STMT_DECL;
    }
    return KEK_STMT_EXPR;
}

static struct KekStmt* ParseStatementList(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* block);

static struct KekStmt* ParseStatement(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* statement) {
    if (!statement || statement->type != AST_STATEMENT || !statement->firstChild) {
        return AddStmt(frontend, module, KEK_STMT_UNKNOWN, statement);
    }

    struct AstNode* first = statement->firstChild;
    enum KekStmtKind kind = ClassifyStatementKind(first);

    struct KekStmt* stmt = AddStmt(frontend, module, kind, statement);
    if (!stmt) {
        return NULL;
    }

    if (kind == KEK_STMT_DECL) {
        stmt->declName = DeclNameAfterType(first);
        stmt->declType = ParseType(frontend, module, first, stmt->declName);
        struct AstNode* afterDecl = AfterNameAndArraySuffixes(stmt->declName);
        if (afterDecl && IsOperatorNode(afterDecl, OPERATOR_ASSIGN)) {
            stmt->expr = ParseExprUntil(frontend, module, Next(afterDecl), NULL);
            if (stmt->expr && stmt->expr->kind == KEK_EXPR_STRUCT_LITERAL && !stmt->expr->type) {
                stmt->expr->type = stmt->declType;
            }
        }
    } else if (kind == KEK_STMT_FOR) {
        struct AstNode* group = Next(first);
        struct AstNode* init = group && group->type == AST_GROUP ? group->firstChild : NULL;
        struct AstNode* condition = init ? init->nextSibling : NULL;
        struct AstNode* step = condition ? condition->nextSibling : NULL;
        if (init && init->firstChild) {
            if (LooksLikeDecl(init->firstChild)) {
                stmt->initStmt = ParseStatement(frontend, module, init);
            } else {
                stmt->expr = ParseExprUntil(frontend, module, init->firstChild, NULL);
            }
        }
        if (condition && condition->firstChild) {
            stmt->condition = ParseExprUntil(frontend, module, condition->firstChild, NULL);
        }
        if (step && step->firstChild) {
            stmt->step = ParseExprUntil(frontend, module, step->firstChild, NULL);
        }
    } else if (kind == KEK_STMT_IF || kind == KEK_STMT_WHILE || kind == KEK_STMT_SWITCH || kind == KEK_STMT_CASE) {
        stmt->condition = ParseFirstGroupStatementExpr(frontend, module, Next(first));
        stmt->expr = stmt->condition;
    } else if (kind == KEK_STMT_RETURN) {
        stmt->expr = ParseExprUntil(frontend, module, Next(first), NULL);
    } else if (kind == KEK_STMT_DEFAULT) {
        struct AstNode* colon = Next(first);
        if (IsPunctuationNode(colon, PUNCTUATION_COLON) && Next(colon)) {
            stmt->expr = ParseExprUntil(frontend, module, Next(colon), NULL);
        }
    } else if (kind == KEK_STMT_DEFER) {
        struct AstNode* deferred = Next(first);
        if (deferred && deferred->type != AST_BLOCK) {
            stmt->expr = ParseExprUntil(frontend, module, deferred, NULL);
        }
    } else if (kind == KEK_STMT_EXPR) {
        stmt->expr = ParseExprUntil(frontend, module, first, NULL);
    }

    for (struct AstNode* child = statement->firstChild; child; child = child->nextSibling) {
        if (child->type == AST_BLOCK) {
            if (kind == KEK_STMT_DECL && stmt->expr && stmt->expr->source == child) {
                continue;
            }
            AddChildStmt(stmt, ParseStatementList(frontend, module, child));
        }
    }

    return stmt;
}

static int IsDoWhileConditionStatement(struct AstNode* statement) {
    return statement
        && statement->type == AST_STATEMENT
        && IsKeywordNode(statement->firstChild, KEYWORD_WHILE)
        && Next(statement->firstChild)
        && Next(statement->firstChild)->type == AST_GROUP
        && !Next(Next(statement->firstChild));
}

static struct KekStmt* ParseStatementList(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* block) {
    if (!block || block->type != AST_BLOCK) {
        return NULL;
    }

    struct KekStmt* blockStmt = AddStmt(frontend, module, KEK_STMT_BLOCK, block);
    for (struct AstNode* statement = block->firstChild; statement; statement = statement->nextSibling) {
        struct KekStmt* parsed = ParseStatement(frontend, module, statement);
        if (parsed && parsed->kind == KEK_STMT_DO_WHILE && IsDoWhileConditionStatement(statement->nextSibling)) {
            parsed->condition = ParseFirstGroupStatementExpr(frontend, module, Next(statement->nextSibling->firstChild));
            parsed->expr = parsed->condition;
            statement = statement->nextSibling;
        }
        AddChildStmt(blockStmt, parsed);
    }
    return blockStmt;
}

static enum KekDeclKind ClassifyColonDecl(struct AstNode* first, struct SourceFile* file, struct KekDecl* decl) {
    struct AstNode* colon = Next(first);
    if (IsGenericNode(colon)) {
        colon = Next(colon);
    }
    struct AstNode* name = Next(colon);
    if (!IsPunctuationNode(colon, PUNCTUATION_COLON) || !IsTokenNode(name) || name->token.type != TOKEN_IDENTIFIER) {
        return KEK_DECL_UNKNOWN;
    }

    decl->type = first;
    decl->name = name;

    struct AstNode* afterName = Next(name);
    if (IsTokenNode(name)
        && name->token.type == TOKEN_IDENTIFIER
        && afterName
        && IsAnyOverloadableOperatorNode(afterName)
        && TokenTextEquals(name, file, "operator")) {
        decl->name = afterName;
        afterName = Next(afterName);
    }
    if (IsGenericNode(afterName)) {
        decl->genericParams = afterName;
        afterName = Next(afterName);
    }
    if (afterName && afterName->type == AST_GROUP) {
        decl->body = Next(afterName);
        return KEK_DECL_FUNCTION;
    }

    decl->body = afterName;
    return KEK_DECL_VARIABLE;
}

static enum KekDeclKind ClassifyKeywordDecl(struct AstNode* first, struct SourceFile* file, struct KekDecl* decl) {
    if (IsKeywordNode(first, KEYWORD_USING)) {
        decl->name = Next(first);
        return KEK_DECL_USING;
    }

    if (IsKeywordNode(first, KEYWORD_ALIAS)) {
        struct AstNode* colon = Next(first);
        struct AstNode* name = Next(colon);
        struct AstNode* equals = Next(name);
        if (IsPunctuationNode(colon, PUNCTUATION_COLON)
            && IsTokenNode(name)
            && IsOperatorNode(equals, OPERATOR_ASSIGN)) {
            decl->name = name;
            decl->type = Next(equals);
        }
        return KEK_DECL_ALIAS;
    }

    if (IsKeywordNode(first, KEYWORD_EXTERN)
        && TokenTextEquals(Next(first), file, "\"C\"")
        && Next(Next(first))
        && Next(Next(first))->type == AST_BLOCK) {
        decl->body = Next(Next(first));
        return KEK_DECL_EXTERN_C;
    }

    if (IsKeywordNode(first, KEYWORD_STRUCT)
        || IsKeywordNode(first, KEYWORD_UNION)) {
        struct AstNode* colon = Next(first);
        struct AstNode* name = Next(colon);
        if (IsPunctuationNode(colon, PUNCTUATION_COLON) && IsTokenNode(name)) {
            decl->name = name;
            decl->genericParams = IsGenericNode(Next(name)) ? Next(name) : NULL;
            decl->body = decl->genericParams ? Next(decl->genericParams) : Next(name);
        }
        return IsKeywordNode(first, KEYWORD_STRUCT) ? KEK_DECL_STRUCT : KEK_DECL_UNION;
    }

    if (IsKeywordNode(first, KEYWORD_ENUM)) {
        struct AstNode* firstColon = Next(first);
        struct AstNode* underlyingType = Next(firstColon);
        struct AstNode* secondColon = Next(underlyingType);
        struct AstNode* name = Next(secondColon);
        if (IsPunctuationNode(firstColon, PUNCTUATION_COLON)
            && IsPunctuationNode(secondColon, PUNCTUATION_COLON)
            && IsTokenNode(name)) {
            decl->type = underlyingType;
            decl->name = name;
            decl->body = Next(name);
        }
        return KEK_DECL_ENUM;
    }

    (void)file;
    return KEK_DECL_UNKNOWN;
}

static enum KekDeclKind ClassifyDecl(struct KekDecl* decl, struct SourceFile* file) {
    struct AstNode* first = decl->source ? decl->source->firstChild : NULL;
    if (!first) {
        return KEK_DECL_UNKNOWN;
    }

    if (IsPunctuationNode(first, PUNCTUATION_HASH) && TokenTextEquals(Next(first), file, "import")) {
        decl->body = Next(Next(first));
        return KEK_DECL_IMPORT;
    }

    if (first->type == AST_INDEX) {
        first = Next(first);
    }

    if (IsKeywordNode(first, KEYWORD_USING)
        || IsKeywordNode(first, KEYWORD_ALIAS)
        || IsKeywordNode(first, KEYWORD_EXTERN)
        || IsKeywordNode(first, KEYWORD_STRUCT)
        || IsKeywordNode(first, KEYWORD_ENUM)
        || IsKeywordNode(first, KEYWORD_UNION)) {
        return ClassifyKeywordDecl(first, file, decl);
    }

    struct AstNode* colon = IsTokenNode(first) ? Next(first) : NULL;
    if (IsGenericNode(colon)) {
        colon = Next(colon);
    }
    if (IsTokenNode(first) && colon && IsPunctuationNode(colon, PUNCTUATION_COLON)) {
        struct AstNode* name = Next(colon);
        struct AstNode* afterName = Next(name);
        if (IsGenericNode(afterName)) {
            afterName = Next(afterName);
        }
        if (IsTokenNode(name) && afterName && IsOperatorNode(afterName, OPERATOR_SCOPE)) {
            decl->type = first;
            decl->name = Next(afterName);
            if (IsTokenNode(decl->name)
                && decl->name->token.type == TOKEN_IDENTIFIER
                && IsAnyOverloadableOperatorNode(Next(decl->name))
                && TokenTextEquals(decl->name, file, "operator")) {
                decl->name = Next(decl->name);
            }
            struct AstNode* receiverGenericParams = IsGenericNode(Next(name)) ? Next(name) : NULL;
            decl->genericParams = IsGenericNode(Next(decl->name)) ? Next(decl->name) : receiverGenericParams;
            struct AstNode* params = Next(decl->genericParams ? decl->genericParams : decl->name);
            if (receiverGenericParams && decl->genericParams == receiverGenericParams) {
                params = Next(decl->name);
            }
            decl->body = params ? Next(params) : NULL;
            return KEK_DECL_FUNCTION;
        }
        return ClassifyColonDecl(first, file, decl);
    }

    return KEK_DECL_UNKNOWN;
}

static void AddDeclStmt(struct KekDecl* decl, struct KekStmt* stmt) {
    if (!decl || !stmt) {
        return;
    }
    if (decl->lastStmt) {
        decl->lastStmt->next = stmt;
    } else {
        decl->firstStmt = stmt;
    }
    decl->lastStmt = stmt;
}

static void AddDeclParam(struct KekDecl* decl, struct KekParam* param) {
    if (!decl || !param) {
        return;
    }
    if (decl->lastParam) {
        decl->lastParam->next = param;
    } else {
        decl->firstParam = param;
    }
    decl->lastParam = param;
}

static void AddDeclField(struct KekDecl* decl, struct KekField* field) {
    if (!decl || !field) {
        return;
    }
    if (decl->lastField) {
        decl->lastField->next = field;
    } else {
        decl->firstField = field;
    }
    decl->lastField = field;
}

static void AddDeclVariant(struct KekDecl* decl, struct KekVariant* variant) {
    if (!decl || !variant) {
        return;
    }
    if (decl->lastVariant) {
        decl->lastVariant->next = variant;
    } else {
        decl->firstVariant = variant;
    }
    decl->lastVariant = variant;
}

static void BuildParamList(struct KekFrontend* frontend, struct KekModule* module, struct KekDecl* decl, struct AstNode* group) {
    if (!group || group->type != AST_GROUP) {
        return;
    }

    for (struct AstNode* paramNode = group->firstChild; paramNode; paramNode = paramNode->nextSibling) {
        struct AstNode* typeNode = paramNode->firstChild;
        if (!LooksLikeDecl(typeNode)) {
            continue;
        }

        struct AstNode* name = DeclNameAfterType(typeNode);
        struct KekParam* param = AddParam(frontend, module, paramNode);
        if (!param) {
            return;
        }
        param->type = ParseType(frontend, module, typeNode, name);
        param->name = name;
        struct AstNode* afterParam = AfterNameAndArraySuffixes(name);
        if (afterParam && IsOperatorNode(afterParam, OPERATOR_ASSIGN)) {
            param->defaultValue = ParseExprUntil(frontend, module, Next(afterParam), NULL);
        }
        AddDeclParam(decl, param);
    }
}

static void BuildStructFields(struct KekFrontend* frontend, struct KekModule* module, struct KekDecl* decl) {
    if (!decl->body || decl->body->type != AST_BLOCK) {
        return;
    }

    for (struct AstNode* field = decl->body->firstChild; field; field = field->nextSibling) {
        if (LooksLikeDecl(field->firstChild)) {
            struct AstNode* fieldName = DeclNameAfterType(field->firstChild);
            struct KekField* typedField = AddField(frontend, module, field);
            if (!typedField) {
                return;
            }
            typedField->type = ParseType(frontend, module, field->firstChild, fieldName);
            typedField->name = fieldName;
            struct AstNode* afterField = AfterNameAndArraySuffixes(fieldName);
            if (afterField && IsOperatorNode(afterField, OPERATOR_ASSIGN)) {
                typedField->defaultValue = ParseExprUntil(frontend, module, Next(afterField), NULL);
            }
            AddDeclField(decl, typedField);
        }
    }
}

static void BuildEnumVariants(struct KekFrontend* frontend, struct KekModule* module, struct KekDecl* decl) {
    if (!decl->body || decl->body->type != AST_BLOCK) {
        return;
    }

    for (struct AstNode* variantNode = decl->body->firstChild; variantNode; variantNode = variantNode->nextSibling) {
        struct AstNode* name = variantNode->firstChild;
        if (!IsTokenNode(name) || name->token.type != TOKEN_IDENTIFIER) {
            continue;
        }
        struct KekVariant* variant = AddVariant(frontend, module, variantNode);
        if (!variant) {
            return;
        }
        variant->name = name;
        if (Next(name) && IsOperatorNode(Next(name), OPERATOR_ASSIGN)) {
            variant->value = ParseExprUntil(frontend, module, Next(Next(name)), NULL);
        }
        AddDeclVariant(decl, variant);
    }
}

static void BuildDeclDetails(struct KekFrontend* frontend, struct KekModule* module, struct KekDecl* decl) {
    if (!decl) {
        return;
    }

    if (decl->type) {
        decl->parsedType = ParseType(frontend, module, decl->type, decl->name);
    }

    if (decl->kind == KEK_DECL_FUNCTION) {
        struct AstNode* params = decl->name ? Next(decl->name) : NULL;
        if (IsGenericNode(params)) {
            params = Next(params);
        }
        if (params && params->type == AST_GROUP) {
            BuildParamList(frontend, module, decl, params);
        }
        if (decl->body && decl->body->type == AST_BLOCK) {
            AddDeclStmt(decl, ParseStatementList(frontend, module, decl->body));
        }
    }

    if (decl->kind == KEK_DECL_STRUCT || decl->kind == KEK_DECL_UNION) {
        BuildStructFields(frontend, module, decl);
    }

    if (decl->kind == KEK_DECL_ENUM) {
        BuildEnumVariants(frontend, module, decl);
    }
}

struct KekModule ParseKekModule(struct KekFrontend* frontend, struct AstNode* ast, struct SourceFile* file) {
    struct KekModule module = {0};
    module.file = file;

    if (!frontend || !ast || ast->type != AST_FILE || !file) {
        if (frontend) {
            frontend->errorCount++;
        }
        module.errorCount++;
        return module;
    }

    frontend->file = file;

    for (struct AstNode* statement = ast->firstChild; statement; statement = statement->nextSibling) {
        if (statement->type != AST_STATEMENT || statement->childCount == 0) {
            continue;
        }

        struct KekDecl* decl = AddDecl(frontend, &module, KEK_DECL_UNKNOWN, statement);
        if (!decl) {
            break;
        }

        decl->kind = ClassifyDecl(decl, file);
        module.declKindCounts[KEK_DECL_UNKNOWN]--;
        module.declKindCounts[decl->kind]++;
        if (decl->kind == KEK_DECL_UNKNOWN) {
            KekAddDiagnostic(frontend->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_TYPED_PARSE,
                file ? file->fileIndex : -1, decl->location, "unknown top-level declaration");
            module.errorCount++;
            frontend->errorCount++;
        }
        BuildDeclDetails(frontend, &module, decl);
    }

    return module;
}

static void WriteKekModuleSummary(FILE* out, struct KekModule* module) {
    if (!module) {
        return;
    }

    fprintf(out, "Typed module %s: %zu declarations", module->file ? module->file->path : "<unknown>", module->declCount);
    if (module->errorCount > 0) {
        fprintf(out, ", %d errors", module->errorCount);
    }
    fprintf(out, "\n");

    for (size_t i = 0; i < KEK_DECL_COUNT; i++) {
        if (module->declKindCounts[i] > 0) {
            fprintf(out, "  %s: %zu\n", KekDeclKindNames[i], module->declKindCounts[i]);
        }
    }

    fprintf(out, "Typed nodes\n");
    fprintf(out, "  Params: %zu\n", module->paramCount);
    fprintf(out, "  Fields: %zu\n", module->fieldCount);
    fprintf(out, "  Variants: %zu\n", module->variantCount);
    fprintf(out, "  Statements: %zu\n", module->typedStmtCount);
    fprintf(out, "  Expressions: %zu\n", module->typedExprCount);
    fprintf(out, "  Types: %zu\n", module->typedTypeCount);

    fprintf(out, "Statements\n");
    for (size_t i = 0; i < KEK_STMT_COUNT; i++) {
        if (module->stmtKindCounts[i] > 0) {
            fprintf(out, "  %s: %zu\n", KekStmtKindNames[i], module->stmtKindCounts[i]);
        }
    }

    fprintf(out, "Expressions\n");
    for (size_t i = 0; i < KEK_EXPR_COUNT; i++) {
        if (module->exprKindCounts[i] > 0) {
            fprintf(out, "  %s: %zu\n", KekExprKindNames[i], module->exprKindCounts[i]);
        }
    }

    fprintf(out, "Types\n");
    for (size_t i = 0; i < KEK_TYPE_COUNT; i++) {
        if (module->typeKindCounts[i] > 0) {
            fprintf(out, "  %s: %zu\n", KekTypeKindNames[i], module->typeKindCounts[i]);
        }
    }
}

void PrintKekModuleSummary(struct KekModule* module) {
    WriteKekModuleSummary(stdout, module);
}

int WriteKekModuleSummaryFile(const char* path, struct KekModule* modules, size_t moduleCount, struct KekProgram* program) {
    FILE* out = fopen(path, "w");
    if (!out) {
        return -1;
    }

    for (size_t i = 0; i < moduleCount; i++) {
        if (i > 0) {
            fputc('\n', out);
        }
        WriteKekModuleSummary(out, &modules[i]);
    }

    if (program) {
        fprintf(out, "\nTyped program symbols: %zu", program->symbolCount);
        if (program->errorCount > 0) {
            fprintf(out, ", %d errors", program->errorCount);
        }
        fputc('\n', out);
        for (size_t i = 0; i < KEK_SYMBOL_COUNT; i++) {
            if (program->symbolKindCounts[i] > 0) {
                fprintf(out, "  %s: %zu\n", KekSymbolKindNames[i], program->symbolKindCounts[i]);
            }
        }
        fprintf(out, "Typed scopes: %zu\n", program->scopeCount);
        for (size_t i = 0; i < KEK_SCOPE_COUNT; i++) {
            if (program->scopeKindCounts[i] > 0) {
                fprintf(out, "  %s: %zu\n", KekScopeKindNames[i], program->scopeKindCounts[i]);
            }
        }
        fprintf(out, "Semantic checks: %zu\n", program->semanticCheckCount);
    }

    fclose(out);
    return 0;
}
