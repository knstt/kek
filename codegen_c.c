#include "kek.h"

struct CFunctionInfo {
    char name[64];
    char paramNames[16][64];
    char defaults[16][64];
    size_t paramCount;
};

struct CStructInfo {
    char name[64];
    char fieldNames[64][64];
    char defaults[64][64];
    size_t fieldCount;
};

struct CGenericInstance {
    struct KekDecl* decl;
    struct AstNode* args;
    struct SourceFile* declFile;
    struct SourceFile* argsFile;
    char name[128];
};

struct CWriter {
    FILE* out;
    struct SourceFile* file;
    int indent;
    struct CFunctionInfo functions[64];
    size_t functionCount;
    struct CStructInfo structs[64];
    size_t structCount;
    struct CGenericInstance genericStructs[64];
    size_t genericStructCount;
    struct CGenericInstance genericFunctions[64];
    size_t genericFunctionCount;
    char localNames[128][64];
    char localTypes[128][64];
    int localIsPointer[128];
    size_t localCount;
    int thisIsPointer;
    struct AstNode* genericParams;
    struct AstNode* genericArgs;
    struct SourceFile* genericArgFile;
    char namespacePrefix[64];
    struct KekStmt* deferred[128];
    size_t deferCount;
};

static int IsPunctuationToken(struct Token* token, enum PunctuationType punctuation) {
    return token->type == TOKEN_PUNCTUATION && token->value.punctuation == punctuation;
}

static int IsOperatorToken(struct Token* token, enum OperatorType operator) {
    return token->type == TOKEN_OPERATOR && token->value.operator == operator;
}

static int IsKeywordToken(struct Token* token, enum KeywordType keyword) {
    return token->type == TOKEN_KEYWORD && token->value.keyword == keyword;
}

static int IsTokenNode(struct AstNode* node) {
    return node && node->type == AST_TOKEN;
}

static struct AstNode* NextSibling(struct AstNode* node) {
    return node ? node->nextSibling : NULL;
}

static const char* CTokenText(struct Token* token, struct SourceFile* file, char* buffer, size_t bufferSize) {
    if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_NUMBER || token->type == TOKEN_STRING) {
        size_t length = token->location.length;
        if (length >= bufferSize) {
            length = bufferSize - 1;
        }
        memcpy(buffer, file->content + token->location.offset, length);
        buffer[length] = '\0';
        return buffer;
    }

    if (token->type == TOKEN_KEYWORD && token->value.keyword == KEYWORD_TRUE) {
        return "1";
    }

    if (token->type == TOKEN_KEYWORD && token->value.keyword == KEYWORD_FALSE) {
        return "0";
    }

    return TokenLexeme(token, file);
}

static int TokenTextEquals(struct Token* token, struct SourceFile* file, const char* text) {
    size_t length = strlen(text);
    return (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_NUMBER || token->type == TOKEN_STRING)
        && token->location.length == length
        && strncmp(file->content + token->location.offset, text, length) == 0;
}

static void CopyTokenText(struct Token* token, struct SourceFile* file, char* buffer, size_t bufferSize) {
    const char* text = CTokenText(token, file, buffer, bufferSize);
    if (text != buffer) {
        snprintf(buffer, bufferSize, "%s", text);
    }
}

static const char* OperatorMangleName(enum OperatorType operator) {
    switch (operator) {
        case OPERATOR_EQUAL:
            return "operator_equal";
        case OPERATOR_NOT_EQUAL:
            return "operator_not_equal";
        case OPERATOR_LESS_EQUAL:
            return "operator_less_equal";
        case OPERATOR_GREATER_EQUAL:
            return "operator_greater_equal";
        case OPERATOR_LOGICAL_AND:
            return "operator_logical_and";
        case OPERATOR_LOGICAL_OR:
            return "operator_logical_or";
        case OPERATOR_PLUS_ASSIGN:
            return "operator_plus_assign";
        case OPERATOR_MINUS_ASSIGN:
            return "operator_minus_assign";
        case OPERATOR_ARROW:
            return "operator_arrow";
        case OPERATOR_PLUS:
            return "operator_plus";
        case OPERATOR_MINUS:
            return "operator_minus";
        case OPERATOR_MULTIPLY:
            return "operator_multiply";
        case OPERATOR_DIVIDE:
            return "operator_divide";
        case OPERATOR_MODULO:
            return "operator_modulo";
        case OPERATOR_LESS:
            return "operator_less";
        case OPERATOR_GREATER:
            return "operator_greater";
        case OPERATOR_LOGICAL_NOT:
            return "operator_logical_not";
        case OPERATOR_BITWISE_AND:
            return "operator_bitwise_and";
        case OPERATOR_BITWISE_OR:
            return "operator_bitwise_or";
        case OPERATOR_BITWISE_NOT:
            return "operator_bitwise_not";
        case OPERATOR_SCOPE:
        case OPERATOR_ASSIGN:
        case OPERATOR_COUNT:
            return NULL;
    }
    return NULL;
}

static int IsOperatorDeclName(struct AstNode* node) {
    return IsTokenNode(node)
        && node->token.type == TOKEN_OPERATOR
        && node->token.value.operator != OPERATOR_SCOPE
        && node->token.value.operator != OPERATOR_ASSIGN;
}

static void TypedDeclBaseName(struct CWriter* writer, struct KekDecl* decl, char* buffer, size_t bufferSize) {
    if (!decl || !decl->name || bufferSize == 0) {
        if (bufferSize > 0) {
            buffer[0] = '\0';
        }
        return;
    }
    if (IsOperatorDeclName(decl->name)) {
        const char* name = OperatorMangleName(decl->name->token.value.operator);
        size_t count = 0;
        for (struct KekParam* param = decl->firstParam; param; param = param->next) {
            count++;
        }
        snprintf(buffer, bufferSize, "%s_%zu", name ? name : "operator_unknown", count);
        return;
    }
    CopyTokenText(&decl->name->token, writer->file, buffer, bufferSize);
}

static struct CFunctionInfo* FindFunctionInfo(struct CWriter* writer, const char* name) {
    for (size_t i = 0; i < writer->functionCount; i++) {
        if (strcmp(writer->functions[i].name, name) == 0) {
            return &writer->functions[i];
        }
    }
    return NULL;
}

static struct CFunctionInfo* AddFunctionInfo(struct CWriter* writer, const char* name) {
    struct CFunctionInfo* function = FindFunctionInfo(writer, name);
    if (function) {
        function->paramCount = 0;
        memset(function->paramNames, 0, sizeof(function->paramNames));
        memset(function->defaults, 0, sizeof(function->defaults));
        return function;
    }

    if (writer->functionCount >= sizeof(writer->functions) / sizeof(writer->functions[0])) {
        return NULL;
    }

    function = &writer->functions[writer->functionCount++];
    memset(function, 0, sizeof(*function));
    snprintf(function->name, sizeof(function->name), "%s", name);
    return function;
}

static int IsKnownStruct(struct CWriter* writer, const char* name) {
    for (size_t i = 0; i < writer->structCount; i++) {
        if (strcmp(writer->structs[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

static struct CStructInfo* FindStructInfo(struct CWriter* writer, const char* name) {
    for (size_t i = 0; i < writer->structCount; i++) {
        if (strcmp(writer->structs[i].name, name) == 0) {
            return &writer->structs[i];
        }
    }
    return NULL;
}

static struct CStructInfo* AddStructInfo(struct CWriter* writer, const char* name) {
    struct CStructInfo* info = FindStructInfo(writer, name);
    if (info) {
        info->fieldCount = 0;
        memset(info->fieldNames, 0, sizeof(info->fieldNames));
        memset(info->defaults, 0, sizeof(info->defaults));
        return info;
    }

    if (writer->structCount >= sizeof(writer->structs) / sizeof(writer->structs[0])) {
        return NULL;
    }

    info = &writer->structs[writer->structCount++];
    memset(info, 0, sizeof(*info));
    snprintf(info->name, sizeof(info->name), "%s", name);
    return info;
}

static int IsCIdentifierStart(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

static int IsCIdentifierPart(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

static void RegisterExternCStructs(struct CWriter* writer, struct KekDecl* decl) {
    if (!writer || !decl || !decl->body || !writer->file || decl->body->location.length < 2) {
        return;
    }

    size_t start = decl->body->location.offset + 1;
    size_t end = decl->body->location.offset + decl->body->location.length - 1;
    const char* content = writer->file->content;
    const char keyword[] = "struct";
    const size_t keywordLength = sizeof(keyword) - 1;

    for (size_t i = start; i + keywordLength < end; i++) {
        if (strncmp(content + i, keyword, keywordLength) != 0) {
            continue;
        }
        if (i > start && IsCIdentifierPart(content[i - 1])) {
            continue;
        }
        if (i + keywordLength < end && IsCIdentifierPart(content[i + keywordLength])) {
            continue;
        }

        size_t nameStart = i + keywordLength;
        while (nameStart < end && isspace((unsigned char)content[nameStart])) {
            nameStart++;
        }
        if (nameStart >= end || !IsCIdentifierStart(content[nameStart])) {
            continue;
        }

        size_t nameEnd = nameStart + 1;
        while (nameEnd < end && IsCIdentifierPart(content[nameEnd])) {
            nameEnd++;
        }

        char name[64];
        size_t length = nameEnd - nameStart;
        if (length >= sizeof(name)) {
            length = sizeof(name) - 1;
        }
        memcpy(name, content + nameStart, length);
        name[length] = '\0';
        (void)AddStructInfo(writer, name);
        i = nameEnd;
    }
}

static void AddLocalTypeEx(struct CWriter* writer, const char* name, const char* type, int isPointer) {
    for (size_t i = 0; i < writer->localCount; i++) {
        if (strcmp(writer->localNames[i], name) == 0) {
            snprintf(writer->localTypes[i], sizeof(writer->localTypes[i]), "%s", type);
            writer->localIsPointer[i] = isPointer;
            return;
        }
    }

    if (writer->localCount >= sizeof(writer->localNames) / sizeof(writer->localNames[0])) {
        return;
    }

    snprintf(writer->localNames[writer->localCount], sizeof(writer->localNames[0]), "%s", name);
    snprintf(writer->localTypes[writer->localCount], sizeof(writer->localTypes[0]), "%s", type);
    writer->localIsPointer[writer->localCount] = isPointer;
    writer->localCount++;
}

static const char* FindLocalType(struct CWriter* writer, const char* name) {
    for (size_t i = writer->localCount; i > 0; i--) {
        if (strcmp(writer->localNames[i - 1], name) == 0) {
            return writer->localTypes[i - 1];
        }
    }
    return NULL;
}

static int FindLocalIsPointer(struct CWriter* writer, const char* name) {
    for (size_t i = writer->localCount; i > 0; i--) {
        if (strcmp(writer->localNames[i - 1], name) == 0) {
            return writer->localIsPointer[i - 1];
        }
    }
    return 0;
}

static void WriteIndent(FILE* out, int indent) {
    for (int i = 0; i < indent; i++) {
        fputs("    ", out);
    }
}

static void WritePrelude(FILE* out) {
    fputs("#include <assert.h>\n", out);
    fputs("#include <stdint.h>\n", out);
    fputs("#include <stddef.h>\n\n", out);
    fputs("#include <stdbool.h>\n\n", out);
    fputs("typedef uint8_t u8;\n", out);
    fputs("typedef uint16_t u16;\n", out);
    fputs("typedef uint32_t u32;\n", out);
    fputs("typedef uint64_t u64;\n", out);
    fputs("typedef int8_t i8;\n", out);
    fputs("typedef int16_t i16;\n", out);
    fputs("typedef int32_t i32;\n", out);
    fputs("typedef int64_t i64;\n", out);
    fputs("typedef float f32;\n", out);
    fputs("typedef double f64;\n", out);
    fputs("typedef void* ptr;\n\n", out);
}

static void PackagePrefixFromPath(const char* path, char* buffer, size_t bufferSize) {
    const char* slash = strrchr(path, '/');
    if (!slash) {
        buffer[0] = '\0';
        return;
    }

    const char* end = slash;
    const char* start = end;
    while (start > path && start[-1] != '/') {
        start--;
    }

    size_t length = (size_t)(end - start);
    if (length == 0 || length + 1 >= bufferSize) {
        buffer[0] = '\0';
        return;
    }

    memcpy(buffer, start, length);
    buffer[length] = '_';
    buffer[length + 1] = '\0';
}

static const char* TypedNodeText(struct CWriter* writer, struct AstNode* node, char* buffer, size_t bufferSize) {
    if (!node || !IsTokenNode(node)) {
        if (bufferSize > 0) {
            buffer[0] = '\0';
        }
        return buffer;
    }
    return CTokenText(&node->token, writer->file, buffer, bufferSize);
}

static void CopyTypedNodeText(struct CWriter* writer, struct AstNode* node, char* buffer, size_t bufferSize) {
    const char* text = TypedNodeText(writer, node, buffer, bufferSize);
    if (text != buffer) {
        snprintf(buffer, bufferSize, "%s", text);
    }
}

static int IsGenericNode(struct AstNode* node) {
    return node && node->type == AST_GENERIC;
}

static int TypedTokenTextEquals(struct CWriter* writer, struct AstNode* node, const char* text) {
    size_t length = strlen(text);
    return node
        && IsTokenNode(node)
        && (node->token.type == TOKEN_IDENTIFIER || node->token.type == TOKEN_NUMBER || node->token.type == TOKEN_STRING)
        && node->token.location.length == length
        && strncmp(writer->file->content + node->token.location.offset, text, length) == 0;
}

static struct AstNode* GenericArgNode(struct AstNode* args, size_t index) {
    if (!IsGenericNode(args)) {
        return NULL;
    }
    struct AstNode* arg = args->firstChild;
    while (arg && index > 0) {
        arg = arg->nextSibling;
        index--;
    }
    return arg ? arg->firstChild : NULL;
}

static struct AstNode* GenericParamName(struct AstNode* params, size_t index) {
    struct AstNode* param = GenericArgNode(params, index);
    if (!param) {
        return NULL;
    }
    if (param->type == AST_INDEX) {
        param = param->nextSibling;
    }
    if (!param) {
        return NULL;
    }
    if (param->nextSibling && IsPunctuationToken(&param->nextSibling->token, PUNCTUATION_COLON)) {
        return param->nextSibling->nextSibling;
    }
    return param;
}

static struct AstNode* FindGenericSubstitution(struct CWriter* writer, struct AstNode* params, struct AstNode* args, struct AstNode* name) {
    if (!name || !IsGenericNode(params) || !IsGenericNode(args)) {
        return NULL;
    }
    size_t index = 0;
    for (struct AstNode* param = params->firstChild; param; param = param->nextSibling, index++) {
        struct AstNode* paramName = GenericParamName(params, index);
        if (paramName && TypedTokenTextEquals(writer, name, TypedNodeText(writer, paramName, (char[128]){0}, 128))) {
            return GenericArgNode(args, index);
        }
    }
    return NULL;
}

static void AppendSanitized(char* buffer, size_t bufferSize, const char* text) {
    size_t length = strlen(buffer);
    for (const char* cursor = text; *cursor && length + 1 < bufferSize; cursor++) {
        char ch = *cursor;
        if (isalnum((unsigned char)ch) || ch == '_') {
            buffer[length++] = ch;
        } else if (length == 0 || buffer[length - 1] != '_') {
            buffer[length++] = '_';
        }
    }
    buffer[length] = '\0';
}

static void GenericArgTextFromFile(struct SourceFile* file, struct AstNode* arg, char* buffer, size_t bufferSize) {
    if (bufferSize == 0) {
        return;
    }
    buffer[0] = '\0';
    if (!file || !arg) {
        return;
    }
    size_t length = arg->location.length;
    if (length >= bufferSize) {
        length = bufferSize - 1;
    }
    memcpy(buffer, file->content + arg->location.offset, length);
    buffer[length] = '\0';
}

static void GenericArgText(struct CWriter* writer, struct AstNode* arg, char* buffer, size_t bufferSize) {
    GenericArgTextFromFile(writer->genericArgFile ? writer->genericArgFile : writer->file, arg, buffer, bufferSize);
}

static void GenericArgMangleText(struct CWriter* writer, struct SourceFile* argsFile, struct AstNode* arg, char* buffer, size_t bufferSize) {
    struct AstNode* substitution = arg && IsTokenNode(arg)
        ? FindGenericSubstitution(writer, writer->genericParams, writer->genericArgs, arg)
        : NULL;
    if (substitution) {
        GenericArgText(writer, substitution, buffer, bufferSize);
        return;
    }
    if (arg && IsTokenNode(arg) && IsGenericNode(arg->nextSibling)) {
        char part[128];
        struct SourceFile* previousFile = writer->file;
        writer->file = argsFile ? argsFile : writer->file;
        CopyTypedNodeText(writer, arg, buffer, bufferSize);
        writer->file = previousFile;
        for (struct AstNode* child = arg->nextSibling->firstChild; child; child = child->nextSibling) {
            AppendSanitized(buffer, bufferSize, "__");
            GenericArgMangleText(writer, argsFile, child->firstChild, part, sizeof(part));
            AppendSanitized(buffer, bufferSize, part);
        }
        return;
    }
    GenericArgTextFromFile(argsFile ? argsFile : writer->file, arg, buffer, bufferSize);
}

static void MangleGenericNameWithFiles(struct CWriter* writer, struct SourceFile* baseFile, struct AstNode* baseName, struct SourceFile* argsFile, struct AstNode* args, char* buffer, size_t bufferSize) {
    char part[128];
    struct SourceFile* previousFile = writer->file;
    buffer[0] = '\0';
    writer->file = baseFile ? baseFile : previousFile;
    CopyTypedNodeText(writer, baseName, part, sizeof(part));
    writer->file = previousFile;
    AppendSanitized(buffer, bufferSize, part);
    for (struct AstNode* arg = IsGenericNode(args) ? args->firstChild : NULL; arg; arg = arg->nextSibling) {
        AppendSanitized(buffer, bufferSize, "__");
        GenericArgMangleText(writer, argsFile ? argsFile : previousFile, arg->firstChild, part, sizeof(part));
        AppendSanitized(buffer, bufferSize, part);
    }
}

static void AppendGenericArgsToNameWithWriter(struct CWriter* writer, struct SourceFile* argsFile, struct AstNode* args, char* buffer, size_t bufferSize) {
    char part[128];
    for (struct AstNode* arg = IsGenericNode(args) ? args->firstChild : NULL; arg; arg = arg->nextSibling) {
        AppendSanitized(buffer, bufferSize, "__");
        GenericArgMangleText(writer, argsFile, arg->firstChild, part, sizeof(part));
        AppendSanitized(buffer, bufferSize, part);
    }
}

static void MangleGenericName(struct CWriter* writer, struct AstNode* baseName, struct AstNode* args, char* buffer, size_t bufferSize) {
    MangleGenericNameWithFiles(writer, writer->file, baseName, writer->file, args, buffer, bufferSize);
}

static void WriteSourceSlice(FILE* out, struct SourceFile* file, struct SourceLocation location) {
    if (!file || location.offset >= file->length) {
        return;
    }
    size_t length = location.length;
    if (location.offset + length > file->length) {
        length = file->length - location.offset;
    }
    fwrite(file->content + location.offset, 1, length, out);
}

static void CopyExprSource(struct CWriter* writer, struct KekExpr* expr, char* buffer, size_t bufferSize) {
    if (bufferSize == 0) {
        return;
    }
    buffer[0] = '\0';
    if (!expr || !expr->source || !writer->file) {
        return;
    }
    size_t length = expr->source->location.length;
    if (length >= bufferSize) {
        length = bufferSize - 1;
    }
    memcpy(buffer, writer->file->content + expr->source->location.offset, length);
    buffer[length] = '\0';
}

static void CopyTypedFieldDefault(struct CWriter* writer, struct KekField* field, char* buffer, size_t bufferSize) {
    if (field && field->defaultValue) {
        CopyExprSource(writer, field->defaultValue, buffer, bufferSize);
        return;
    }

    struct KekType* base = field ? field->type : NULL;
    while (base && base->kind == KEK_TYPE_ARRAY) {
        base = base->element;
    }
    char typeName[128];
    if (base && base->name) {
        CopyTypedNodeText(writer, base->name, typeName, sizeof(typeName));
        if (IsKnownStruct(writer, typeName)) {
            snprintf(buffer, bufferSize, "{0}");
            return;
        }
    }

    snprintf(buffer, bufferSize, "0");
}

static void WriteTypedExpr(struct CWriter* writer, struct KekExpr* expr);

static struct KekDecl* FindGenericDecl(struct KekModule* modules, size_t count, enum KekDeclKind kind, const char* name, struct SourceFile** fileOut) {
    for (size_t i = 0; i < count; i++) {
        for (struct KekDecl* decl = modules[i].firstDecl; decl; decl = decl->next) {
            if (decl->kind != kind || !decl->genericParams || !decl->name) {
                continue;
            }
            struct SourceFile* file = modules[i].file;
            if (!file || decl->name->token.location.length != strlen(name)) {
                continue;
            }
            if (strncmp(file->content + decl->name->token.location.offset, name, decl->name->token.location.length) == 0) {
                if (fileOut) {
                    *fileOut = file;
                }
                return decl;
            }
        }
    }
    if (fileOut) {
        *fileOut = NULL;
    }
    return NULL;
}

static int TypedDeclIsMethod(struct KekDecl* decl);
static struct AstNode* TypedDeclReceiverName(struct KekDecl* decl);

static struct CGenericInstance* FindGenericInstance(struct CGenericInstance* instances, size_t count, struct KekDecl* decl, struct AstNode* args) {
    for (size_t i = 0; i < count; i++) {
        if (instances[i].decl == decl && instances[i].args == args) {
            return &instances[i];
        }
    }
    return NULL;
}

static struct CGenericInstance* FindGenericInstanceByName(struct CGenericInstance* instances, size_t count, const char* name) {
    for (size_t i = 0; i < count; i++) {
        if (strcmp(instances[i].name, name) == 0) {
            return &instances[i];
        }
    }
    return NULL;
}

static void AddGenericStructInstance(struct CWriter* writer, struct KekDecl* decl, struct SourceFile* declFile, struct AstNode* args, struct SourceFile* argsFile) {
    if (!decl || !args || FindGenericInstance(writer->genericStructs, writer->genericStructCount, decl, args)
        || writer->genericStructCount >= sizeof(writer->genericStructs) / sizeof(writer->genericStructs[0])) {
        return;
    }

    char instanceName[128];
    MangleGenericNameWithFiles(writer, declFile, decl->name, argsFile, args, instanceName, sizeof(instanceName));
    if (FindGenericInstanceByName(writer->genericStructs, writer->genericStructCount, instanceName)) {
        return;
    }

    struct CGenericInstance* instance = &writer->genericStructs[writer->genericStructCount++];
    memset(instance, 0, sizeof(*instance));
    instance->decl = decl;
    instance->args = args;
    instance->declFile = declFile;
    instance->argsFile = argsFile;
    snprintf(instance->name, sizeof(instance->name), "%s", instanceName);
    struct CStructInfo* info = AddStructInfo(writer, instance->name);
    if (info) {
        struct SourceFile* previousFile = writer->file;
        writer->file = declFile ? declFile : previousFile;
        for (struct KekField* field = decl->firstField; field && info->fieldCount < 64; field = field->next) {
            CopyTypedNodeText(writer, field->name, info->fieldNames[info->fieldCount], sizeof(info->fieldNames[info->fieldCount]));
            CopyTypedFieldDefault(writer, field, info->defaults[info->fieldCount], sizeof(info->defaults[info->fieldCount]));
            info->fieldCount++;
        }
        writer->file = previousFile;
    }
}

static void AddGenericFunctionInstance(struct CWriter* writer, struct KekDecl* decl, struct SourceFile* declFile, struct AstNode* args, struct SourceFile* argsFile) {
    if (!decl || !args || FindGenericInstance(writer->genericFunctions, writer->genericFunctionCount, decl, args)
        || writer->genericFunctionCount >= sizeof(writer->genericFunctions) / sizeof(writer->genericFunctions[0])) {
        return;
    }

    char instanceName[128];
    if (TypedDeclIsMethod(decl)) {
        char receiver[128];
        MangleGenericNameWithFiles(writer, declFile, TypedDeclReceiverName(decl), argsFile, args, receiver, sizeof(receiver));
        char method[64];
        struct SourceFile* previousFile = writer->file;
        writer->file = declFile ? declFile : previousFile;
        CopyTypedNodeText(writer, decl->name, method, sizeof(method));
        writer->file = previousFile;
        snprintf(instanceName, sizeof(instanceName), "%s_%s", receiver, method);
    } else {
        char prefix[64] = {0};
        if (declFile) {
            PackagePrefixFromPath(declFile->path, prefix, sizeof(prefix));
        }
        char base[128];
        MangleGenericNameWithFiles(writer, declFile, decl->name, argsFile, args, base, sizeof(base));
        snprintf(instanceName, sizeof(instanceName), "%s", prefix);
        AppendSanitized(instanceName, sizeof(instanceName), base);
    }
    if (FindGenericInstanceByName(writer->genericFunctions, writer->genericFunctionCount, instanceName)) {
        return;
    }

    struct CGenericInstance* instance = &writer->genericFunctions[writer->genericFunctionCount++];
    memset(instance, 0, sizeof(*instance));
    instance->decl = decl;
    instance->args = args;
    instance->declFile = declFile;
    instance->argsFile = argsFile;
    snprintf(instance->name, sizeof(instance->name), "%s", instanceName);
}

static const char* GenericStructInstanceName(struct CWriter* writer, struct KekType* type, char* buffer, size_t bufferSize) {
    if (!type || !type->name || !type->genericArgs) {
        return NULL;
    }
    MangleGenericName(writer, type->name, type->genericArgs, buffer, bufferSize);
    return buffer;
}

static void AddGenericMethodInstancesForStructs(struct CWriter* writer, struct KekModule* modules, size_t count) {
    for (size_t i = 0; i < writer->genericStructCount; i++) {
        struct CGenericInstance* structInstance = &writer->genericStructs[i];
        if (!structInstance->decl || !structInstance->decl->name) {
            continue;
        }
        char structName[64];
        struct SourceFile* previousFile = writer->file;
        writer->file = structInstance->declFile ? structInstance->declFile : previousFile;
        CopyTypedNodeText(writer, structInstance->decl->name, structName, sizeof(structName));
        writer->file = previousFile;

        for (size_t moduleIndex = 0; moduleIndex < count; moduleIndex++) {
            for (struct KekDecl* decl = modules[moduleIndex].firstDecl; decl; decl = decl->next) {
                if (!decl->genericParams || !TypedDeclIsMethod(decl)) {
                    continue;
                }
                struct SourceFile* declFile = modules[moduleIndex].file;
                previousFile = writer->file;
                writer->file = declFile ? declFile : previousFile;
                char receiver[64];
                CopyTypedNodeText(writer, TypedDeclReceiverName(decl), receiver, sizeof(receiver));
                writer->file = previousFile;
                if (strcmp(receiver, structName) == 0) {
                    AddGenericFunctionInstance(writer, decl, declFile, structInstance->args, structInstance->argsFile);
                }
            }
        }
    }
}

static void WriteTypedNonArrayTypeWithGenericContext(struct CWriter* writer, struct KekType* type, struct AstNode* params, struct AstNode* args) {
    if (!type || !type->name) {
        fputs("void", writer->out);
        return;
    }

    if (type->kind == KEK_TYPE_POINTER && type->element) {
        WriteTypedNonArrayTypeWithGenericContext(writer, type->element, params, args);
        fputc('*', writer->out);
        return;
    }

    char buffer[128];
    struct AstNode* substitution = FindGenericSubstitution(writer, params, args, type->name);
    if (substitution) {
        GenericArgMangleText(writer, writer->genericArgFile ? writer->genericArgFile : writer->file, substitution, buffer, sizeof(buffer));
        if (IsKnownStruct(writer, buffer)) {
            fprintf(writer->out, "struct %s", buffer);
        } else {
            fputs(buffer, writer->out);
        }
        return;
    }
    if (GenericStructInstanceName(writer, type, buffer, sizeof(buffer))) {
        fprintf(writer->out, "struct %s", buffer);
        return;
    }
    const char* typeName = TypedNodeText(writer, type->name, buffer, sizeof(buffer));
    if (IsKnownStruct(writer, typeName)) {
        fprintf(writer->out, "struct %s", typeName);
    } else {
        fputs(typeName, writer->out);
    }
}

static void WriteTypedBaseTypeWithGenericContext(struct CWriter* writer, struct KekType* type, struct AstNode* params, struct AstNode* args) {
    while (type && type->kind == KEK_TYPE_ARRAY) {
        type = type->element;
    }
    WriteTypedNonArrayTypeWithGenericContext(writer, type, params, args);
}

static void WriteTypedBaseType(struct CWriter* writer, struct KekType* type) {
    WriteTypedBaseTypeWithGenericContext(writer, type, writer->genericParams, writer->genericArgs);
}

static void WriteTypedArraySuffix(struct CWriter* writer, struct KekType* type) {
    if (!type || type->kind != KEK_TYPE_ARRAY) {
        return;
    }
    WriteTypedArraySuffix(writer, type->element);
    fputc('[', writer->out);
    WriteTypedExpr(writer, type->arraySize);
    fputc(']', writer->out);
}

static void WriteTypedTypeAndNameWithGenericContext(struct CWriter* writer, struct KekType* type, struct AstNode* name, struct AstNode* params, struct AstNode* args) {
    char nameBuffer[128];
    struct KekType* base = type;
    while (base && base->kind == KEK_TYPE_ARRAY) {
        base = base->element;
    }

    WriteTypedNonArrayTypeWithGenericContext(writer, base, params, args);
    fputc(' ', writer->out);
    fputs(TypedNodeText(writer, name, nameBuffer, sizeof(nameBuffer)), writer->out);
    WriteTypedArraySuffix(writer, type);
}

static void WriteTypedTypeAndName(struct CWriter* writer, struct KekType* type, struct AstNode* name) {
    WriteTypedTypeAndNameWithGenericContext(writer, type, name, writer->genericParams, writer->genericArgs);
}

static void WriteTypedExprList(struct CWriter* writer, struct KekExpr* firstArg) {
    int first = 1;
    for (struct KekExpr* arg = firstArg; arg; arg = arg->next) {
        if (!first) {
            fputc(',', writer->out);
        }
        first = 0;
        WriteTypedExpr(writer, arg);
    }
}

static int ExprCalleeNameEquals(struct CWriter* writer, struct KekExpr* expr, const char* name) {
    return expr && expr->kind == KEK_EXPR_NAME && expr->token && TypedTokenTextEquals(writer, expr->token, name);
}

static int TypedCalleeName(struct CWriter* writer, struct KekExpr* expr, char* buffer, size_t bufferSize) {
    if (!expr || bufferSize == 0) {
        return 0;
    }
    buffer[0] = '\0';
    if (expr->kind == KEK_EXPR_NAME && expr->token) {
        char name[128];
        CopyTypedNodeText(writer, expr->token, name, sizeof(name));
        snprintf(buffer, bufferSize, "%s%s", writer->namespacePrefix, name);
        return 1;
    }
    if (expr->kind == KEK_EXPR_SCOPE) {
        char left[128] = {0};
        char right[128] = {0};
        if (expr->left && expr->left->kind == KEK_EXPR_NAME && expr->left->token) {
            CopyTypedNodeText(writer, expr->left->token, left, sizeof(left));
        } else if (expr->left) {
            (void)TypedCalleeName(writer, expr->left, left, sizeof(left));
        }
        if (expr->right && expr->right->token) {
            CopyTypedNodeText(writer, expr->right->token, right, sizeof(right));
        }
        if (left[0] != '\0') {
            snprintf(buffer, bufferSize, "%s_%s", left, right);
        } else {
            snprintf(buffer, bufferSize, "%s", right);
        }
        return buffer[0] != '\0';
    }
    return 0;
}

static int TypedNamedArgument(struct CWriter* writer, struct KekExpr* arg, char* name, size_t nameSize, struct KekExpr** value) {
    if (!arg || arg->kind != KEK_EXPR_ASSIGN || !arg->left || arg->left->kind != KEK_EXPR_NAME || !arg->left->token) {
        return 0;
    }
    CopyTypedNodeText(writer, arg->left->token, name, nameSize);
    *value = arg->right;
    return 1;
}

static int FindParameterIndex(struct CFunctionInfo* function, const char* name) {
    for (size_t i = 0; i < function->paramCount; i++) {
        if (strcmp(function->paramNames[i], name) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static void WriteTypedKnownCallArgs(struct CWriter* writer, struct KekExpr* call, struct CFunctionInfo* function, struct KekExpr* implicitThis, int implicitThisIsPointer) {
    struct KekExpr* ordered[16] = {0};
    int consumed[16] = {0};
    size_t positionalIndex = 0;

    if (implicitThis && function->paramCount > 0) {
        ordered[0] = implicitThis;
        consumed[0] = 1;
        positionalIndex = 1;
    }

    for (struct KekExpr* arg = call->firstArg; arg; arg = arg->next) {
        char name[64];
        struct KekExpr* value = NULL;
        if (TypedNamedArgument(writer, arg, name, sizeof(name), &value)) {
            int index = FindParameterIndex(function, name);
            if (index >= 0 && (size_t)index < function->paramCount) {
                ordered[index] = value;
                consumed[index] = 1;
                continue;
            }
        }

        while (positionalIndex < function->paramCount && consumed[positionalIndex]) {
            positionalIndex++;
        }
        if (positionalIndex < function->paramCount) {
            ordered[positionalIndex] = arg;
            consumed[positionalIndex] = 1;
            positionalIndex++;
        }
    }

    for (size_t i = 0; i < function->paramCount; i++) {
        if (i > 0) {
            fputc(',', writer->out);
        }
        if (ordered[i]) {
            if (i == 0 && implicitThis && implicitThisIsPointer) {
                WriteTypedExpr(writer, ordered[i]);
            } else if (i == 0 && implicitThis) {
                fputc('&', writer->out);
                WriteTypedExpr(writer, ordered[i]);
            } else {
                WriteTypedExpr(writer, ordered[i]);
            }
        } else if (function->defaults[i][0] != '\0') {
            fputs(function->defaults[i], writer->out);
        } else {
            fputc('0', writer->out);
        }
    }
}

static void WriteTypedCall(struct CWriter* writer, struct KekExpr* expr) {
    if (ExprCalleeNameEquals(writer, expr->callee, "sizeof") && expr->firstArg) {
        fputs("sizeof(", writer->out);
        if (expr->firstArg->kind == KEK_EXPR_NAME && expr->firstArg->token) {
            struct AstNode* substitution = FindGenericSubstitution(writer, writer->genericParams, writer->genericArgs, expr->firstArg->token);
            char typeName[128];
            if (substitution) {
                GenericArgMangleText(writer, writer->genericArgFile ? writer->genericArgFile : writer->file, substitution, typeName, sizeof(typeName));
            } else {
                CopyTypedNodeText(writer, expr->firstArg->token, typeName, sizeof(typeName));
            }
            if (IsKnownStruct(writer, typeName)) {
                fprintf(writer->out, "struct %s", typeName);
            } else {
                fputs(typeName, writer->out);
            }
        } else {
            WriteTypedExpr(writer, expr->firstArg);
        }
        fputc(')', writer->out);
        return;
    }

    if (ExprCalleeNameEquals(writer, expr->callee, "len") && expr->firstArg) {
        fputs("(sizeof(", writer->out);
        WriteTypedExpr(writer, expr->firstArg);
        fputs(")/sizeof((", writer->out);
        WriteTypedExpr(writer, expr->firstArg);
        fputs(")[0]))", writer->out);
        return;
    }

    if (expr->callee && expr->callee->kind == KEK_EXPR_FIELD && expr->callee->right && expr->callee->right->token) {
        char objectName[64] = {0};
        char methodName[64] = {0};
        char functionName[128];
        if (expr->callee->left && expr->callee->left->kind == KEK_EXPR_NAME && expr->callee->left->token) {
            CopyTypedNodeText(writer, expr->callee->left->token, objectName, sizeof(objectName));
        }
        CopyTypedNodeText(writer, expr->callee->right->token, methodName, sizeof(methodName));
        const char* objectType = FindLocalType(writer, objectName);
        if (objectType) {
            snprintf(functionName, sizeof(functionName), "%s_%s", objectType, methodName);
            fputs(functionName, writer->out);
            fputc('(', writer->out);
            struct CFunctionInfo* function = FindFunctionInfo(writer, functionName);
            if (function) {
                WriteTypedKnownCallArgs(writer, expr, function, expr->callee->left, FindLocalIsPointer(writer, objectName));
            } else {
                fputc('&', writer->out);
                WriteTypedExpr(writer, expr->callee->left);
                if (expr->firstArg) {
                    fputc(',', writer->out);
                    WriteTypedExprList(writer, expr->firstArg);
                }
            }
            fputc(')', writer->out);
            return;
        }
    }

    char calleeName[128];
    struct CFunctionInfo* function = NULL;
    if (expr->callee && expr->callee->genericArgs && (expr->callee->kind == KEK_EXPR_NAME || expr->callee->kind == KEK_EXPR_SCOPE)) {
        if (expr->callee->kind == KEK_EXPR_NAME) {
            char baseName[128];
            MangleGenericName(writer, expr->callee->token, expr->callee->genericArgs, baseName, sizeof(baseName));
            snprintf(calleeName, sizeof(calleeName), "%s%s", writer->namespacePrefix, baseName);
        } else if (TypedCalleeName(writer, expr->callee, calleeName, sizeof(calleeName))) {
            AppendGenericArgsToNameWithWriter(writer, writer->file, expr->callee->genericArgs, calleeName, sizeof(calleeName));
        }
        function = FindFunctionInfo(writer, calleeName);
        fputs(calleeName, writer->out);
        fputc('(', writer->out);
        if (function) {
            WriteTypedKnownCallArgs(writer, expr, function, NULL, 0);
        } else {
            WriteTypedExprList(writer, expr->firstArg);
        }
        fputc(')', writer->out);
        return;
    }
    if (TypedCalleeName(writer, expr->callee, calleeName, sizeof(calleeName))) {
        function = FindFunctionInfo(writer, calleeName);
        fputs(calleeName, writer->out);
    } else {
        WriteTypedExpr(writer, expr->callee);
    }
    fputc('(', writer->out);
    if (function) {
        WriteTypedKnownCallArgs(writer, expr, function, NULL, 0);
    } else {
        WriteTypedExprList(writer, expr->firstArg);
    }
    fputc(')', writer->out);
}

static void WriteTypedStructLiteral(struct CWriter* writer, struct KekExpr* expr) {
    int isArrayLiteral = expr && expr->type && expr->type->kind == KEK_TYPE_ARRAY;
    if (isArrayLiteral) {
        fputc('{', writer->out);
    } else {
        fputc('(', writer->out);
        WriteTypedBaseType(writer, expr ? expr->type : NULL);
        fputs("){", writer->out);
    }

    int first = 1;
    for (struct KekExpr* field = expr ? expr->firstArg : NULL; field; field = field->next) {
        if (!first) {
            fputc(',', writer->out);
        }
        first = 0;

        if (field->kind == KEK_EXPR_ASSIGN && field->left && field->left->kind == KEK_EXPR_NAME && field->left->token) {
            char fieldName[64];
            CopyTypedNodeText(writer, field->left->token, fieldName, sizeof(fieldName));
            fprintf(writer->out, ".%s=", fieldName);
            WriteTypedExpr(writer, field->right);
        } else {
            WriteTypedExpr(writer, field);
        }
    }

    fputc('}', writer->out);
}

static const char* TypedExprLocalType(struct CWriter* writer, struct KekExpr* expr) {
    if (!expr) {
        return NULL;
    }
    if (expr->kind == KEK_EXPR_NAME && expr->token) {
        char name[64];
        CopyTypedNodeText(writer, expr->token, name, sizeof(name));
        return FindLocalType(writer, name);
    }
    if (expr->kind == KEK_EXPR_GROUP) {
        return TypedExprLocalType(writer, expr->right);
    }
    if (expr->kind == KEK_EXPR_STRUCT_LITERAL && expr->type && expr->type->name) {
        static char typeName[128];
        CopyTypedNodeText(writer, expr->type->name, typeName, sizeof(typeName));
        return typeName;
    }
    return NULL;
}

static const char* ExprOperatorMangle(struct KekExpr* expr) {
    if (!expr || !expr->token || expr->token->token.type != TOKEN_OPERATOR) {
        return NULL;
    }
    return OperatorMangleName(expr->token->token.value.operator);
}

static int TryWriteTypedUnaryOperatorCall(struct CWriter* writer, struct KekExpr* expr) {
    const char* operandType = TypedExprLocalType(writer, expr ? expr->right : NULL);
    const char* opName = ExprOperatorMangle(expr);
    if (expr && expr->token && IsOperatorToken(&expr->token->token, OPERATOR_BITWISE_AND)) {
        return 0;
    }
    if (!operandType || !opName || !IsKnownStruct(writer, operandType)) {
        return 0;
    }

    char functionName[128];
    snprintf(functionName, sizeof(functionName), "%s_%s_0", operandType, opName);
    if (!FindFunctionInfo(writer, functionName)) {
        return 0;
    }

    fputs(functionName, writer->out);
    fputc('(', writer->out);
    fputc('&', writer->out);
    WriteTypedExpr(writer, expr->right);
    fputc(')', writer->out);
    return 1;
}

static int TryWriteTypedBinaryOperatorCall(struct CWriter* writer, struct KekExpr* expr) {
    const char* leftType = TypedExprLocalType(writer, expr ? expr->left : NULL);
    const char* opName = ExprOperatorMangle(expr);
    if (!leftType || !opName || !IsKnownStruct(writer, leftType)) {
        return 0;
    }

    char functionName[128];
    snprintf(functionName, sizeof(functionName), "%s_%s_1", leftType, opName);
    if (!FindFunctionInfo(writer, functionName)) {
        return 0;
    }

    fputs(functionName, writer->out);
    fputc('(', writer->out);
    fputc('&', writer->out);
    WriteTypedExpr(writer, expr->left);
    fputc(',', writer->out);
    WriteTypedExpr(writer, expr->right);
    fputc(')', writer->out);
    return 1;
}

static void WriteTypedExpr(struct CWriter* writer, struct KekExpr* expr) {
    if (!expr) {
        return;
    }

    char buffer[128];
    switch (expr->kind) {
        case KEK_EXPR_NAME:
        case KEK_EXPR_NUMBER:
        case KEK_EXPR_STRING:
            if (expr->kind == KEK_EXPR_NAME) {
                struct AstNode* substitution = FindGenericSubstitution(writer, writer->genericParams, writer->genericArgs, expr->token);
                if (substitution) {
                    GenericArgText(writer, substitution, buffer, sizeof(buffer));
                    fputs(buffer, writer->out);
                    break;
                }
            }
            fputs(TypedNodeText(writer, expr->token, buffer, sizeof(buffer)), writer->out);
            break;
        case KEK_EXPR_BOOL:
            fputs(IsKeywordToken(&expr->token->token, KEYWORD_TRUE) ? "1" : "0", writer->out);
            break;
        case KEK_EXPR_CALL:
            WriteTypedCall(writer, expr);
            break;
        case KEK_EXPR_FIELD:
            WriteTypedExpr(writer, expr->left);
            if (expr->left && expr->left->kind == KEK_EXPR_NAME && expr->left->token) {
                char localName[64];
                CopyTypedNodeText(writer, expr->left->token, localName, sizeof(localName));
                fputs((writer->thisIsPointer && TokenTextEquals(&expr->left->token->token, writer->file, "this"))
                    || FindLocalIsPointer(writer, localName) ? "->" : ".", writer->out);
            } else {
                fputc('.', writer->out);
            }
            WriteTypedExpr(writer, expr->right);
            break;
        case KEK_EXPR_SCOPE:
            if (expr->left) {
                WriteTypedExpr(writer, expr->left);
                fputc('_', writer->out);
            }
            WriteTypedExpr(writer, expr->right);
            break;
        case KEK_EXPR_INDEX:
            WriteTypedExpr(writer, expr->left);
            fputc('[', writer->out);
            WriteTypedExpr(writer, expr->right);
            fputc(']', writer->out);
            break;
        case KEK_EXPR_GROUP:
            fputc('(', writer->out);
            WriteTypedExpr(writer, expr->right);
            fputc(')', writer->out);
            break;
        case KEK_EXPR_STRUCT_LITERAL:
            WriteTypedStructLiteral(writer, expr);
            break;
        case KEK_EXPR_UNARY:
            if (TryWriteTypedUnaryOperatorCall(writer, expr)) {
                break;
            }
            fputs(TypedNodeText(writer, expr->token, buffer, sizeof(buffer)), writer->out);
            WriteTypedExpr(writer, expr->right);
            break;
        case KEK_EXPR_BINARY:
        case KEK_EXPR_ASSIGN:
            if (TryWriteTypedBinaryOperatorCall(writer, expr)) {
                break;
            }
            WriteTypedExpr(writer, expr->left);
            fputs(TypedNodeText(writer, expr->token, buffer, sizeof(buffer)), writer->out);
            WriteTypedExpr(writer, expr->right);
            break;
        case KEK_EXPR_CAST:
            fputs("((", writer->out);
            WriteTypedBaseType(writer, expr->type);
            fputs(")(", writer->out);
            WriteTypedExpr(writer, expr->right);
            fputs("))", writer->out);
            break;
        case KEK_EXPR_UNKNOWN:
        case KEK_EXPR_COUNT:
            if (expr->source) {
                WriteSourceSlice(writer->out, writer->file, expr->source->location);
            }
            break;
    }
}

static void WriteTypedStatement(struct CWriter* writer, struct KekStmt* stmt);
static void WriteTypedBlock(struct CWriter* writer, struct KekStmt* block);
static struct KekStmt* FirstTypedBlockChild(struct KekStmt* stmt);

static void WriteDeferredStatement(struct CWriter* writer, struct KekStmt* stmt) {
    if (!stmt || stmt->kind != KEK_STMT_DEFER) {
        return;
    }

    struct KekStmt* block = FirstTypedBlockChild(stmt);
    if (block) {
        WriteTypedBlock(writer, block);
        return;
    }

    WriteTypedExpr(writer, stmt->expr);
    fputc(';', writer->out);
}

static void WriteDeferredRange(struct CWriter* writer, size_t start, size_t end) {
    while (end > start) {
        end--;
        WriteIndent(writer->out, writer->indent);
        WriteDeferredStatement(writer, writer->deferred[end]);
        fputc('\n', writer->out);
    }
}

static void WriteAllActiveDefers(struct CWriter* writer) {
    WriteDeferredRange(writer, 0, writer->deferCount);
}

static void WriteTypedBlock(struct CWriter* writer, struct KekStmt* block) {
    fputs("{\n", writer->out);
    writer->indent++;
    size_t blockDeferStart = writer->deferCount;
    for (struct KekStmt* child = block ? block->firstChild : NULL; child; child = child->next) {
        if (child->kind == KEK_STMT_DEFER) {
            if (writer->deferCount < sizeof(writer->deferred) / sizeof(writer->deferred[0])) {
                writer->deferred[writer->deferCount++] = child;
            }
            continue;
        }
        if (child->kind == KEK_STMT_RETURN) {
            WriteAllActiveDefers(writer);
        } else if (child->kind == KEK_STMT_BREAK || child->kind == KEK_STMT_CONTINUE) {
            WriteDeferredRange(writer, blockDeferStart, writer->deferCount);
        }
        WriteIndent(writer->out, writer->indent);
        WriteTypedStatement(writer, child);
        fputc('\n', writer->out);
    }
    WriteDeferredRange(writer, blockDeferStart, writer->deferCount);
    writer->deferCount = blockDeferStart;
    if (writer->indent > 0) {
        writer->indent--;
    }
    WriteIndent(writer->out, writer->indent);
    fputc('}', writer->out);
}

static struct KekStmt* FirstTypedBlockChild(struct KekStmt* stmt) {
    for (struct KekStmt* child = stmt ? stmt->firstChild : NULL; child; child = child->next) {
        if (child->kind == KEK_STMT_BLOCK) {
            return child;
        }
    }
    return NULL;
}

static void WriteTypedDeclStatement(struct CWriter* writer, struct KekStmt* stmt, int withSemicolon) {
    char nameBuffer[128];
    WriteTypedTypeAndName(writer, stmt->declType, stmt->declName);
    CopyTypedNodeText(writer, stmt->declName, nameBuffer, sizeof(nameBuffer));

    struct KekType* base = stmt->declType;
    while (base && base->kind == KEK_TYPE_ARRAY) {
        base = base->element;
    }
    if (base && base->name) {
        char typeBuffer[128];
        if (!GenericStructInstanceName(writer, base, typeBuffer, sizeof(typeBuffer))) {
            CopyTypedNodeText(writer, base->name, typeBuffer, sizeof(typeBuffer));
        }
        AddLocalTypeEx(writer, nameBuffer, typeBuffer, stmt->declType && stmt->declType->kind == KEK_TYPE_POINTER);
        if (!stmt->expr && IsKnownStruct(writer, typeBuffer)) {
            struct CStructInfo* info = FindStructInfo(writer, typeBuffer);
            if (info && info->fieldCount > 0) {
                fputs(" = {", writer->out);
                for (size_t i = 0; i < info->fieldCount; i++) {
                    if (i > 0) {
                        fputc(',', writer->out);
                    }
                    fprintf(writer->out, ".%s=%s", info->fieldNames[i], info->defaults[i]);
                }
                fputc('}', writer->out);
            }
        }
    }

    if (stmt->expr) {
        fputc('=', writer->out);
        WriteTypedExpr(writer, stmt->expr);
    }
    if (withSemicolon) {
        fputc(';', writer->out);
    }
}

static void WriteTypedStatement(struct CWriter* writer, struct KekStmt* stmt) {
    if (!stmt) {
        return;
    }
    switch (stmt->kind) {
        case KEK_STMT_BLOCK:
            WriteTypedBlock(writer, stmt);
            break;
        case KEK_STMT_DECL:
            WriteTypedDeclStatement(writer, stmt, 1);
            break;
        case KEK_STMT_EXPR:
            WriteTypedExpr(writer, stmt->expr);
            fputc(';', writer->out);
            break;
        case KEK_STMT_IF:
            fputs("if (", writer->out);
            WriteTypedExpr(writer, stmt->condition);
            fputs(") ", writer->out);
            WriteTypedBlock(writer, FirstTypedBlockChild(stmt));
            break;
        case KEK_STMT_ELSE:
            fputs("else ", writer->out);
            WriteTypedBlock(writer, FirstTypedBlockChild(stmt));
            break;
        case KEK_STMT_WHILE:
            fputs("while (", writer->out);
            WriteTypedExpr(writer, stmt->condition);
            fputs(") ", writer->out);
            WriteTypedBlock(writer, FirstTypedBlockChild(stmt));
            break;
        case KEK_STMT_DO_WHILE:
            fputs("do ", writer->out);
            WriteTypedBlock(writer, FirstTypedBlockChild(stmt));
            fputs(" while (", writer->out);
            WriteTypedExpr(writer, stmt->condition);
            fputs(");", writer->out);
            break;
        case KEK_STMT_FOR:
            fputs("for (", writer->out);
            if (stmt->initStmt) {
                WriteTypedDeclStatement(writer, stmt->initStmt, 0);
            } else {
                WriteTypedExpr(writer, stmt->expr);
            }
            fputc(';', writer->out);
            WriteTypedExpr(writer, stmt->condition);
            fputc(';', writer->out);
            WriteTypedExpr(writer, stmt->step);
            fputs(") ", writer->out);
            WriteTypedBlock(writer, FirstTypedBlockChild(stmt));
            break;
        case KEK_STMT_SWITCH:
            fputs("switch (", writer->out);
            WriteTypedExpr(writer, stmt->condition);
            fputs(") ", writer->out);
            WriteTypedBlock(writer, FirstTypedBlockChild(stmt));
            break;
        case KEK_STMT_CASE:
            fputs("case ", writer->out);
            WriteTypedExpr(writer, stmt->condition);
            fputs(": ", writer->out);
            if (FirstTypedBlockChild(stmt)) {
                WriteTypedBlock(writer, FirstTypedBlockChild(stmt));
            }
            break;
        case KEK_STMT_DEFAULT:
            fputs("default:", writer->out);
            if (stmt->expr) {
                fputc(' ', writer->out);
                WriteTypedExpr(writer, stmt->expr);
                fputc(';', writer->out);
            }
            break;
        case KEK_STMT_DEFER:
            WriteDeferredStatement(writer, stmt);
            break;
        case KEK_STMT_RETURN:
            fputs("return", writer->out);
            if (stmt->expr) {
                fputc('(', writer->out);
                WriteTypedExpr(writer, stmt->expr);
                fputc(')', writer->out);
            }
            fputc(';', writer->out);
            break;
        case KEK_STMT_BREAK:
            fputs("break;", writer->out);
            break;
        case KEK_STMT_CONTINUE:
            fputs("continue;", writer->out);
            break;
        case KEK_STMT_UNKNOWN:
        case KEK_STMT_COUNT:
            if (stmt->source) {
                WriteSourceSlice(writer->out, writer->file, stmt->source->location);
            }
            break;
    }
}

static void RegisterTypedStruct(struct CWriter* writer, struct KekDecl* decl) {
    if (!decl || !decl->name) {
        return;
    }
    char structName[64];
    CopyTypedNodeText(writer, decl->name, structName, sizeof(structName));
    struct CStructInfo* info = AddStructInfo(writer, structName);
    if (!info) {
        return;
    }
    for (struct KekField* field = decl->firstField; field && info->fieldCount < 64; field = field->next) {
        CopyTypedNodeText(writer, field->name, info->fieldNames[info->fieldCount], sizeof(info->fieldNames[info->fieldCount]));
        CopyTypedFieldDefault(writer, field, info->defaults[info->fieldCount], sizeof(info->defaults[info->fieldCount]));
        info->fieldCount++;
    }
}

static void RegisterTypedFunction(struct CWriter* writer, const char* functionName, struct KekDecl* decl, int includeThis) {
    struct CFunctionInfo* function = AddFunctionInfo(writer, functionName);
    if (!function) {
        return;
    }
    if (includeThis && function->paramCount < 16) {
        snprintf(function->paramNames[function->paramCount++], sizeof(function->paramNames[0]), "this");
    }
    for (struct KekParam* param = decl->firstParam; param && function->paramCount < 16; param = param->next) {
        CopyTypedNodeText(writer, param->name, function->paramNames[function->paramCount], sizeof(function->paramNames[function->paramCount]));
        if (param->defaultValue) {
            CopyExprSource(writer, param->defaultValue, function->defaults[function->paramCount], sizeof(function->defaults[function->paramCount]));
        }
        function->paramCount++;
    }
}

static void RegisterTypedFunctionWithName(struct CWriter* writer, const char* functionName, struct KekDecl* decl, int includeThis) {
    RegisterTypedFunction(writer, functionName, decl, includeThis);
}

static int TypedDeclIsMethod(struct KekDecl* decl) {
    struct AstNode* receiver = TypedDeclReceiverName(decl);
    struct AstNode* afterReceiver = receiver ? NextSibling(receiver) : NULL;
    if (IsGenericNode(afterReceiver)) {
        afterReceiver = NextSibling(afterReceiver);
    }
    return decl
        && decl->kind == KEK_DECL_FUNCTION
        && decl->type
        && receiver
        && IsTokenNode(receiver)
        && afterReceiver
        && IsTokenNode(afterReceiver)
        && IsOperatorToken(&afterReceiver->token, OPERATOR_SCOPE);
}

static struct AstNode* TypedDeclReceiverName(struct KekDecl* decl) {
    if (!decl || !decl->type) {
        return NULL;
    }
    struct AstNode* colon = NextSibling(decl->type);
    if (IsGenericNode(colon)) {
        colon = NextSibling(colon);
    }
    if (!colon) {
        return NULL;
    }
    return NextSibling(colon);
}

static void TypedFunctionName(struct CWriter* writer, struct KekDecl* decl, char* buffer, size_t bufferSize) {
    char name[64];
    TypedDeclBaseName(writer, decl, name, sizeof(name));
    if (TypedDeclIsMethod(decl)) {
        char receiver[64];
        CopyTypedNodeText(writer, TypedDeclReceiverName(decl), receiver, sizeof(receiver));
        snprintf(buffer, bufferSize, "%s_%s", receiver, name);
    } else {
        snprintf(buffer, bufferSize, "%s%s", writer->namespacePrefix, name);
    }
}

static void RegisterTypedDeclForCodegen(struct CWriter* writer, struct KekDecl* decl) {
    if (decl->genericParams) {
        return;
    }
    if (decl->kind == KEK_DECL_STRUCT) {
        RegisterTypedStruct(writer, decl);
    } else if (decl->kind == KEK_DECL_EXTERN_C) {
        RegisterExternCStructs(writer, decl);
    } else if (decl->kind == KEK_DECL_FUNCTION) {
        char functionName[128];
        TypedFunctionName(writer, decl, functionName, sizeof(functionName));
        RegisterTypedFunction(writer, functionName, decl, TypedDeclIsMethod(decl));
    }
}

static int AttributeListContains(struct CWriter* writer, struct AstNode* attributes, const char* name) {
    if (!attributes || attributes->type != AST_INDEX) {
        return 0;
    }
    for (struct AstNode* attribute = attributes->firstChild; attribute; attribute = attribute->nextSibling) {
        struct AstNode* token = attribute->firstChild;
        if (token && IsTokenNode(token) && TokenTextEquals(&token->token, writer->file, name)) {
            return 1;
        }
    }
    return 0;
}

static int TypedDeclHasAttribute(struct CWriter* writer, struct KekDecl* decl, const char* name) {
    struct AstNode* first = decl && decl->source ? decl->source->firstChild : NULL;
    return first && first->type == AST_INDEX && AttributeListContains(writer, first, name);
}

static void WriteTypedParams(struct CWriter* writer, struct KekDecl* decl) {
    if (!decl->firstParam) {
        fputs("void", writer->out);
        return;
    }
    int first = 1;
    for (struct KekParam* param = decl->firstParam; param; param = param->next) {
        if (!first) {
            fputc(',', writer->out);
        }
        first = 0;
        WriteTypedTypeAndName(writer, param->type, param->name);
    }
}

static void WriteTypedFunctionSignature(struct CWriter* writer, struct KekDecl* decl, const char* functionName) {
    char returnName[128];
    if (TypedDeclHasAttribute(writer, decl, "static")) {
        fputs("static ", writer->out);
    }
    if (TypedDeclHasAttribute(writer, decl, "inline")) {
        fputs("inline ", writer->out);
    }
    if (!TypedDeclIsMethod(decl)
        && TypedNodeText(writer, decl->parsedType ? decl->parsedType->name : NULL, returnName, sizeof(returnName))
        && strcmp(returnName, "i64") == 0
        && strcmp(functionName, "main") == 0) {
        fputs("int", writer->out);
    } else {
        WriteTypedBaseType(writer, decl->parsedType);
    }
    fprintf(writer->out, " %s(", functionName);
    if (TypedDeclIsMethod(decl)) {
        char receiver[64];
        CopyTypedNodeText(writer, TypedDeclReceiverName(decl), receiver, sizeof(receiver));
        fprintf(writer->out, "struct %s* this", receiver);
        if (decl->firstParam) {
            fputc(',', writer->out);
            WriteTypedParams(writer, decl);
        }
    } else {
        WriteTypedParams(writer, decl);
    }
    fputc(')', writer->out);
}

static void WriteTypedGenericStructInstance(struct CWriter* writer, struct CGenericInstance* instance) {
    if (!instance || !instance->decl) {
        return;
    }
    struct SourceFile* previousFile = writer->file;
    struct AstNode* previousParams = writer->genericParams;
    struct AstNode* previousArgs = writer->genericArgs;
    struct SourceFile* previousArgFile = writer->genericArgFile;
    writer->file = instance->declFile ? instance->declFile : writer->file;
    writer->genericParams = instance->decl->genericParams;
    writer->genericArgs = instance->args;
    writer->genericArgFile = instance->argsFile ? instance->argsFile : writer->file;

    fprintf(writer->out, "struct %s {\n", instance->name);
    writer->indent++;
    for (struct KekField* field = instance->decl->firstField; field; field = field->next) {
        WriteIndent(writer->out, writer->indent);
        WriteTypedTypeAndName(writer, field->type, field->name);
        fputs(";\n", writer->out);
    }
    writer->indent--;
    fputs("};", writer->out);

    writer->file = previousFile;
    writer->genericParams = previousParams;
    writer->genericArgs = previousArgs;
    writer->genericArgFile = previousArgFile;
}

static void WriteTypedGenericFunctionSignature(struct CWriter* writer, struct CGenericInstance* instance) {
    struct KekDecl* decl = instance->decl;
    struct SourceFile* previousFile = writer->file;
    struct AstNode* previousParams = writer->genericParams;
    struct AstNode* previousArgs = writer->genericArgs;
    struct SourceFile* previousArgFile = writer->genericArgFile;

    writer->file = instance->declFile ? instance->declFile : previousFile;
    writer->genericParams = decl->genericParams;
    writer->genericArgs = instance->args;
    writer->genericArgFile = instance->argsFile;

    WriteTypedBaseType(writer, decl->parsedType);
    fprintf(writer->out, " %s(", instance->name);
    if (TypedDeclIsMethod(decl)) {
        char receiver[128];
        MangleGenericNameWithFiles(writer, instance->declFile, TypedDeclReceiverName(decl), instance->argsFile, instance->args, receiver, sizeof(receiver));
        fprintf(writer->out, "struct %s* this", receiver);
        if (decl->firstParam) {
            fputc(',', writer->out);
            WriteTypedParams(writer, decl);
        }
    } else {
        WriteTypedParams(writer, decl);
    }
    fputc(')', writer->out);

    writer->file = previousFile;
    writer->genericParams = previousParams;
    writer->genericArgs = previousArgs;
    writer->genericArgFile = previousArgFile;
}

static void WriteTypedGenericFunctionInstance(struct CWriter* writer, struct CGenericInstance* instance) {
    if (!instance || !instance->decl) {
        return;
    }
    struct KekDecl* decl = instance->decl;
    struct SourceFile* previousFile = writer->file;
    struct AstNode* previousParams = writer->genericParams;
    struct AstNode* previousArgs = writer->genericArgs;
    struct SourceFile* previousArgFile = writer->genericArgFile;
    writer->file = instance->declFile ? instance->declFile : writer->file;
    if (writer->file) {
        PackagePrefixFromPath(writer->file->path, writer->namespacePrefix, sizeof(writer->namespacePrefix));
    }
    writer->genericParams = decl->genericParams;
    writer->genericArgs = instance->args;
    writer->genericArgFile = instance->argsFile ? instance->argsFile : writer->file;

    WriteTypedGenericFunctionSignature(writer, instance);
    fputc(' ', writer->out);
    int previousThisIsPointer = writer->thisIsPointer;
    size_t previousLocalCount = writer->localCount;
    if (TypedDeclIsMethod(decl)) {
        char receiver[128];
        MangleGenericNameWithFiles(writer, instance->declFile, TypedDeclReceiverName(decl), instance->argsFile, instance->args, receiver, sizeof(receiver));
        AddLocalTypeEx(writer, "this", receiver, 1);
    }
    for (struct KekParam* param = decl->firstParam; param; param = param->next) {
        char paramName[64];
        char typeName[64];
        struct KekType* base = param->type;
        while (base && (base->kind == KEK_TYPE_ARRAY || base->kind == KEK_TYPE_POINTER)) {
            base = base->element;
        }
        CopyTypedNodeText(writer, param->name, paramName, sizeof(paramName));
        CopyTypedNodeText(writer, base ? base->name : NULL, typeName, sizeof(typeName));
        AddLocalTypeEx(writer, paramName, typeName, param->type && param->type->kind == KEK_TYPE_POINTER);
    }
    writer->thisIsPointer = TypedDeclIsMethod(decl);
    WriteTypedBlock(writer, decl->firstStmt);
    writer->thisIsPointer = previousThisIsPointer;
    writer->localCount = previousLocalCount;

    writer->file = previousFile;
    writer->genericParams = previousParams;
    writer->genericArgs = previousArgs;
    writer->genericArgFile = previousArgFile;
}

static void WriteTypedDecl(struct CWriter* writer, struct KekDecl* decl) {
    char name[128];
    if (decl->genericParams) {
        return;
    }
    switch (decl->kind) {
        case KEK_DECL_IMPORT:
        case KEK_DECL_USING:
            break;
        case KEK_DECL_EXTERN_C:
            if (decl->body && decl->body->location.length >= 2) {
                struct SourceLocation body = decl->body->location;
                body.offset++;
                body.length -= 2;
                WriteSourceSlice(writer->out, writer->file, body);
            }
            break;
        case KEK_DECL_ALIAS:
            fputs("typedef ", writer->out);
            WriteTypedBaseType(writer, decl->parsedType);
            fputc(' ', writer->out);
            fputs(TypedNodeText(writer, decl->name, name, sizeof(name)), writer->out);
            fputc(';', writer->out);
            break;
        case KEK_DECL_STRUCT:
            fputs("struct", writer->out);
            if (TypedDeclHasAttribute(writer, decl, "packed")) {
                fputs(" __attribute__((packed))", writer->out);
            }
            fputc(' ', writer->out);
            fputs(TypedNodeText(writer, decl->name, name, sizeof(name)), writer->out);
            fputs(" {\n", writer->out);
            writer->indent++;
            for (struct KekField* field = decl->firstField; field; field = field->next) {
                WriteIndent(writer->out, writer->indent);
                WriteTypedTypeAndName(writer, field->type, field->name);
                fputs(";\n", writer->out);
            }
            writer->indent--;
            fputs("};", writer->out);
            break;
        case KEK_DECL_ENUM:
            fputs("typedef enum ", writer->out);
            fputs(TypedNodeText(writer, decl->name, name, sizeof(name)), writer->out);
            fputs(" {\n", writer->out);
            writer->indent++;
            for (struct KekVariant* variant = decl->firstVariant; variant; variant = variant->next) {
                char variantName[128];
                WriteIndent(writer->out, writer->indent);
                fprintf(writer->out, "%s_%s", name, TypedNodeText(writer, variant->name, variantName, sizeof(variantName)));
                if (variant->value) {
                    fputc('=', writer->out);
                    WriteTypedExpr(writer, variant->value);
                }
                fputs(",\n", writer->out);
            }
            writer->indent--;
            fprintf(writer->out, "} %s;", name);
            break;
        case KEK_DECL_UNION:
            fputs("typedef union ", writer->out);
            fputs(TypedNodeText(writer, decl->name, name, sizeof(name)), writer->out);
            fputs(" {\n", writer->out);
            writer->indent++;
            for (struct KekField* field = decl->firstField; field; field = field->next) {
                WriteIndent(writer->out, writer->indent);
                WriteTypedTypeAndName(writer, field->type, field->name);
                fputs(";\n", writer->out);
            }
            writer->indent--;
            fprintf(writer->out, "} %s;", name);
            break;
        case KEK_DECL_FUNCTION: {
            char functionName[128];
            TypedFunctionName(writer, decl, functionName, sizeof(functionName));
            WriteTypedFunctionSignature(writer, decl, functionName);
            fputc(' ', writer->out);
            int previousThisIsPointer = writer->thisIsPointer;
            size_t previousLocalCount = writer->localCount;
            if (TypedDeclIsMethod(decl)) {
                char receiver[64];
                CopyTypedNodeText(writer, TypedDeclReceiverName(decl), receiver, sizeof(receiver));
                AddLocalTypeEx(writer, "this", receiver, 1);
            }
            for (struct KekParam* param = decl->firstParam; param; param = param->next) {
                char paramName[64];
                char typeName[64];
                struct KekType* base = param->type;
                while (base && (base->kind == KEK_TYPE_ARRAY || base->kind == KEK_TYPE_POINTER)) {
                    base = base->element;
                }
                CopyTypedNodeText(writer, param->name, paramName, sizeof(paramName));
                CopyTypedNodeText(writer, base ? base->name : NULL, typeName, sizeof(typeName));
                AddLocalTypeEx(writer, paramName, typeName, param->type && param->type->kind == KEK_TYPE_POINTER);
            }
            writer->thisIsPointer = TypedDeclIsMethod(decl);
            WriteTypedBlock(writer, decl->firstStmt);
            writer->thisIsPointer = previousThisIsPointer;
            writer->localCount = previousLocalCount;
            break;
        }
        case KEK_DECL_VARIABLE:
            break;
        case KEK_DECL_UNKNOWN:
        case KEK_DECL_COUNT:
            break;
    }
}

static void CollectGenericTypeUse(struct CWriter* writer, struct KekModule* modules, size_t count, struct KekType* type) {
    for (struct KekType* current = type; current; current = current->element) {
        if (current->name && current->genericArgs) {
            char baseName[128];
            CopyTypedNodeText(writer, current->name, baseName, sizeof(baseName));
            struct SourceFile* declFile = NULL;
            struct KekDecl* structDecl = FindGenericDecl(modules, count, KEK_DECL_STRUCT, baseName, &declFile);
            if (structDecl) {
                AddGenericStructInstance(writer, structDecl, declFile, current->genericArgs, writer->file);
            }
        }
        if (current->kind != KEK_TYPE_ARRAY && current->kind != KEK_TYPE_POINTER) {
            break;
        }
    }
}

static void CollectGenericExprUses(struct CWriter* writer, struct KekModule* modules, size_t count, struct KekExpr* expr) {
    if (!expr) {
        return;
    }
    if (expr->kind == KEK_EXPR_CALL
        && expr->callee
        && expr->callee->genericArgs
        && (expr->callee->kind == KEK_EXPR_NAME || expr->callee->kind == KEK_EXPR_SCOPE)) {
        char baseName[128];
        if (expr->callee->kind == KEK_EXPR_NAME) {
            CopyTypedNodeText(writer, expr->callee->token, baseName, sizeof(baseName));
        } else if (expr->callee->right && expr->callee->right->token) {
            CopyTypedNodeText(writer, expr->callee->right->token, baseName, sizeof(baseName));
        } else {
            baseName[0] = '\0';
        }
        struct SourceFile* declFile = NULL;
        struct KekDecl* functionDecl = FindGenericDecl(modules, count, KEK_DECL_FUNCTION, baseName, &declFile);
        if (functionDecl) {
            AddGenericFunctionInstance(writer, functionDecl, declFile, expr->callee->genericArgs, writer->file);
        }
    }
    CollectGenericTypeUse(writer, modules, count, expr->type);
    CollectGenericExprUses(writer, modules, count, expr->left);
    CollectGenericExprUses(writer, modules, count, expr->right);
    CollectGenericExprUses(writer, modules, count, expr->callee);
    for (struct KekExpr* arg = expr->firstArg; arg; arg = arg->next) {
        CollectGenericExprUses(writer, modules, count, arg);
    }
}

static void CollectGenericStmtUses(struct CWriter* writer, struct KekModule* modules, size_t count, struct KekStmt* stmt) {
    for (; stmt; stmt = stmt->next) {
        CollectGenericTypeUse(writer, modules, count, stmt->declType);
        CollectGenericExprUses(writer, modules, count, stmt->expr);
        CollectGenericExprUses(writer, modules, count, stmt->condition);
        CollectGenericExprUses(writer, modules, count, stmt->step);
        CollectGenericStmtUses(writer, modules, count, stmt->initStmt);
        CollectGenericStmtUses(writer, modules, count, stmt->firstChild);
    }
}

static void CollectGenericDeclUses(struct CWriter* writer, struct KekModule* modules, size_t count, struct KekDecl* decl) {
    if (decl->genericParams) {
        return;
    }
    CollectGenericTypeUse(writer, modules, count, decl->parsedType);
    for (struct KekParam* param = decl->firstParam; param; param = param->next) {
        CollectGenericTypeUse(writer, modules, count, param->type);
        CollectGenericExprUses(writer, modules, count, param->defaultValue);
    }
    for (struct KekField* field = decl->firstField; field; field = field->next) {
        CollectGenericTypeUse(writer, modules, count, field->type);
        CollectGenericExprUses(writer, modules, count, field->defaultValue);
    }
    CollectGenericStmtUses(writer, modules, count, decl->firstStmt);
}

static int IsTypedTypeDecl(struct KekDecl* decl) {
    return decl
        && (decl->kind == KEK_DECL_ALIAS
            || decl->kind == KEK_DECL_ENUM
            || decl->kind == KEK_DECL_STRUCT
            || decl->kind == KEK_DECL_UNION
            || decl->kind == KEK_DECL_EXTERN_C);
}

int WriteTypedCFileForModules(const char* path, struct KekModule* modules, size_t count) {
    FILE* out = fopen(path, "w");
    if (!out) {
        return -1;
    }

    struct CWriter writer = {0};
    writer.out = out;

    WritePrelude(out);
    for (size_t i = 0; i < count; i++) {
        writer.file = modules[i].file;
        if (modules[i].file && i + 1 < count) {
            PackagePrefixFromPath(modules[i].file->path, writer.namespacePrefix, sizeof(writer.namespacePrefix));
        } else {
            writer.namespacePrefix[0] = '\0';
        }

        for (struct KekDecl* decl = modules[i].firstDecl; decl; decl = decl->next) {
            RegisterTypedDeclForCodegen(&writer, decl);
        }
        for (struct KekDecl* decl = modules[i].firstDecl; decl; decl = decl->next) {
            CollectGenericDeclUses(&writer, modules, count, decl);
        }
    }

    AddGenericMethodInstancesForStructs(&writer, modules, count);

    for (size_t i = 0; i < count; i++) {
        writer.file = modules[i].file;
        if (modules[i].file && i + 1 < count) {
            PackagePrefixFromPath(modules[i].file->path, writer.namespacePrefix, sizeof(writer.namespacePrefix));
        } else {
            writer.namespacePrefix[0] = '\0';
        }

        for (struct KekDecl* decl = modules[i].firstDecl; decl; decl = decl->next) {
            if (IsTypedTypeDecl(decl)) {
                WriteTypedDecl(&writer, decl);
                fputc('\n', writer.out);
            }
        }
    }

    for (size_t i = 0; i < writer.genericFunctionCount; i++) {
        writer.file = writer.genericFunctions[i].declFile ? writer.genericFunctions[i].declFile : writer.file;
        RegisterTypedFunctionWithName(&writer, writer.genericFunctions[i].name, writer.genericFunctions[i].decl, TypedDeclIsMethod(writer.genericFunctions[i].decl));
    }

    for (size_t i = 0; i < writer.genericStructCount; i++) {
        WriteTypedGenericStructInstance(&writer, &writer.genericStructs[i]);
        fputc('\n', writer.out);
    }

    for (size_t i = 0; i < count; i++) {
        writer.file = modules[i].file;
        if (modules[i].file && i + 1 < count) {
            PackagePrefixFromPath(modules[i].file->path, writer.namespacePrefix, sizeof(writer.namespacePrefix));
        } else {
            writer.namespacePrefix[0] = '\0';
        }

        for (struct KekDecl* decl = modules[i].firstDecl; decl; decl = decl->next) {
            if (decl->kind == KEK_DECL_FUNCTION && !decl->genericParams) {
                char functionName[128];
                TypedFunctionName(&writer, decl, functionName, sizeof(functionName));
                WriteTypedFunctionSignature(&writer, decl, functionName);
                fputs(";\n", writer.out);
            }
        }
    }

    for (size_t i = 0; i < writer.genericFunctionCount; i++) {
        WriteTypedGenericFunctionSignature(&writer, &writer.genericFunctions[i]);
        fputs(";\n", writer.out);
    }

    for (size_t i = 0; i < writer.genericFunctionCount; i++) {
        WriteTypedGenericFunctionInstance(&writer, &writer.genericFunctions[i]);
        fputc('\n', writer.out);
    }

    for (size_t i = 0; i < count; i++) {
        writer.file = modules[i].file;
        if (modules[i].file && i + 1 < count) {
            PackagePrefixFromPath(modules[i].file->path, writer.namespacePrefix, sizeof(writer.namespacePrefix));
        } else {
            writer.namespacePrefix[0] = '\0';
        }

        for (struct KekDecl* decl = modules[i].firstDecl; decl; decl = decl->next) {
            if (decl->kind == KEK_DECL_FUNCTION) {
                WriteTypedDecl(&writer, decl);
                fputc('\n', writer.out);
            }
        }
    }

    fclose(out);
    return 0;
}
