#include "kek_internal.h"

const char* KekSymbolKindNames[] = {
    "Type",
    "Function",
    "Global",
    "Param",
    "Local",
    "Import",
    "Unknown",
};

const char* KekScopeKindNames[] = {
    "Program",
    "Module",
    "Function",
    "Block",
    "Loop",
};

static enum KekSymbolKind SymbolKindForDecl(enum KekDeclKind kind) {
    switch (kind) {
        case KEK_DECL_IMPORT:
        case KEK_DECL_USING:
            return KEK_SYMBOL_IMPORT;
        case KEK_DECL_STRUCT:
        case KEK_DECL_ENUM:
        case KEK_DECL_UNION:
        case KEK_DECL_ALIAS:
            return KEK_SYMBOL_TYPE;
        case KEK_DECL_FUNCTION:
        case KEK_DECL_EXTERN_C:
            return KEK_SYMBOL_FUNCTION;
        case KEK_DECL_VARIABLE:
            return KEK_SYMBOL_GLOBAL;
        case KEK_DECL_UNKNOWN:
        case KEK_DECL_COUNT:
            return KEK_SYMBOL_UNKNOWN;
    }

    return KEK_SYMBOL_UNKNOWN;
}

static int SameSymbolNameInFile(struct AstNode* left, struct AstNode* right, struct SourceFile* file) {
    if (!left || !right || !file) {
        return 0;
    }
    if (left->token.location.length != right->token.location.length) {
        return 0;
    }
    return strncmp(file->content + left->token.location.offset,
        file->content + right->token.location.offset,
        left->token.location.length) == 0;
}

static int SameSymbolNameAcrossFiles(struct AstNode* left, struct SourceFile* leftFile, struct AstNode* right, struct SourceFile* rightFile) {
    if (!left || !leftFile || !right || !rightFile) {
        return 0;
    }
    if (left->token.location.length != right->token.location.length) {
        return 0;
    }
    return strncmp(leftFile->content + left->token.location.offset,
        rightFile->content + right->token.location.offset,
        left->token.location.length) == 0;
}

static int IsOperatorDeclName(struct AstNode* name) {
    return name
        && name->type == AST_TOKEN
        && name->token.type == TOKEN_OPERATOR
        && name->token.value.operator != OPERATOR_SCOPE
        && name->token.value.operator != OPERATOR_ASSIGN;
}

static int DeclIsMethod(struct KekDecl* decl) {
    struct AstNode* type = decl ? decl->type : NULL;
    return decl
        && decl->kind == KEK_DECL_FUNCTION
        && type
        && type->nextSibling
        && type->nextSibling->nextSibling
        && type->nextSibling->nextSibling->nextSibling
        && type->nextSibling->nextSibling->nextSibling->type == AST_TOKEN
        && type->nextSibling->nextSibling->nextSibling->token.type == TOKEN_OPERATOR
        && type->nextSibling->nextSibling->nextSibling->token.value.operator == OPERATOR_SCOPE;
}

static struct AstNode* DeclReceiverName(struct KekDecl* decl) {
    return DeclIsMethod(decl) ? decl->type->nextSibling->nextSibling : NULL;
}

static size_t DeclParamCount(struct KekDecl* decl) {
    size_t count = 0;
    for (struct KekParam* param = decl ? decl->firstParam : NULL; param; param = param->next) {
        count++;
    }
    return count;
}

static int SameFunctionSymbolSlot(struct KekDecl* left, struct KekDecl* right, struct SourceFile* file) {
    if (!left || !right || left->kind != KEK_DECL_FUNCTION || right->kind != KEK_DECL_FUNCTION) {
        return 0;
    }
    if (!SameSymbolNameInFile(left->name, right->name, file)) {
        return 0;
    }

    struct AstNode* leftReceiver = DeclReceiverName(left);
    struct AstNode* rightReceiver = DeclReceiverName(right);
    if ((leftReceiver || rightReceiver) && !SameSymbolNameInFile(leftReceiver, rightReceiver, file)) {
        return 0;
    }

    if (IsOperatorDeclName(left->name) || IsOperatorDeclName(right->name)) {
        return DeclParamCount(left) == DeclParamCount(right);
    }
    return 1;
}

static struct KekScope* AddScope(struct KekProgram* program, enum KekScopeKind kind, struct SourceFile* file, struct KekScope* parent) {
    if (program->scopeCount >= program->scopeCapacity) {
        KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
            file ? file->fileIndex : -1, (struct SourceLocation){0}, "symbol scope storage capacity exceeded");
        program->errorCount++;
        return NULL;
    }

    struct KekScope* scope = &program->scopes[program->scopeCount++];
    memset(scope, 0, sizeof(*scope));
    scope->kind = kind;
    scope->file = file;
    scope->parent = parent;
    if (kind < KEK_SCOPE_COUNT) {
        program->scopeKindCounts[kind]++;
    }
    return scope;
}

static struct KekSymbol* ScopeFindDuplicate(struct KekScope* scope, struct AstNode* name, struct KekDecl* decl) {
    if (!scope || !name) {
        return NULL;
    }

    for (struct KekSymbol* symbol = scope->firstSymbol; symbol; symbol = symbol->nextInScope) {
        if (decl && symbol->decl && SameFunctionSymbolSlot(symbol->decl, decl, scope->file)) {
            return symbol;
        }
        if ((!decl || !symbol->decl || decl->kind != KEK_DECL_FUNCTION || symbol->decl->kind != KEK_DECL_FUNCTION)
            && SameSymbolNameInFile(symbol->name, name, scope->file)) {
            return symbol;
        }
    }
    return NULL;
}

static int AddSymbolWithFile(struct KekProgram* program, struct KekScope* scope, enum KekSymbolKind kind, struct AstNode* name, struct KekDecl* decl, struct KekParam* param, struct KekStmt* stmt, struct SourceFile* file) {
    if (!scope) {
        return -1;
    }
    if (program->symbolCount >= program->symbolCapacity) {
        KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
            scope->file ? scope->file->fileIndex : -1, name ? name->location : (struct SourceLocation){0},
            "symbol storage capacity exceeded");
        program->errorCount++;
        return -1;
    }
    struct KekSymbol* existing = name ? ScopeFindDuplicate(scope, name, decl) : NULL;
    if (existing) {
        KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
            scope->file ? scope->file->fileIndex : -1, name->location, "duplicate symbol");
        if (existing->name) {
            KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_NOTE, KEK_PHASE_SEMANTIC,
                existing->file ? existing->file->fileIndex : -1, existing->name->location, "previously defined here");
        }
        program->errorCount++;
        return -1;
    }

    struct KekSymbol* symbol = &program->symbols[program->symbolCount++];
    memset(symbol, 0, sizeof(*symbol));
    symbol->kind = kind;
    symbol->file = file ? file : scope->file;
    symbol->decl = decl;
    symbol->param = param;
    symbol->stmt = stmt;
    symbol->name = name;
    symbol->scope = scope;

    if (scope->lastSymbol) {
        scope->lastSymbol->nextInScope = symbol;
    } else {
        scope->firstSymbol = symbol;
    }
    scope->lastSymbol = symbol;
    scope->symbolCount++;

    if (kind < KEK_SYMBOL_COUNT) {
        scope->symbolKindCounts[kind]++;
        program->symbolKindCounts[kind]++;
    }
    return 0;
}

static int AddSymbol(struct KekProgram* program, struct KekScope* scope, enum KekSymbolKind kind, struct AstNode* name, struct KekDecl* decl, struct KekParam* param, struct KekStmt* stmt) {
    return AddSymbolWithFile(program, scope, kind, name, decl, param, stmt, NULL);
}

static int IsTokenText(struct AstNode* node, struct SourceFile* file, const char* text) {
    size_t length = strlen(text);
    return node
        && file
        && node->token.location.length == length
        && strncmp(file->content + node->token.location.offset, text, length) == 0;
}

static int IsBuiltinType(struct AstNode* name, struct SourceFile* file) {
    return IsTokenText(name, file, "void")
        || IsTokenText(name, file, "bool")
        || IsTokenText(name, file, "byte")
        || IsTokenText(name, file, "char")
        || IsTokenText(name, file, "str")
        || IsTokenText(name, file, "int")
        || IsTokenText(name, file, "uint")
        || IsTokenText(name, file, "i8")
        || IsTokenText(name, file, "i16")
        || IsTokenText(name, file, "i32")
        || IsTokenText(name, file, "i64")
        || IsTokenText(name, file, "u8")
        || IsTokenText(name, file, "u16")
        || IsTokenText(name, file, "u32")
        || IsTokenText(name, file, "u64")
        || IsTokenText(name, file, "f32")
        || IsTokenText(name, file, "f64")
        || IsTokenText(name, file, "size")
        || IsTokenText(name, file, "usize")
        || IsTokenText(name, file, "isize")
        || IsTokenText(name, file, "uptr")
        || IsTokenText(name, file, "iptr");
}

static struct KekSymbol* LookupSymbol(struct KekScope* scope, struct AstNode* name) {
    for (struct KekScope* current = scope; current; current = current->parent) {
        for (struct KekSymbol* symbol = current->firstSymbol; symbol; symbol = symbol->nextInScope) {
            if (SameSymbolNameAcrossFiles(symbol->name, symbol->file, name, scope->file)) {
                return symbol;
            }
        }
    }
    return NULL;
}

static int ProgramScopeHasSymbol(struct KekScope* programScope, struct AstNode* name, struct SourceFile* file) {
    for (struct KekSymbol* symbol = programScope ? programScope->firstSymbol : NULL; symbol; symbol = symbol->nextInScope) {
        if (SameSymbolNameAcrossFiles(symbol->name, symbol->file, name, file)) {
            return 1;
        }
    }
    return 0;
}

static int AddExternCStructSymbols(struct KekProgram* program, struct KekScope* programScope, struct KekDecl* decl, struct SourceFile* file) {
    if (!decl || decl->kind != KEK_DECL_EXTERN_C || !decl->body || decl->body->type != AST_BLOCK) {
        return 0;
    }

    for (struct AstNode* statement = decl->body->firstChild; statement; statement = statement->nextSibling) {
        struct AstNode* keyword = statement->firstChild;
        struct AstNode* name = keyword ? keyword->nextSibling : NULL;
        if (!keyword
            || !name
            || keyword->type != AST_TOKEN
            || keyword->token.type != TOKEN_KEYWORD
            || keyword->token.value.keyword != KEYWORD_STRUCT
            || name->type != AST_TOKEN
            || name->token.type != TOKEN_IDENTIFIER) {
            continue;
        }
        if (ProgramScopeHasSymbol(programScope, name, file)) {
            continue;
        }
        if (AddSymbolWithFile(program, programScope, KEK_SYMBOL_TYPE, name, decl, NULL, NULL, file) != 0) {
            return -1;
        }
    }

    return 0;
}

static int IsAssignableExpr(struct KekExpr* expr) {
    return expr
        && (expr->kind == KEK_EXPR_NAME
            || expr->kind == KEK_EXPR_FIELD
            || expr->kind == KEK_EXPR_SCOPE
            || expr->kind == KEK_EXPR_INDEX);
}

static void CheckTypeSemantics(struct KekProgram* program, struct KekScope* scope, struct KekType* type) {
    if (!type || !scope) {
        return;
    }

    if (type->kind == KEK_TYPE_NAME && type->name) {
        if (!IsBuiltinType(type->name, scope->file) && !LookupSymbol(scope, type->name)) {
            KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                scope->file ? scope->file->fileIndex : -1, type->name->location, "unknown type");
            program->errorCount++;
        }
    }

    // Recursively check element types (for pointers, arrays, etc.)
    if (type->element) {
        CheckTypeSemantics(program, scope, type->element);
    }
}

static void CheckExprSemantics(struct KekProgram* program, struct KekScope* scope, struct KekExpr* expr, int allowUnresolvedName) {
    if (!expr || !scope) {
        return;
    }
    program->semanticCheckCount++;

    switch (expr->kind) {
        case KEK_EXPR_NAME:
            if (!allowUnresolvedName
                && !IsTokenText(expr->token, scope->file, "this")
                && !LookupSymbol(scope, expr->token)) {
                KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                    scope->file ? scope->file->fileIndex : -1, expr->location, "unresolved name");
                program->errorCount++;
            }
            break;
        case KEK_EXPR_CALL:
            if (!expr->callee) {
                KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                    scope->file ? scope->file->fileIndex : -1, expr->location, "call without callee");
                program->errorCount++;
            }
            CheckExprSemantics(program, scope, expr->callee, 1);
            for (struct KekExpr* arg = expr->firstArg; arg; arg = arg->next) {
                if (arg->kind == KEK_EXPR_ASSIGN && arg->left && arg->left->kind == KEK_EXPR_NAME) {
                    program->semanticCheckCount++;
                    CheckExprSemantics(program, scope, arg->right, 0);
                } else {
                    CheckExprSemantics(program, scope, arg, 0);
                }
            }
            break;
        case KEK_EXPR_FIELD:
            CheckExprSemantics(program, scope, expr->left, 0);
            break;
        case KEK_EXPR_SCOPE:
            CheckExprSemantics(program, scope, expr->left, 1);
            break;
        case KEK_EXPR_INDEX:
            CheckExprSemantics(program, scope, expr->left, 0);
            CheckExprSemantics(program, scope, expr->right, 0);
            break;
        case KEK_EXPR_UNARY:
        case KEK_EXPR_CAST:
        case KEK_EXPR_GROUP:
            CheckExprSemantics(program, scope, expr->right, 0);
            break;
        case KEK_EXPR_SIZEOF:
        case KEK_EXPR_ALIGNOF:
        case KEK_EXPR_OFFSETOF:
            // Type-based builtins - type is checked separately, right may be field name
            break;
        case KEK_EXPR_LEN:
            CheckExprSemantics(program, scope, expr->right, 0);
            break;
        case KEK_EXPR_RANGE:
            // Range expression: check start, end, and optional step
            CheckExprSemantics(program, scope, expr->left, 0);
            CheckExprSemantics(program, scope, expr->right, 0);
            if (expr->step) {
                CheckExprSemantics(program, scope, expr->step, 0);
            }
            break;
        case KEK_EXPR_STRUCT_LITERAL:
            for (struct KekExpr* arg = expr->firstArg; arg; arg = arg->next) {
                if (arg->kind == KEK_EXPR_ASSIGN && arg->left && arg->left->kind == KEK_EXPR_NAME) {
                    program->semanticCheckCount++;
                    CheckExprSemantics(program, scope, arg->right, 0);
                } else {
                    CheckExprSemantics(program, scope, arg, 0);
                }
            }
            break;
        case KEK_EXPR_BINARY:
            CheckExprSemantics(program, scope, expr->left, 0);
            CheckExprSemantics(program, scope, expr->right, 0);
            break;
        case KEK_EXPR_ASSIGN:
            if (!IsAssignableExpr(expr->left)) {
                KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                    scope->file ? scope->file->fileIndex : -1, expr->location, "assignment target is not assignable");
                program->errorCount++;
            }
            CheckExprSemantics(program, scope, expr->left, 0);
            CheckExprSemantics(program, scope, expr->right, 0);
            break;
        case KEK_EXPR_NUMBER:
        case KEK_EXPR_STRING:
        case KEK_EXPR_BOOL:
        case KEK_EXPR_UNKNOWN:
        case KEK_EXPR_COUNT:
            break;
    }
}

static void BuildStmtSymbols(struct KekProgram* program, struct KekScope* scope, struct KekStmt* stmt, int inFunction, int loopDepth, int switchDepth);

static void BuildChildStmtSymbols(struct KekProgram* program, struct KekScope* scope, struct KekStmt* firstStmt, int inFunction, int loopDepth, int switchDepth) {
    for (struct KekStmt* child = firstStmt; child; child = child->next) {
        BuildStmtSymbols(program, scope, child, inFunction, loopDepth, switchDepth);
    }
}

static void BuildStmtSymbols(struct KekProgram* program, struct KekScope* scope, struct KekStmt* stmt, int inFunction, int loopDepth, int switchDepth) {
    if (!stmt || !scope) {
        return;
    }

    if (stmt->kind == KEK_STMT_BLOCK) {
        struct KekScope* blockScope = AddScope(program, KEK_SCOPE_BLOCK, scope->file, scope);
        if (blockScope) {
            blockScope->stmt = stmt;
            BuildChildStmtSymbols(program, blockScope, stmt->firstChild, inFunction, loopDepth, switchDepth);
        }
        return;
    }

    if (stmt->kind == KEK_STMT_FOR) {
        struct KekScope* loopScope = AddScope(program, KEK_SCOPE_LOOP, scope->file, scope);
        if (loopScope) {
            loopScope->stmt = stmt;
            if (stmt->initStmt) {
                BuildStmtSymbols(program, loopScope, stmt->initStmt, inFunction, loopDepth + 1, switchDepth);
            }
            CheckExprSemantics(program, loopScope, stmt->expr, 0);
            CheckExprSemantics(program, loopScope, stmt->condition, 0);
            CheckExprSemantics(program, loopScope, stmt->step, 0);
            BuildChildStmtSymbols(program, loopScope, stmt->firstChild, inFunction, loopDepth + 1, switchDepth);
        }
        return;
    }

    if (stmt->kind == KEK_STMT_EACH) {
        struct KekScope* loopScope = AddScope(program, KEK_SCOPE_LOOP, scope->file, scope);
        if (loopScope) {
            loopScope->stmt = stmt;
            // Check the iterable expression
            CheckExprSemantics(program, loopScope, stmt->expr, 0);
            // Check types (indexType and declType are parsed types, not expressions)
            CheckTypeSemantics(program, loopScope, stmt->indexType);
            CheckTypeSemantics(program, loopScope, stmt->declType);
            // Add loop variables as local symbols in the loop scope
            if (stmt->indexName) {
                (void)AddSymbol(program, loopScope, KEK_SYMBOL_LOCAL, stmt->indexName, NULL, NULL, stmt);
            }
            if (stmt->declName) {
                (void)AddSymbol(program, loopScope, KEK_SYMBOL_LOCAL, stmt->declName, NULL, NULL, stmt);
            }
            BuildChildStmtSymbols(program, loopScope, stmt->firstChild, inFunction, loopDepth + 1, switchDepth);
        }
        return;
    }

    if (stmt->kind == KEK_STMT_DECL) {
        CheckTypeSemantics(program, scope, stmt->declType);
        (void)AddSymbol(program, scope, KEK_SYMBOL_LOCAL, stmt->declName, NULL, NULL, stmt);
    }

    if (stmt->kind == KEK_STMT_RETURN && !inFunction) {
        KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
            scope->file ? scope->file->fileIndex : -1, stmt->location, "return outside function");
        program->errorCount++;
    }
    if (stmt->kind == KEK_STMT_BREAK && loopDepth == 0 && switchDepth == 0) {
        KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
            scope->file ? scope->file->fileIndex : -1, stmt->location, "break outside loop or switch");
        program->errorCount++;
    }
    if (stmt->kind == KEK_STMT_CONTINUE && loopDepth == 0) {
        KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
            scope->file ? scope->file->fileIndex : -1, stmt->location, "continue outside loop");
        program->errorCount++;
    }
    if (stmt->kind == KEK_STMT_DEFER && !inFunction) {
        KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
            scope->file ? scope->file->fileIndex : -1, stmt->location, "defer outside function");
        program->errorCount++;
    }

    CheckExprSemantics(program, scope, stmt->expr, 0);
    CheckExprSemantics(program, scope, stmt->condition, 0);
    CheckExprSemantics(program, scope, stmt->step, 0);

    if (stmt->kind == KEK_STMT_WHILE || stmt->kind == KEK_STMT_DO_WHILE || stmt->kind == KEK_STMT_EACH) {
        BuildChildStmtSymbols(program, scope, stmt->firstChild, inFunction, loopDepth + 1, switchDepth);
    } else if (stmt->kind == KEK_STMT_SWITCH) {
        BuildChildStmtSymbols(program, scope, stmt->firstChild, inFunction, loopDepth, switchDepth + 1);
    } else {
        BuildChildStmtSymbols(program, scope, stmt->firstChild, inFunction, loopDepth, switchDepth);
    }
}

static void BuildFunctionSymbols(struct KekProgram* program, struct KekScope* moduleScope, struct KekDecl* decl) {
    struct KekScope* functionScope = AddScope(program, KEK_SCOPE_FUNCTION, moduleScope->file, moduleScope);
    if (!functionScope) {
        return;
    }
    functionScope->decl = decl;

    for (struct AstNode* param = decl->genericParams ? decl->genericParams->firstChild : NULL; param; param = param->nextSibling) {
        struct AstNode* name = param->firstChild;
        if (name) {
            (void)AddSymbol(program, functionScope, KEK_SYMBOL_TYPE, name, decl, NULL, NULL);
        }
    }

    for (struct KekParam* param = decl->firstParam; param; param = param->next) {
        (void)AddSymbol(program, functionScope, KEK_SYMBOL_PARAM, param->name, NULL, param, NULL);
    }

    BuildChildStmtSymbols(program, functionScope, decl->firstStmt, 1, 0, 0);
}

int BuildKekProgramSymbols(struct KekProgram* program, struct KekModule* modules, size_t moduleCount) {
    if (!program || !modules) {
        return -1;
    }

    struct KekScope* programScope = AddScope(program, KEK_SCOPE_PROGRAM, NULL, NULL);
    if (!programScope) {
        return -1;
    }

    for (size_t moduleIndex = 0; moduleIndex < moduleCount; moduleIndex++) {
        struct KekModule* module = &modules[moduleIndex];
        for (struct KekDecl* decl = module->firstDecl; decl; decl = decl->next) {
            enum KekSymbolKind kind = SymbolKindForDecl(decl->kind);
            if (kind == KEK_SYMBOL_UNKNOWN || kind == KEK_SYMBOL_IMPORT || !decl->name) {
                if (decl->kind == KEK_DECL_EXTERN_C && AddExternCStructSymbols(program, programScope, decl, module->file) != 0) {
                    return -1;
                }
                continue;
            }
            if (ProgramScopeHasSymbol(programScope, decl->name, module->file)) {
                continue;
            }
            if (AddSymbolWithFile(program, programScope, kind, decl->name, decl, NULL, NULL, module->file) != 0) {
                return -1;
            }
        }
    }

    for (size_t moduleIndex = 0; moduleIndex < moduleCount; moduleIndex++) {
        struct KekModule* module = &modules[moduleIndex];
        struct KekScope* moduleScope = AddScope(program, KEK_SCOPE_MODULE, module->file, programScope);
        if (!moduleScope) {
            return -1;
        }

        for (struct KekDecl* decl = module->firstDecl; decl; decl = decl->next) {
            enum KekSymbolKind kind = SymbolKindForDecl(decl->kind);
            if (AddSymbol(program, moduleScope, kind, decl->name, decl, NULL, NULL) != 0) {
                return -1;
            }
            if (decl->kind == KEK_DECL_FUNCTION) {
                BuildFunctionSymbols(program, moduleScope, decl);
            }
        }
    }

    return program->errorCount == 0 ? 0 : -1;
}
