#include "kek.h"

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

static int SameSymbolName(struct AstNode* left, struct AstNode* right, struct SourceFile* file) {
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
    if (!SameSymbolName(left->name, right->name, file)) {
        return 0;
    }

    struct AstNode* leftReceiver = DeclReceiverName(left);
    struct AstNode* rightReceiver = DeclReceiverName(right);
    if ((leftReceiver || rightReceiver) && !SameSymbolName(leftReceiver, rightReceiver, file)) {
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
            && SameSymbolName(symbol->name, name, scope->file)) {
            return symbol;
        }
    }
    return NULL;
}

static int AddSymbol(struct KekProgram* program, struct KekScope* scope, enum KekSymbolKind kind, struct AstNode* name, struct KekDecl* decl, struct KekParam* param, struct KekStmt* stmt) {
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
    symbol->file = scope->file;
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
            if (SameSymbolName(symbol->name, name, current->file)) {
                return symbol;
            }
        }
    }
    return NULL;
}

static int IsAssignableExpr(struct KekExpr* expr) {
    return expr
        && (expr->kind == KEK_EXPR_NAME
            || expr->kind == KEK_EXPR_FIELD
            || expr->kind == KEK_EXPR_SCOPE
            || expr->kind == KEK_EXPR_INDEX);
}

// ============================================================================
// Type Utilities for Semantic Analysis
// ============================================================================

static void CopyTokenTextToBuffer(struct AstNode* node, struct SourceFile* file, char* buffer, size_t bufferSize) {
    if (!node || !file || !buffer || bufferSize == 0) {
        if (buffer && bufferSize > 0) buffer[0] = '\0';
        return;
    }
    size_t length = node->token.location.length;
    if (length >= bufferSize) length = bufferSize - 1;
    memcpy(buffer, file->content + node->token.location.offset, length);
    buffer[length] = '\0';
}

static void FormatTypeName(struct KekType* type, struct SourceFile* file, char* buffer, size_t bufferSize) {
    if (!buffer || bufferSize == 0) return;
    buffer[0] = '\0';
    
    if (!type) {
        snprintf(buffer, bufferSize, "<unknown>");
        return;
    }
    
    if (type->kind == KEK_TYPE_ARRAY && type->element) {
        char elemName[64];
        FormatTypeName(type->element, file, elemName, sizeof(elemName));
        snprintf(buffer, bufferSize, "%s[]", elemName);
        return;
    }
    
    if (type->kind == KEK_TYPE_POINTER && type->element) {
        char elemName[64];
        FormatTypeName(type->element, file, elemName, sizeof(elemName));
        snprintf(buffer, bufferSize, "ptr<%s>", elemName);
        return;
    }
    
    if (type->name) {
        CopyTokenTextToBuffer(type->name, file, buffer, bufferSize);
    } else {
        snprintf(buffer, bufferSize, "<unknown>");
    }
}

// Returns integer rank: i8/u8=1, i16/u16=2, i32/u32/int/uint=3, i64/u64=4, 0 for non-integer
static int IntegerTypeRank(struct KekType* type, struct SourceFile* file) {
    if (!type || !type->name || !file) return 0;
    if (IsTokenText(type->name, file, "i8") || IsTokenText(type->name, file, "u8") || IsTokenText(type->name, file, "byte")) return 1;
    if (IsTokenText(type->name, file, "i16") || IsTokenText(type->name, file, "u16")) return 2;
    if (IsTokenText(type->name, file, "i32") || IsTokenText(type->name, file, "u32") || 
        IsTokenText(type->name, file, "int") || IsTokenText(type->name, file, "uint")) return 3;
    if (IsTokenText(type->name, file, "i64") || IsTokenText(type->name, file, "u64") ||
        IsTokenText(type->name, file, "size") || IsTokenText(type->name, file, "usize") ||
        IsTokenText(type->name, file, "isize") || IsTokenText(type->name, file, "uptr") ||
        IsTokenText(type->name, file, "iptr")) return 4;
    return 0;
}

static int IsSignedIntegerType(struct KekType* type, struct SourceFile* file) {
    if (!type || !type->name || !file) return 0;
    return IsTokenText(type->name, file, "i8") || IsTokenText(type->name, file, "i16") ||
           IsTokenText(type->name, file, "i32") || IsTokenText(type->name, file, "i64") ||
           IsTokenText(type->name, file, "int") || IsTokenText(type->name, file, "isize");
}

static int IsUnsignedIntegerType(struct KekType* type, struct SourceFile* file) {
    if (!type || !type->name || !file) return 0;
    return IsTokenText(type->name, file, "u8") || IsTokenText(type->name, file, "u16") ||
           IsTokenText(type->name, file, "u32") || IsTokenText(type->name, file, "u64") ||
           IsTokenText(type->name, file, "uint") || IsTokenText(type->name, file, "byte") ||
           IsTokenText(type->name, file, "size") || IsTokenText(type->name, file, "usize") ||
           IsTokenText(type->name, file, "uptr") || IsTokenText(type->name, file, "iptr");
}

static int IsIntegerType(struct KekType* type, struct SourceFile* file) {
    return IsSignedIntegerType(type, file) || IsUnsignedIntegerType(type, file);
}

static int IsFloatType(struct KekType* type, struct SourceFile* file) {
    if (!type || !type->name || !file) return 0;
    return IsTokenText(type->name, file, "f32") || IsTokenText(type->name, file, "f64");
}

static int IsNumericType(struct KekType* type, struct SourceFile* file) {
    return IsIntegerType(type, file) || IsFloatType(type, file);
}

static int IsVoidType(struct KekType* type, struct SourceFile* file) {
    if (!type || !type->name || !file) return 0;
    return IsTokenText(type->name, file, "void");
}

static int IsBoolType(struct KekType* type, struct SourceFile* file) {
    if (!type || !type->name || !file) return 0;
    return IsTokenText(type->name, file, "bool");
}

static int IsStringType(struct KekType* type, struct SourceFile* file) {
    if (!type || !type->name || !file) return 0;
    return IsTokenText(type->name, file, "str");
}

static int IsPointerType(struct KekType* type, struct SourceFile* file) {
    if (!type) return 0;
    if (type->kind == KEK_TYPE_POINTER) return 1;
    if (type->name && IsTokenText(type->name, file, "ptr")) return 1;
    return 0;
}

static int IsVoidPointerType(struct KekType* type, struct SourceFile* file) {
    if (!type || !type->name || !file) return 0;
    // "ptr" without element is void pointer
    if (IsTokenText(type->name, file, "ptr") && !type->element) return 1;
    return 0;
}

static int IsBoolOrNumericType(struct KekType* type, struct SourceFile* file) {
    return IsBoolType(type, file) || IsNumericType(type, file) || IsPointerType(type, file);
}

// Check if two types are exactly equal
static int TypesEqual(struct KekType* a, struct KekType* b, struct SourceFile* file) {
    if (!a && !b) return 1;
    if (!a || !b) return 0;
    
    // Array types
    if (a->kind == KEK_TYPE_ARRAY && b->kind == KEK_TYPE_ARRAY) {
        return TypesEqual(a->element, b->element, file);
    }
    
    // Pointer types
    if (a->kind == KEK_TYPE_POINTER && b->kind == KEK_TYPE_POINTER) {
        return TypesEqual(a->element, b->element, file);
    }
    
    // Named types - compare names
    if (a->name && b->name) {
        if (a->name->token.location.length != b->name->token.location.length) return 0;
        return strncmp(file->content + a->name->token.location.offset,
                       file->content + b->name->token.location.offset,
                       a->name->token.location.length) == 0;
    }
    
    return 0;
}

// Check if 'from' can be implicitly converted to 'to' (widening only, no cross-sign)
static int TypesImplicitlyCompatible(struct KekType* from, struct KekType* to, struct SourceFile* file) {
    if (!from || !to) return 1;  // Allow if types unknown (avoid cascading errors)
    
    // Same types always compatible
    if (TypesEqual(from, to, file)) return 1;
    
    // Integer widening within same signedness
    if (IsSignedIntegerType(from, file) && IsSignedIntegerType(to, file)) {
        int fromRank = IntegerTypeRank(from, file);
        int toRank = IntegerTypeRank(to, file);
        return fromRank > 0 && toRank > 0 && fromRank <= toRank;
    }
    
    if (IsUnsignedIntegerType(from, file) && IsUnsignedIntegerType(to, file)) {
        int fromRank = IntegerTypeRank(from, file);
        int toRank = IntegerTypeRank(to, file);
        return fromRank > 0 && toRank > 0 && fromRank <= toRank;
    }
    
    // Any pointer to void pointer (ptr)
    if (IsPointerType(from, file) && IsVoidPointerType(to, file)) {
        return 1;
    }
    
    // Array to pointer of same element type
    if (from->kind == KEK_TYPE_ARRAY && IsPointerType(to, file)) {
        if (to->element && TypesEqual(from->element, to->element, file)) return 1;
        if (IsVoidPointerType(to, file)) return 1;
    }
    
    return 0;
}

// Check if explicit cast from 'from' to 'to' is valid
static int TypesExplicitlyCastable(struct KekType* from, struct KekType* to, struct SourceFile* file) {
    if (!from || !to) return 1;  // Allow if types unknown
    
    // Implicit compatibility implies castable
    if (TypesImplicitlyCompatible(from, to, file)) return 1;
    
    // All numeric conversions are explicitly castable
    if (IsNumericType(from, file) && IsNumericType(to, file)) return 1;
    
    // Integer to/from pointer
    if (IsIntegerType(from, file) && IsPointerType(to, file)) return 1;
    if (IsPointerType(from, file) && IsIntegerType(to, file)) return 1;
    
    // Pointer to pointer
    if (IsPointerType(from, file) && IsPointerType(to, file)) return 1;
    
    // Bool to/from integer
    if (IsBoolType(from, file) && IsIntegerType(to, file)) return 1;
    if (IsIntegerType(from, file) && IsBoolType(to, file)) return 1;
    
    return 0;
}

// Forward declarations for type resolution
static struct KekType* ResolveExprType(struct KekProgram* program, struct KekScope* scope, struct KekExpr* expr);
static struct KekDecl* LookupTypeDecl(struct KekScope* scope, struct KekType* type);

// Find function declaration from scope (for return type checking)
static struct KekType* GetFunctionReturnType(struct KekScope* scope) {
    for (struct KekScope* s = scope; s; s = s->parent) {
        if (s->kind == KEK_SCOPE_FUNCTION && s->decl && s->decl->parsedType) {
            return s->decl->parsedType;
        }
    }
    return NULL;
}

// Find a struct/union declaration and check if it has a field
static int DeclHasField(struct KekDecl* decl, struct AstNode* fieldName, struct SourceFile* file) {
    if (!decl || !fieldName) return 0;
    for (struct KekField* field = decl->firstField; field; field = field->next) {
        if (field->name && SameSymbolName(field->name, fieldName, file)) {
            return 1;
        }
        // Check nested struct fields
        if (field->isNestedStruct) {
            for (struct KekField* nested = field->nestedFields; nested; nested = nested->next) {
                if (nested->name && SameSymbolName(nested->name, fieldName, file)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Find field type from struct/union declaration
static struct KekType* GetFieldType(struct KekDecl* decl, struct AstNode* fieldName, struct SourceFile* file) {
    if (!decl || !fieldName) return NULL;
    for (struct KekField* field = decl->firstField; field; field = field->next) {
        if (field->name && SameSymbolName(field->name, fieldName, file)) {
            return field->type;
        }
    }
    return NULL;
}

// Look up type declaration (struct, union, enum) from type
static struct KekDecl* LookupTypeDecl(struct KekScope* scope, struct KekType* type) {
    if (!scope || !type || !type->name) return NULL;
    struct KekSymbol* sym = LookupSymbol(scope, type->name);
    if (sym && sym->decl && (sym->decl->kind == KEK_DECL_STRUCT || 
        sym->decl->kind == KEK_DECL_UNION || sym->decl->kind == KEK_DECL_ENUM)) {
        return sym->decl;
    }
    return NULL;
}

// Look up function symbol from callee expression
static struct KekSymbol* LookupCalleeSymbol(struct KekScope* scope, struct KekExpr* callee) {
    if (!scope || !callee) return NULL;
    
    if (callee->kind == KEK_EXPR_NAME && callee->token) {
        return LookupSymbol(scope, callee->token);
    }
    
    // For scope expressions like Package::Function or Type::Method
    if (callee->kind == KEK_EXPR_SCOPE && callee->right && callee->right->token) {
        // This is simplified - full resolution would need to look up in the type's scope
        return LookupSymbol(scope, callee->right->token);
    }
    
    return NULL;
}

// Count function arguments
static size_t CountExprArgs(struct KekExpr* call) {
    size_t count = 0;
    for (struct KekExpr* arg = call ? call->firstArg : NULL; arg; arg = arg->next) {
        count++;
    }
    return count;
}

// Count required params (without defaults)
static size_t CountRequiredParams(struct KekDecl* decl) {
    size_t count = 0;
    for (struct KekParam* p = decl ? decl->firstParam : NULL; p; p = p->next) {
        if (!p->defaultValue) count++;
    }
    return count;
}

// Count total params
static size_t CountTotalParams(struct KekDecl* decl) {
    size_t count = 0;
    for (struct KekParam* p = decl ? decl->firstParam : NULL; p; p = p->next) {
        count++;
    }
    return count;
}

// Create a synthetic type for builtin types (used when resolving literal types)
static struct KekType* CreateBuiltinType(struct KekProgram* program, const char* name) {
    // We return NULL since we can't easily allocate types here
    // Type checking functions handle NULL gracefully
    (void)program;
    (void)name;
    return NULL;
}

// Resolve the type of an expression
static struct KekType* ResolveExprType(struct KekProgram* program, struct KekScope* scope, struct KekExpr* expr) {
    if (!expr || !scope) return NULL;
    
    // Return cached type if already resolved
    if (expr->resolvedType) return expr->resolvedType;
    
    struct KekType* result = NULL;
    
    switch (expr->kind) {
        case KEK_EXPR_NAME: {
            struct KekSymbol* sym = expr->token ? LookupSymbol(scope, expr->token) : NULL;
            if (sym) {
                if (sym->type) {
                    result = sym->type;
                } else if (sym->param && sym->param->type) {
                    result = sym->param->type;
                } else if (sym->stmt && sym->stmt->declType) {
                    result = sym->stmt->declType;
                } else if (sym->decl && sym->decl->parsedType) {
                    result = sym->decl->parsedType;
                }
            }
            break;
        }
        
        case KEK_EXPR_NUMBER:
            // Number literals default to i32
            result = CreateBuiltinType(program, "i32");
            break;
            
        case KEK_EXPR_STRING:
            result = CreateBuiltinType(program, "str");
            break;
            
        case KEK_EXPR_BOOL:
            result = CreateBuiltinType(program, "bool");
            break;
            
        case KEK_EXPR_CALL: {
            struct KekSymbol* funcSym = LookupCalleeSymbol(scope, expr->callee);
            if (funcSym && funcSym->decl && funcSym->decl->parsedType) {
                result = funcSym->decl->parsedType;
            }
            break;
        }
        
        case KEK_EXPR_FIELD: {
            struct KekType* leftType = ResolveExprType(program, scope, expr->left);
            if (leftType) {
                struct KekDecl* typeDecl = LookupTypeDecl(scope, leftType);
                if (typeDecl && expr->right && expr->right->token) {
                    result = GetFieldType(typeDecl, expr->right->token, scope->file);
                }
            }
            break;
        }
        
        case KEK_EXPR_INDEX: {
            struct KekType* leftType = ResolveExprType(program, scope, expr->left);
            if (leftType && leftType->kind == KEK_TYPE_ARRAY && leftType->element) {
                result = leftType->element;
            }
            break;
        }
        
        case KEK_EXPR_UNARY:
            // For most unary ops, type is same as operand
            result = ResolveExprType(program, scope, expr->right);
            break;
            
        case KEK_EXPR_BINARY: {
            // For comparison operators, result is bool
            if (expr->token && expr->token->token.type == TOKEN_OPERATOR) {
                enum OperatorType op = expr->token->token.value.operator;
                if (op == OPERATOR_EQUAL || op == OPERATOR_NOT_EQUAL ||
                    op == OPERATOR_LESS || op == OPERATOR_LESS_EQUAL ||
                    op == OPERATOR_GREATER || op == OPERATOR_GREATER_EQUAL ||
                    op == OPERATOR_LOGICAL_AND || op == OPERATOR_LOGICAL_OR) {
                    result = CreateBuiltinType(program, "bool");
                    break;
                }
            }
            // For arithmetic, use left operand type (simplified)
            result = ResolveExprType(program, scope, expr->left);
            break;
        }
        
        case KEK_EXPR_ASSIGN:
            result = ResolveExprType(program, scope, expr->left);
            break;
            
        case KEK_EXPR_CAST:
            result = expr->type;
            break;
            
        case KEK_EXPR_SIZEOF:
        case KEK_EXPR_ALIGNOF:
        case KEK_EXPR_LEN:
            result = CreateBuiltinType(program, "u64");
            break;
            
        case KEK_EXPR_STRUCT_LITERAL:
            result = expr->type;
            break;
            
        case KEK_EXPR_GROUP:
            result = ResolveExprType(program, scope, expr->right);
            break;
            
        case KEK_EXPR_SCOPE:
        case KEK_EXPR_OFFSETOF:
        case KEK_EXPR_RANGE:
        case KEK_EXPR_UNKNOWN:
        case KEK_EXPR_COUNT:
            break;
    }
    
    expr->resolvedType = result;
    return result;
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
        case KEK_EXPR_CALL: {
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
            
            // Type check: argument count and types
            struct KekSymbol* funcSym = LookupCalleeSymbol(scope, expr->callee);
            if (funcSym && funcSym->decl && funcSym->decl->kind == KEK_DECL_FUNCTION) {
                size_t argCount = CountExprArgs(expr);
                size_t requiredCount = CountRequiredParams(funcSym->decl);
                size_t totalCount = CountTotalParams(funcSym->decl);
                
                if (argCount < requiredCount) {
                    char msg[128];
                    snprintf(msg, sizeof(msg), "too few arguments: expected at least %zu, got %zu", requiredCount, argCount);
                    KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                        scope->file ? scope->file->fileIndex : -1, expr->location, msg);
                    program->errorCount++;
                } else if (argCount > totalCount) {
                    char msg[128];
                    snprintf(msg, sizeof(msg), "too many arguments: expected at most %zu, got %zu", totalCount, argCount);
                    KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                        scope->file ? scope->file->fileIndex : -1, expr->location, msg);
                    program->errorCount++;
                }
                
                // Check argument types
                struct KekParam* param = funcSym->decl->firstParam;
                for (struct KekExpr* arg = expr->firstArg; arg && param; arg = arg->next, param = param->next) {
                    struct KekExpr* actualArg = (arg->kind == KEK_EXPR_ASSIGN) ? arg->right : arg;
                    struct KekType* argType = ResolveExprType(program, scope, actualArg);
                    if (argType && param->type && !TypesImplicitlyCompatible(argType, param->type, scope->file)) {
                        char paramName[64], argName[64], msg[256];
                        FormatTypeName(param->type, scope->file, paramName, sizeof(paramName));
                        FormatTypeName(argType, scope->file, argName, sizeof(argName));
                        snprintf(msg, sizeof(msg), "argument type mismatch: expected '%s', got '%s'", paramName, argName);
                        KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                            scope->file ? scope->file->fileIndex : -1, actualArg->location, msg);
                        program->errorCount++;
                    }
                }
            }
            break;
        }
        case KEK_EXPR_FIELD: {
            CheckExprSemantics(program, scope, expr->left, 0);
            
            // Type check: field exists on struct
            struct KekType* leftType = ResolveExprType(program, scope, expr->left);
            if (leftType && leftType->kind == KEK_TYPE_NAME && leftType->name) {
                struct KekDecl* typeDecl = LookupTypeDecl(scope, leftType);
                if (typeDecl && (typeDecl->kind == KEK_DECL_STRUCT || typeDecl->kind == KEK_DECL_UNION)) {
                    if (expr->right && expr->right->token && !DeclHasField(typeDecl, expr->right->token, scope->file)) {
                        char typeName[64], fieldName[64], msg[256];
                        FormatTypeName(leftType, scope->file, typeName, sizeof(typeName));
                        CopyTokenTextToBuffer(expr->right->token, scope->file, fieldName, sizeof(fieldName));
                        snprintf(msg, sizeof(msg), "'%s' has no field named '%s'", typeName, fieldName);
                        KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                            scope->file ? scope->file->fileIndex : -1, expr->right->location, msg);
                        program->errorCount++;
                    }
                }
            }
            break;
        }
        case KEK_EXPR_SCOPE:
            CheckExprSemantics(program, scope, expr->left, 1);
            break;
        case KEK_EXPR_INDEX: {
            CheckExprSemantics(program, scope, expr->left, 0);
            CheckExprSemantics(program, scope, expr->right, 0);
            
            // Type check: index must be integer
            struct KekType* indexType = ResolveExprType(program, scope, expr->right);
            if (indexType && !IsIntegerType(indexType, scope->file)) {
                char typeName[64], msg[128];
                FormatTypeName(indexType, scope->file, typeName, sizeof(typeName));
                snprintf(msg, sizeof(msg), "array index must be integer, got '%s'", typeName);
                KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                    scope->file ? scope->file->fileIndex : -1, expr->right->location, msg);
                program->errorCount++;
            }
            break;
        }
        case KEK_EXPR_UNARY:
        case KEK_EXPR_GROUP:
            CheckExprSemantics(program, scope, expr->right, 0);
            break;
        case KEK_EXPR_CAST: {
            CheckExprSemantics(program, scope, expr->right, 0);
            
            // Type check: cast is valid
            struct KekType* fromType = ResolveExprType(program, scope, expr->right);
            struct KekType* toType = expr->type;
            if (fromType && toType && !TypesExplicitlyCastable(fromType, toType, scope->file)) {
                char fromName[64], toName[64], msg[256];
                FormatTypeName(fromType, scope->file, fromName, sizeof(fromName));
                FormatTypeName(toType, scope->file, toName, sizeof(toName));
                snprintf(msg, sizeof(msg), "invalid cast from '%s' to '%s'", fromName, toName);
                KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                    scope->file ? scope->file->fileIndex : -1, expr->location, msg);
                program->errorCount++;
            }
            break;
        }
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
        case KEK_EXPR_BINARY: {
            CheckExprSemantics(program, scope, expr->left, 0);
            CheckExprSemantics(program, scope, expr->right, 0);
            
            // Type check: operands must be valid for operator
            struct KekType* leftType = ResolveExprType(program, scope, expr->left);
            struct KekType* rightType = ResolveExprType(program, scope, expr->right);
            
            if (expr->token && expr->token->token.type == TOKEN_OPERATOR) {
                enum OperatorType op = expr->token->token.value.operator;
                
                // Arithmetic operators: +, -, *, /, %
                if (op == OPERATOR_PLUS || op == OPERATOR_MINUS || op == OPERATOR_MULTIPLY ||
                    op == OPERATOR_DIVIDE || op == OPERATOR_MODULO) {
                    if (leftType && !IsNumericType(leftType, scope->file) && !IsPointerType(leftType, scope->file)) {
                        char typeName[64], msg[128];
                        FormatTypeName(leftType, scope->file, typeName, sizeof(typeName));
                        snprintf(msg, sizeof(msg), "left operand of arithmetic operator must be numeric, got '%s'", typeName);
                        KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                            scope->file ? scope->file->fileIndex : -1, expr->left->location, msg);
                        program->errorCount++;
                    }
                    if (rightType && !IsNumericType(rightType, scope->file) && !IsPointerType(rightType, scope->file)) {
                        char typeName[64], msg[128];
                        FormatTypeName(rightType, scope->file, typeName, sizeof(typeName));
                        snprintf(msg, sizeof(msg), "right operand of arithmetic operator must be numeric, got '%s'", typeName);
                        KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                            scope->file ? scope->file->fileIndex : -1, expr->right->location, msg);
                        program->errorCount++;
                    }
                }
                
                // Logical operators: &&, ||
                if (op == OPERATOR_LOGICAL_AND || op == OPERATOR_LOGICAL_OR) {
                    if (leftType && !IsBoolOrNumericType(leftType, scope->file)) {
                        char typeName[64], msg[128];
                        FormatTypeName(leftType, scope->file, typeName, sizeof(typeName));
                        snprintf(msg, sizeof(msg), "left operand of logical operator must be bool or numeric, got '%s'", typeName);
                        KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                            scope->file ? scope->file->fileIndex : -1, expr->left->location, msg);
                        program->errorCount++;
                    }
                    if (rightType && !IsBoolOrNumericType(rightType, scope->file)) {
                        char typeName[64], msg[128];
                        FormatTypeName(rightType, scope->file, typeName, sizeof(typeName));
                        snprintf(msg, sizeof(msg), "right operand of logical operator must be bool or numeric, got '%s'", typeName);
                        KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                            scope->file ? scope->file->fileIndex : -1, expr->right->location, msg);
                        program->errorCount++;
                    }
                }
            }
            break;
        }
        case KEK_EXPR_ASSIGN: {
            if (!IsAssignableExpr(expr->left)) {
                KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                    scope->file ? scope->file->fileIndex : -1, expr->location, "assignment target is not assignable");
                program->errorCount++;
            }
            CheckExprSemantics(program, scope, expr->left, 0);
            CheckExprSemantics(program, scope, expr->right, 0);
            
            // Type check: assignment type compatibility
            struct KekType* leftType = ResolveExprType(program, scope, expr->left);
            struct KekType* rightType = ResolveExprType(program, scope, expr->right);
            if (leftType && rightType && !TypesImplicitlyCompatible(rightType, leftType, scope->file)) {
                char leftName[64], rightName[64], msg[256];
                FormatTypeName(leftType, scope->file, leftName, sizeof(leftName));
                FormatTypeName(rightType, scope->file, rightName, sizeof(rightName));
                snprintf(msg, sizeof(msg), "cannot implicitly convert '%s' to '%s' in assignment", rightName, leftName);
                KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                    scope->file ? scope->file->fileIndex : -1, expr->location, msg);
                program->errorCount++;
            }
            break;
        }
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
        
        // Type check: initializer matches declared type
        if (stmt->expr && stmt->declType) {
            struct KekType* initType = ResolveExprType(program, scope, stmt->expr);
            if (initType && !TypesImplicitlyCompatible(initType, stmt->declType, scope->file)) {
                char declName[64], initName[64], msg[256];
                FormatTypeName(stmt->declType, scope->file, declName, sizeof(declName));
                FormatTypeName(initType, scope->file, initName, sizeof(initName));
                snprintf(msg, sizeof(msg), "cannot initialize '%s' with '%s'", declName, initName);
                KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                    scope->file ? scope->file->fileIndex : -1, stmt->expr->location, msg);
                program->errorCount++;
            }
        }
    }

    if (stmt->kind == KEK_STMT_RETURN) {
        if (!inFunction) {
            KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                scope->file ? scope->file->fileIndex : -1, stmt->location, "return outside function");
            program->errorCount++;
        } else {
            // Type check: return type matches function return type
            struct KekType* expectedType = GetFunctionReturnType(scope);
            struct KekType* actualType = stmt->expr ? ResolveExprType(program, scope, stmt->expr) : NULL;
            
            int expectsVoid = expectedType && IsVoidType(expectedType, scope->file);
            
            if (expectsVoid && stmt->expr) {
                KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                    scope->file ? scope->file->fileIndex : -1, stmt->location, "void function should not return a value");
                program->errorCount++;
            } else if (!expectsVoid && !stmt->expr) {
                KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                    scope->file ? scope->file->fileIndex : -1, stmt->location, "non-void function must return a value");
                program->errorCount++;
            } else if (!expectsVoid && actualType && expectedType && !TypesImplicitlyCompatible(actualType, expectedType, scope->file)) {
                char expName[64], actName[64], msg[256];
                FormatTypeName(expectedType, scope->file, expName, sizeof(expName));
                FormatTypeName(actualType, scope->file, actName, sizeof(actName));
                snprintf(msg, sizeof(msg), "return type mismatch: expected '%s', got '%s'", expName, actName);
                KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                    scope->file ? scope->file->fileIndex : -1, stmt->location, msg);
                program->errorCount++;
            }
        }
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
    
    // Type check: condition must be bool or numeric
    if (stmt->condition && (stmt->kind == KEK_STMT_IF || stmt->kind == KEK_STMT_WHILE ||
        stmt->kind == KEK_STMT_DO_WHILE || stmt->kind == KEK_STMT_FOR)) {
        struct KekType* condType = ResolveExprType(program, scope, stmt->condition);
        if (condType && !IsBoolOrNumericType(condType, scope->file)) {
            char typeName[64], msg[128];
            FormatTypeName(condType, scope->file, typeName, sizeof(typeName));
            snprintf(msg, sizeof(msg), "condition must be bool or numeric, got '%s'", typeName);
            KekAddDiagnostic(program->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
                scope->file ? scope->file->fileIndex : -1, stmt->condition->location, msg);
            program->errorCount++;
        }
    }

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
