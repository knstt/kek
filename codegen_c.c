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
    int atLineStart;
    int needSpace;
    struct Token* previous;
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

static int IsWordToken(struct Token* token) {
    return token->type == TOKEN_IDENTIFIER || token->type == TOKEN_NUMBER || token->type == TOKEN_KEYWORD;
}

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

static int IsWordTokenNode(struct AstNode* node) {
    return IsTokenNode(node) && IsWordToken(&node->token);
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

static void AddLocalType(struct CWriter* writer, const char* name, const char* type) {
    AddLocalTypeEx(writer, name, type, 0);
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

static int IsMainReturnType(struct AstNode* node, struct SourceFile* file) {
    return IsTokenNode(node)
        && IsTokenNode(node->nextSibling)
        && IsTokenNode(node->nextSibling->nextSibling)
        && TokenTextEquals(&node->token, file, "i64")
        && IsPunctuationToken(&node->nextSibling->token, PUNCTUATION_COLON)
        && TokenTextEquals(&node->nextSibling->nextSibling->token, file, "main");
}

static int ShouldSkipKekColon(struct AstNode* node, struct AstNode* previous) {
    return IsTokenNode(node)
        && IsPunctuationToken(&node->token, PUNCTUATION_COLON)
        && IsWordTokenNode(previous)
        && previous->token.type != TOKEN_KEYWORD
        && IsWordTokenNode(node->nextSibling)
        && node->nextSibling->token.type == TOKEN_IDENTIFIER;
}

static size_t NodeEndOffset(struct AstNode* node) {
    return node->location.offset + node->location.length;
}

static int SkipWhitespaceAndComments(struct SourceFile* file, size_t* offset) {
    for (;;) {
        while (*offset < file->length && isspace((unsigned char)file->content[*offset])) {
            (*offset)++;
        }

        if (*offset + 1 < file->length && file->content[*offset] == '/' && file->content[*offset + 1] == '/') {
            *offset += 2;
            while (*offset < file->length && file->content[*offset] != '\n') {
                (*offset)++;
            }
            continue;
        }

        if (*offset + 1 < file->length && file->content[*offset] == '/' && file->content[*offset + 1] == '*') {
            *offset += 2;
            while (*offset + 1 < file->length) {
                if (file->content[*offset] == '*' && file->content[*offset + 1] == '/') {
                    *offset += 2;
                    break;
                }
                (*offset)++;
            }
            continue;
        }

        break;
    }

    return *offset < file->length;
}

static char StatementSeparator(struct SourceFile* file, struct AstNode* statement) {
    size_t offset = NodeEndOffset(statement);
    if (!SkipWhitespaceAndComments(file, &offset)) {
        return '\0';
    }
    if (file->content[offset] == ';' || file->content[offset] == ',') {
        return file->content[offset];
    }
    return '\0';
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

static void ResetStatementState(struct CWriter* writer) {
    writer->atLineStart = 1;
    writer->needSpace = 0;
    writer->previous = NULL;
}

static int WriteStatement(struct CWriter* writer, struct AstNode* statement);
static void WriteStatementList(struct CWriter* writer, struct AstNode* list, int multiline);
static void WriteDelimited(struct CWriter* writer, struct AstNode* node, char open, char close);
static void WriteTokenNode(struct CWriter* writer, struct AstNode* node);
static void WriteStatementChildren(struct CWriter* writer, struct AstNode* firstChild);

static void WriteStatementList(struct CWriter* writer, struct AstNode* list, int multiline) {
    for (struct AstNode* statement = list->firstChild; statement; statement = statement->nextSibling) {
        if (multiline && writer->atLineStart) {
            WriteIndent(writer->out, writer->indent);
            writer->atLineStart = 0;
        }

        if (!WriteStatement(writer, statement)) {
            ResetStatementState(writer);
            continue;
        }

        char separator = StatementSeparator(writer->file, statement);
        if (separator == ';' || separator == ',') {
            fputc(separator, writer->out);
        }

        if (multiline && separator == ',' && list->type == AST_FILE) {
            writer->needSpace = 0;
            writer->previous = NULL;
            continue;
        }

        if (multiline) {
            fputc('\n', writer->out);
            ResetStatementState(writer);
        } else if (separator) {
            writer->needSpace = separator == ';';
            writer->previous = NULL;
        }
    }
}

static void WriteBlock(struct CWriter* writer, struct AstNode* block) {
    if (writer->previous && !IsPunctuationToken(writer->previous, PUNCTUATION_LEFT_PAREN)) {
        fputc(' ', writer->out);
    }

    fputs("{\n", writer->out);
    writer->indent++;
    ResetStatementState(writer);
    WriteStatementList(writer, block, 1);
    if (writer->indent > 0) {
        writer->indent--;
    }
    WriteIndent(writer->out, writer->indent);
    fputc('}', writer->out);
    writer->atLineStart = 0;
    writer->needSpace = 0;
    writer->previous = NULL;
}

static void WriteStructField(struct CWriter* writer, struct AstNode* field) {
    struct AstNode* previousChild = NULL;
    for (struct AstNode* child = field->firstChild; child; child = child->nextSibling) {
        if (IsTokenNode(child) && IsOperatorToken(&child->token, OPERATOR_ASSIGN)) {
            break;
        }

        if (ShouldSkipKekColon(child, previousChild)) {
            previousChild = child;
            continue;
        }

        switch (child->type) {
            case AST_TOKEN:
                WriteTokenNode(writer, child);
                break;
            case AST_BLOCK:
                WriteBlock(writer, child);
                break;
            case AST_GROUP:
                WriteDelimited(writer, child, '(', ')');
                break;
            case AST_INDEX:
                WriteDelimited(writer, child, '[', ']');
                break;
            case AST_GENERIC:
                WriteDelimited(writer, child, '<', '>');
                break;
            case AST_STATEMENT:
            case AST_FILE:
                WriteStatementList(writer, child, child->type == AST_FILE);
                break;
        }

        previousChild = child;
    }
}

static void WriteStructBlock(struct CWriter* writer, struct AstNode* block) {
    fputs(" {\n", writer->out);
    writer->indent++;
    ResetStatementState(writer);
    for (struct AstNode* field = block->firstChild; field; field = field->nextSibling) {
        if (writer->atLineStart) {
            WriteIndent(writer->out, writer->indent);
            writer->atLineStart = 0;
        }

        WriteStructField(writer, field);

        char separator = StatementSeparator(writer->file, field);
        if (separator == ';' || separator == ',') {
            fputc(separator, writer->out);
        }
        fputc('\n', writer->out);
        ResetStatementState(writer);
    }
    if (writer->indent > 0) {
        writer->indent--;
    }
    WriteIndent(writer->out, writer->indent);
    fputc('}', writer->out);
    writer->atLineStart = 0;
    writer->needSpace = 0;
    writer->previous = NULL;
}

static void WriteAttributeList(struct CWriter* writer, struct AstNode* attributes) {
    for (struct AstNode* statement = attributes->firstChild; statement; statement = statement->nextSibling) {
        if (!statement->firstChild || !IsTokenNode(statement->firstChild)) {
            continue;
        }

        if (writer->needSpace) {
            fputc(' ', writer->out);
        }

        char buffer[256];
        fputs(CTokenText(&statement->firstChild->token, writer->file, buffer, sizeof(buffer)), writer->out);
        writer->needSpace = 1;
    }
}

static int AttributeListContains(struct CWriter* writer, struct AstNode* attributes, const char* name) {
    if (!attributes || attributes->type != AST_INDEX) {
        return 0;
    }

    for (struct AstNode* statement = attributes->firstChild; statement; statement = statement->nextSibling) {
        if (statement->firstChild && IsTokenNode(statement->firstChild)) {
            char buffer[256];
            const char* text = CTokenText(&statement->firstChild->token, writer->file, buffer, sizeof(buffer));
            if (strcmp(text, name) == 0) {
                return 1;
            }
        }
    }

    return 0;
}

static int ParameterDefaultText(struct CWriter* writer, struct AstNode* parameter, char* buffer, size_t bufferSize) {
    for (struct AstNode* child = parameter->firstChild; child; child = child->nextSibling) {
        if (IsTokenNode(child) && IsOperatorToken(&child->token, OPERATOR_ASSIGN)) {
            struct AstNode* value = child->nextSibling;
            if (IsTokenNode(value)) {
                CopyTokenText(&value->token, writer->file, buffer, bufferSize);
                return 1;
            }
            break;
        }
    }

    if (bufferSize > 0) {
        buffer[0] = '\0';
    }
    return 0;
}

static int ParameterNameText(struct CWriter* writer, struct AstNode* parameter, char* buffer, size_t bufferSize) {
    for (struct AstNode* child = parameter->firstChild; child; child = child->nextSibling) {
        if (IsTokenNode(child)
            && child->token.type == TOKEN_IDENTIFIER
            && IsTokenNode(child->nextSibling)
            && IsOperatorToken(&child->nextSibling->token, OPERATOR_ASSIGN)) {
            CopyTokenText(&child->token, writer->file, buffer, bufferSize);
            return 1;
        }
    }

    if (bufferSize > 0) {
        buffer[0] = '\0';
    }
    return 0;
}

static int FieldNameText(struct CWriter* writer, struct AstNode* field, char* buffer, size_t bufferSize) {
    struct AstNode* type = field ? field->firstChild : NULL;
    struct AstNode* colon = NextSibling(type);
    struct AstNode* name = NextSibling(colon);
    if (IsTokenNode(type)
        && IsTokenNode(colon)
        && IsPunctuationToken(&colon->token, PUNCTUATION_COLON)
        && IsTokenNode(name)
        && name->token.type == TOKEN_IDENTIFIER) {
        CopyTokenText(&name->token, writer->file, buffer, bufferSize);
        return 1;
    }

    if (bufferSize > 0) {
        buffer[0] = '\0';
    }
    return 0;
}

static int FieldDefaultText(struct CWriter* writer, struct AstNode* field, char* buffer, size_t bufferSize) {
    for (struct AstNode* child = field ? field->firstChild : NULL; child; child = child->nextSibling) {
        if (IsTokenNode(child) && IsOperatorToken(&child->token, OPERATOR_ASSIGN)) {
            struct AstNode* value = child->nextSibling;
            if (IsTokenNode(value)) {
                CopyTokenText(&value->token, writer->file, buffer, bufferSize);
                return 1;
            }
            break;
        }
    }

    if (bufferSize > 0) {
        snprintf(buffer, bufferSize, "0");
    }
    return 0;
}

static void RegisterStructFields(struct CWriter* writer, struct CStructInfo* info, struct AstNode* block) {
    if (!info || !block || block->type != AST_BLOCK) {
        return;
    }

    for (struct AstNode* field = block->firstChild; field && info->fieldCount < 64; field = field->nextSibling) {
        if (FieldNameText(writer, field, info->fieldNames[info->fieldCount], sizeof(info->fieldNames[info->fieldCount]))) {
            FieldDefaultText(writer, field, info->defaults[info->fieldCount], sizeof(info->defaults[info->fieldCount]));
            info->fieldCount++;
        }
    }
}

static void WriteParameter(struct CWriter* writer, struct AstNode* parameter) {
    struct AstNode* previousChild = NULL;
    for (struct AstNode* child = parameter->firstChild; child; child = child->nextSibling) {
        if (IsTokenNode(child) && IsOperatorToken(&child->token, OPERATOR_ASSIGN)) {
            break;
        }

        if (ShouldSkipKekColon(child, previousChild)) {
            previousChild = child;
            continue;
        }

        if (child->type == AST_TOKEN) {
            WriteTokenNode(writer, child);
        } else if (child->type == AST_GROUP) {
            WriteDelimited(writer, child, '(', ')');
        } else if (child->type == AST_INDEX) {
            WriteDelimited(writer, child, '[', ']');
        }

        previousChild = child;
    }
}

static void RegisterFunctionDefaults(struct CWriter* writer, const char* functionName, struct AstNode* params) {
    if (!functionName || !params || params->type != AST_GROUP) {
        return;
    }

    struct CFunctionInfo* function = AddFunctionInfo(writer, functionName);
    if (!function) {
        return;
    }

    for (struct AstNode* parameter = params->firstChild; parameter && function->paramCount < 16; parameter = parameter->nextSibling) {
        ParameterNameText(writer, parameter, function->paramNames[function->paramCount], sizeof(function->paramNames[function->paramCount]));
        ParameterDefaultText(writer, parameter, function->defaults[function->paramCount], sizeof(function->defaults[function->paramCount]));
        function->paramCount++;
    }
}

static void WriteParameterList(struct CWriter* writer, struct AstNode* params, struct AstNode* nameNode) {
    if (params->childCount == 0) {
        (void)nameNode;
        fputs("void", writer->out);
        return;
    }

    int first = 1;
    for (struct AstNode* parameter = params->firstChild; parameter; parameter = parameter->nextSibling) {
        if (!first) {
            fputc(',', writer->out);
        }
        first = 0;
        writer->previous = NULL;
        writer->needSpace = 0;
        WriteParameter(writer, parameter);
    }
}

static int TryWriteFunction(struct CWriter* writer, struct AstNode* statement) {
    struct AstNode* attributes = NULL;
    struct AstNode* returnType = statement->firstChild;
    if (returnType && returnType->type == AST_INDEX) {
        attributes = returnType;
        returnType = returnType->nextSibling;
    }

    struct AstNode* colon = NextSibling(returnType);
    struct AstNode* name = NextSibling(colon);
    struct AstNode* params = NextSibling(name);
    struct AstNode* block = NextSibling(params);

    if (!IsTokenNode(returnType)
        || !IsTokenNode(colon)
        || !IsPunctuationToken(&colon->token, PUNCTUATION_COLON)
        || !IsTokenNode(name)
        || name->token.type != TOKEN_IDENTIFIER
        || !params
        || params->type != AST_GROUP
        || !block
        || block->type != AST_BLOCK) {
        return 0;
    }

    char nameBuffer[64];
    char cFunctionName[128];
    CopyTokenText(&name->token, writer->file, nameBuffer, sizeof(nameBuffer));
    snprintf(cFunctionName, sizeof(cFunctionName), "%s%s", writer->namespacePrefix, nameBuffer);
    RegisterFunctionDefaults(writer, cFunctionName, params);

    if (attributes) {
        WriteAttributeList(writer, attributes);
    }

    writer->previous = NULL;
    writer->needSpace = attributes != NULL;
    WriteTokenNode(writer, returnType);
    fputc(' ', writer->out);
    writer->needSpace = 0;
    fputs(cFunctionName, writer->out);
    fputc('(', writer->out);
    WriteParameterList(writer, params, name);
    fputc(')', writer->out);
    writer->previous = &name->token;
    writer->needSpace = 0;
    WriteBlock(writer, block);
    return 1;
}

static void WriteEnumVariant(struct CWriter* writer, const char* enumName, struct AstNode* variant) {
    struct AstNode* name = variant ? variant->firstChild : NULL;
    if (!IsTokenNode(name) || name->token.type != TOKEN_IDENTIFIER) {
        return;
    }

    char nameBuffer[64];
    CopyTokenText(&name->token, writer->file, nameBuffer, sizeof(nameBuffer));
    fprintf(writer->out, "%s_%s", enumName, nameBuffer);

    for (struct AstNode* child = name->nextSibling; child; child = child->nextSibling) {
        switch (child->type) {
            case AST_TOKEN:
                WriteTokenNode(writer, child);
                break;
            case AST_GROUP:
                WriteDelimited(writer, child, '(', ')');
                break;
            case AST_INDEX:
                WriteDelimited(writer, child, '[', ']');
                break;
            case AST_GENERIC:
                WriteDelimited(writer, child, '<', '>');
                break;
            case AST_BLOCK:
                WriteBlock(writer, child);
                break;
            case AST_STATEMENT:
            case AST_FILE:
                WriteStatementList(writer, child, child->type == AST_FILE);
                break;
        }
    }
}

static int TryWriteEnum(struct CWriter* writer, struct AstNode* statement) {
    struct AstNode* keyword = statement->firstChild;
    struct AstNode* firstColon = NextSibling(keyword);
    struct AstNode* underlyingType = NextSibling(firstColon);
    struct AstNode* secondColon = NextSibling(underlyingType);
    struct AstNode* name = NextSibling(secondColon);
    struct AstNode* block = NextSibling(name);

    if (!IsTokenNode(keyword)
        || !IsKeywordToken(&keyword->token, KEYWORD_ENUM)
        || !IsTokenNode(firstColon)
        || !IsPunctuationToken(&firstColon->token, PUNCTUATION_COLON)
        || !IsWordTokenNode(underlyingType)
        || !IsTokenNode(secondColon)
        || !IsPunctuationToken(&secondColon->token, PUNCTUATION_COLON)
        || !IsTokenNode(name)
        || name->token.type != TOKEN_IDENTIFIER
        || !block
        || block->type != AST_BLOCK) {
        return 0;
    }

    (void)underlyingType;

    char enumName[64];
    CopyTokenText(&name->token, writer->file, enumName, sizeof(enumName));
    fprintf(writer->out, "typedef enum %s {\n", enumName);
    writer->indent++;
    ResetStatementState(writer);
    for (struct AstNode* variant = block->firstChild; variant; variant = variant->nextSibling) {
        WriteIndent(writer->out, writer->indent);
        writer->atLineStart = 0;
        writer->previous = NULL;
        writer->needSpace = 0;
        WriteEnumVariant(writer, enumName, variant);
        char separator = StatementSeparator(writer->file, variant);
        if (separator == ',' || separator == ';') {
            fputc(',', writer->out);
        }
        fputc('\n', writer->out);
    }
    if (writer->indent > 0) {
        writer->indent--;
    }
    WriteIndent(writer->out, writer->indent);
    fprintf(writer->out, "} %s", enumName);
    writer->atLineStart = 0;
    writer->needSpace = 0;
    writer->previous = &name->token;
    return 1;
}

static void WriteStructFunctionDefaults(struct CWriter* writer, const char* functionName, struct AstNode* params) {
    struct CFunctionInfo* function = AddFunctionInfo(writer, functionName);
    if (!function) {
        return;
    }

    snprintf(function->paramNames[function->paramCount++], sizeof(function->paramNames[0]), "this");

    for (struct AstNode* parameter = params->firstChild; parameter && function->paramCount < 16; parameter = parameter->nextSibling) {
        ParameterNameText(writer, parameter, function->paramNames[function->paramCount], sizeof(function->paramNames[function->paramCount]));
        ParameterDefaultText(writer, parameter, function->defaults[function->paramCount], sizeof(function->defaults[function->paramCount]));
        function->paramCount++;
    }
}

static int TryWriteStructFunction(struct CWriter* writer, struct AstNode* statement) {
    struct AstNode* returnType = statement->firstChild;
    struct AstNode* colon = NextSibling(returnType);
    struct AstNode* structName = NextSibling(colon);
    struct AstNode* scope = NextSibling(structName);
    struct AstNode* functionName = NextSibling(scope);
    struct AstNode* params = NextSibling(functionName);
    struct AstNode* block = NextSibling(params);

    if (!IsTokenNode(returnType)
        || !IsTokenNode(colon)
        || !IsPunctuationToken(&colon->token, PUNCTUATION_COLON)
        || !IsTokenNode(structName)
        || structName->token.type != TOKEN_IDENTIFIER
        || !IsTokenNode(scope)
        || !IsOperatorToken(&scope->token, OPERATOR_SCOPE)
        || !IsTokenNode(functionName)
        || functionName->token.type != TOKEN_IDENTIFIER
        || !params
        || params->type != AST_GROUP
        || !block
        || block->type != AST_BLOCK) {
        return 0;
    }

    char structBuffer[64];
    char functionBuffer[64];
    char cFunctionName[128];
    CopyTokenText(&structName->token, writer->file, structBuffer, sizeof(structBuffer));
    CopyTokenText(&functionName->token, writer->file, functionBuffer, sizeof(functionBuffer));
    snprintf(cFunctionName, sizeof(cFunctionName), "%s_%s", structBuffer, functionBuffer);
    WriteStructFunctionDefaults(writer, cFunctionName, params);

    writer->previous = NULL;
    writer->needSpace = 0;
    WriteTokenNode(writer, returnType);
    fprintf(writer->out, " %s(struct %s* this", cFunctionName, structBuffer);
    if (params->childCount > 0) {
        fputc(',', writer->out);
        WriteParameterList(writer, params, functionName);
    }
    fputc(')', writer->out);

    writer->previous = &functionName->token;
    writer->needSpace = 0;
    int previousThisIsPointer = writer->thisIsPointer;
    writer->thisIsPointer = 1;
    WriteBlock(writer, block);
    writer->thisIsPointer = previousThisIsPointer;
    return 1;
}

static int TryWriteStruct(struct CWriter* writer, struct AstNode* statement) {
    struct AstNode* attributes = NULL;
    struct AstNode* keyword = statement->firstChild;
    if (keyword && keyword->type == AST_INDEX) {
        attributes = keyword;
        keyword = keyword->nextSibling;
    }

    struct AstNode* colon = NextSibling(keyword);
    struct AstNode* name = NextSibling(colon);
    struct AstNode* block = NextSibling(name);

    if (!IsTokenNode(keyword)
        || !IsKeywordToken(&keyword->token, KEYWORD_STRUCT)
        || !IsTokenNode(colon)
        || !IsPunctuationToken(&colon->token, PUNCTUATION_COLON)
        || !IsTokenNode(name)
        || name->token.type != TOKEN_IDENTIFIER
        || !block
        || block->type != AST_BLOCK) {
        return 0;
    }

    char structBuffer[64];
    CopyTokenText(&name->token, writer->file, structBuffer, sizeof(structBuffer));
    struct CStructInfo* info = AddStructInfo(writer, structBuffer);
    RegisterStructFields(writer, info, block);

    fputs("struct", writer->out);
    if (AttributeListContains(writer, attributes, "packed")) {
        fputs(" __attribute__((packed))", writer->out);
    }

    fputc(' ', writer->out);
    writer->previous = NULL;
    writer->needSpace = 0;
    WriteTokenNode(writer, name);
    writer->previous = &name->token;
    writer->needSpace = 0;
    WriteStructBlock(writer, block);
    return 1;
}

static int TryWriteUnion(struct CWriter* writer, struct AstNode* statement) {
    struct AstNode* keyword = statement->firstChild;
    struct AstNode* colon = NextSibling(keyword);
    struct AstNode* name = NextSibling(colon);
    struct AstNode* block = NextSibling(name);

    if (!IsTokenNode(keyword)
        || !IsKeywordToken(&keyword->token, KEYWORD_UNION)
        || !IsTokenNode(colon)
        || !IsPunctuationToken(&colon->token, PUNCTUATION_COLON)
        || !IsTokenNode(name)
        || name->token.type != TOKEN_IDENTIFIER
        || !block
        || block->type != AST_BLOCK) {
        return 0;
    }

    char unionName[64];
    CopyTokenText(&name->token, writer->file, unionName, sizeof(unionName));
    fprintf(writer->out, "typedef union %s", unionName);
    writer->previous = &name->token;
    writer->needSpace = 0;
    WriteStructBlock(writer, block);
    fprintf(writer->out, " %s", unionName);
    return 1;
}

static int IsNamedArgument(struct AstNode* argument, char* name, size_t nameSize, struct AstNode** valueStart, struct CWriter* writer) {
    struct AstNode* first = argument ? argument->firstChild : NULL;
    struct AstNode* equals = first ? first->nextSibling : NULL;
    if (!IsTokenNode(first)
        || first->token.type != TOKEN_IDENTIFIER
        || !IsTokenNode(equals)
        || !IsOperatorToken(&equals->token, OPERATOR_ASSIGN)) {
        return 0;
    }

    CopyTokenText(&first->token, writer->file, name, nameSize);
    *valueStart = equals->nextSibling;
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

static void WriteCallValue(struct CWriter* writer, struct AstNode* argument) {
    char name[64];
    struct AstNode* valueStart = NULL;
    if (!IsNamedArgument(argument, name, sizeof(name), &valueStart, writer)) {
        valueStart = argument->firstChild;
    }

    writer->previous = NULL;
    writer->needSpace = 0;
    WriteStatementChildren(writer, valueStart);
}

static void WriteCallValueFrom(struct CWriter* writer, struct AstNode* valueStart) {
    writer->previous = NULL;
    writer->needSpace = 0;
    WriteStatementChildren(writer, valueStart);
}

static int TryWriteKnownFunctionCallArguments(struct CWriter* writer, struct AstNode* node, struct CFunctionInfo* function, int thisArgumentIsPointer) {
    struct AstNode* arguments[16] = {0};
    size_t positionalIndex = 0;
    int hasNamedArgument = 0;

    for (struct AstNode* argument = node->firstChild; argument; argument = argument->nextSibling) {
        char name[64];
        struct AstNode* valueStart = NULL;
        if (IsNamedArgument(argument, name, sizeof(name), &valueStart, writer)) {
            int parameterIndex = FindParameterIndex(function, name);
            if (parameterIndex >= 0 && (size_t)parameterIndex < sizeof(arguments) / sizeof(arguments[0])) {
                arguments[parameterIndex] = argument;
                hasNamedArgument = 1;
            }
        } else if (positionalIndex < sizeof(arguments) / sizeof(arguments[0])) {
            while (positionalIndex < function->paramCount && arguments[positionalIndex]) {
                positionalIndex++;
            }
            if (positionalIndex < sizeof(arguments) / sizeof(arguments[0])) {
                arguments[positionalIndex++] = argument;
            }
        }
    }

    if (!hasNamedArgument && node->childCount < function->paramCount) {
        return 0;
    }

    for (size_t i = 0; i < function->paramCount; i++) {
        if (i > 0) {
            fputc(',', writer->out);
        }

        if (arguments[i]) {
            if (thisArgumentIsPointer && i == 0) {
                fputc('&', writer->out);
                WriteCallValueFrom(writer, arguments[i]->firstChild);
            } else {
                WriteCallValue(writer, arguments[i]);
            }
        } else {
            fputs(function->defaults[i], writer->out);
        }
    }

    return 1;
}

static void WriteDelimited(struct CWriter* writer, struct AstNode* node, char open, char close) {
    int isMainEmptyGroup = open == '('
        && node->childCount == 0
        && writer->previous
        && TokenTextEquals(writer->previous, writer->file, "main");
    char callName[64];
    struct CFunctionInfo* function = NULL;

    if (open == '(' && writer->previous && writer->previous->type == TOKEN_IDENTIFIER) {
        CopyTokenText(writer->previous, writer->file, callName, sizeof(callName));
        function = FindFunctionInfo(writer, callName);
    }

    fputc(open, writer->out);
    if (isMainEmptyGroup) {
        fputs("void", writer->out);
    } else {
        struct Token* previous = writer->previous;
        int needSpace = writer->needSpace;
        writer->previous = NULL;
        writer->needSpace = 0;
        if (!function || !TryWriteKnownFunctionCallArguments(writer, node, function, 0)) {
            WriteStatementList(writer, node, 0);
        }
        writer->previous = previous;
        writer->needSpace = needSpace;
    }
    fputc(close, writer->out);
    writer->needSpace = 0;
}

static void WriteTokenNode(struct CWriter* writer, struct AstNode* node) {
    struct Token* token = &node->token;
    if (token->type == TOKEN_EOF) {
        return;
    }

    if (IsKeywordToken(token, KEYWORD_EXPORT)) {
        writer->previous = token;
        return;
    }

    if (IsOperatorToken(token, OPERATOR_SCOPE)) {
        int isLeadingScope = writer->previous == NULL
            || (writer->previous->type == TOKEN_OPERATOR && writer->previous->value.operator != OPERATOR_SCOPE)
            || IsPunctuationToken(writer->previous, PUNCTUATION_SEMICOLON)
            || IsPunctuationToken(writer->previous, PUNCTUATION_COMMA)
            || IsPunctuationToken(writer->previous, PUNCTUATION_LEFT_BRACE)
            || IsPunctuationToken(writer->previous, PUNCTUATION_RIGHT_BRACE)
            || IsPunctuationToken(writer->previous, PUNCTUATION_LEFT_PAREN);
        if (!isLeadingScope) {
            fputc('_', writer->out);
        }
        writer->needSpace = 0;
        writer->previous = token;
        return;
    }

    if (writer->needSpace && IsWordToken(token)) {
        fputc(' ', writer->out);
    }

    char buffer[256];
    const char* text = IsMainReturnType(node, writer->file) ? "int" : CTokenText(token, writer->file, buffer, sizeof(buffer));
    if (token->type == TOKEN_IDENTIFIER && IsKnownStruct(writer, text) && node->nextSibling && IsTokenNode(node->nextSibling) && IsPunctuationToken(&node->nextSibling->token, PUNCTUATION_COLON)) {
        fputs("struct ", writer->out);
    }
    fputs(text, writer->out);

    writer->needSpace = IsWordToken(token);
    writer->previous = token;
}

static int TryWriteStructVariableDeclaration(struct CWriter* writer, struct AstNode* firstChild) {
    if (!IsTokenNode(firstChild)
        || firstChild->token.type != TOKEN_IDENTIFIER
        || !IsTokenNode(firstChild->nextSibling)
        || !IsPunctuationToken(&firstChild->nextSibling->token, PUNCTUATION_COLON)
        || !IsTokenNode(firstChild->nextSibling->nextSibling)
        || firstChild->nextSibling->nextSibling->token.type != TOKEN_IDENTIFIER) {
        return 0;
    }

    char typeName[64];
    char variableName[64];
    CopyTokenText(&firstChild->token, writer->file, typeName, sizeof(typeName));
    struct CStructInfo* info = FindStructInfo(writer, typeName);
    if (!info) {
        return 0;
    }

    struct AstNode* nameNode = firstChild->nextSibling->nextSibling;
    if (nameNode->nextSibling) {
        return 0;
    }

    CopyTokenText(&nameNode->token, writer->file, variableName, sizeof(variableName));
    fprintf(writer->out, "struct %s %s = {", typeName, variableName);
    if (info->fieldCount == 0) {
        fputc('0', writer->out);
    } else {
        for (size_t i = 0; i < info->fieldCount; i++) {
            if (i > 0) {
                fputc(',', writer->out);
            }
            fprintf(writer->out, ".%s=%s", info->fieldNames[i], info->defaults[i][0] ? info->defaults[i] : "0");
        }
    }
    fputc('}', writer->out);
    writer->previous = &nameNode->token;
    writer->needSpace = 0;
    return 1;
}

static void WriteStatementChildren(struct CWriter* writer, struct AstNode* firstChild) {
    struct AstNode* previousChild = NULL;

    if (IsTokenNode(firstChild)
        && firstChild->token.type == TOKEN_IDENTIFIER
        && IsTokenNode(firstChild->nextSibling)
        && IsPunctuationToken(&firstChild->nextSibling->token, PUNCTUATION_COLON)
        && IsTokenNode(firstChild->nextSibling->nextSibling)
        && firstChild->nextSibling->nextSibling->token.type == TOKEN_IDENTIFIER) {
        char typeName[64];
        char variableName[64];
        CopyTokenText(&firstChild->token, writer->file, typeName, sizeof(typeName));
        CopyTokenText(&firstChild->nextSibling->nextSibling->token, writer->file, variableName, sizeof(variableName));
        AddLocalType(writer, variableName, typeName);
    }

    if (TryWriteStructVariableDeclaration(writer, firstChild)) {
        return;
    }

    for (struct AstNode* child = firstChild; child; child = child->nextSibling) {
        if (ShouldSkipKekColon(child, previousChild)) {
            previousChild = child;
            continue;
        }

        if (IsTokenNode(child)
            && TokenTextEquals(&child->token, writer->file, "cast")
            && IsTokenNode(child->nextSibling)
            && IsOperatorToken(&child->nextSibling->token, OPERATOR_LESS)
            && IsTokenNode(child->nextSibling->nextSibling)
            && IsTokenNode(child->nextSibling->nextSibling->nextSibling)
            && IsOperatorToken(&child->nextSibling->nextSibling->nextSibling->token, OPERATOR_GREATER)
            && child->nextSibling->nextSibling->nextSibling->nextSibling
            && child->nextSibling->nextSibling->nextSibling->nextSibling->type == AST_GROUP) {
            struct AstNode* typeNode = child->nextSibling->nextSibling;
            struct AstNode* valueGroup = child->nextSibling->nextSibling->nextSibling->nextSibling;
            fputs("((", writer->out);
            writer->previous = NULL;
            writer->needSpace = 0;
            WriteTokenNode(writer, typeNode);
            fputc(')', writer->out);
            WriteDelimited(writer, valueGroup, '(', ')');
            fputc(')', writer->out);
            writer->previous = &child->token;
            writer->needSpace = 0;
            child = valueGroup;
            previousChild = child;
            continue;
        }

        if (writer->thisIsPointer
            && IsTokenNode(child)
            && TokenTextEquals(&child->token, writer->file, "this")
            && IsTokenNode(child->nextSibling)
            && IsPunctuationToken(&child->nextSibling->token, PUNCTUATION_DOT)) {
            WriteTokenNode(writer, child);
            fputs("->", writer->out);
            writer->needSpace = 0;
            child = child->nextSibling;
            previousChild = child;
            continue;
        }

        if (IsTokenNode(child)
            && child->token.type == TOKEN_IDENTIFIER
            && IsKnownStruct(writer, CTokenText(&child->token, writer->file, (char[256]){0}, 256))
            && IsTokenNode(child->nextSibling)
            && IsOperatorToken(&child->nextSibling->token, OPERATOR_SCOPE)
            && IsTokenNode(child->nextSibling->nextSibling)
            && child->nextSibling->nextSibling->token.type == TOKEN_IDENTIFIER
            && child->nextSibling->nextSibling->nextSibling
            && child->nextSibling->nextSibling->nextSibling->type == AST_GROUP) {
            char structBuffer[64];
            char methodBuffer[64];
            char cFunctionName[128];
            struct AstNode* methodName = child->nextSibling->nextSibling;
            struct AstNode* args = methodName->nextSibling;
            CopyTokenText(&child->token, writer->file, structBuffer, sizeof(structBuffer));
            CopyTokenText(&methodName->token, writer->file, methodBuffer, sizeof(methodBuffer));
            snprintf(cFunctionName, sizeof(cFunctionName), "%s_%s", structBuffer, methodBuffer);
            fputs(cFunctionName, writer->out);
            fputc('(', writer->out);
            struct CFunctionInfo* function = FindFunctionInfo(writer, cFunctionName);
            if (!function || !TryWriteKnownFunctionCallArguments(writer, args, function, 1)) {
                WriteStatementList(writer, args, 0);
            }
            fputc(')', writer->out);
            writer->previous = &methodName->token;
            writer->needSpace = 0;
            child = args;
            previousChild = child;
            continue;
        }

        if (IsTokenNode(child)
            && child->token.type == TOKEN_IDENTIFIER
            && IsTokenNode(child->nextSibling)
            && IsPunctuationToken(&child->nextSibling->token, PUNCTUATION_DOT)
            && IsTokenNode(child->nextSibling->nextSibling)
            && child->nextSibling->nextSibling->token.type == TOKEN_IDENTIFIER
            && child->nextSibling->nextSibling->nextSibling
            && child->nextSibling->nextSibling->nextSibling->type == AST_GROUP) {
            char receiverName[64];
            char methodBuffer[64];
            char cFunctionName[128];
            CopyTokenText(&child->token, writer->file, receiverName, sizeof(receiverName));
            const char* receiverType = FindLocalType(writer, receiverName);
            if (receiverType && IsKnownStruct(writer, receiverType)) {
                struct AstNode* methodName = child->nextSibling->nextSibling;
                struct AstNode* args = methodName->nextSibling;
                CopyTokenText(&methodName->token, writer->file, methodBuffer, sizeof(methodBuffer));
                snprintf(cFunctionName, sizeof(cFunctionName), "%s_%s", receiverType, methodBuffer);
                fputs(cFunctionName, writer->out);
                fputc('(', writer->out);
                fputc('&', writer->out);
                WriteTokenNode(writer, child);
                if (args->childCount > 0) {
                    fputc(',', writer->out);
                    struct CFunctionInfo* function = FindFunctionInfo(writer, cFunctionName);
                    if (function && function->paramCount > 1) {
                        struct CFunctionInfo callFunction = *function;
                        for (size_t i = 1; i < function->paramCount; i++) {
                            snprintf(callFunction.paramNames[i - 1], sizeof(callFunction.paramNames[0]), "%s", function->paramNames[i]);
                            snprintf(callFunction.defaults[i - 1], sizeof(callFunction.defaults[0]), "%s", function->defaults[i]);
                        }
                        callFunction.paramCount = function->paramCount - 1;
                        if (!TryWriteKnownFunctionCallArguments(writer, args, &callFunction, 0)) {
                            WriteStatementList(writer, args, 0);
                        }
                    } else {
                        WriteStatementList(writer, args, 0);
                    }
                }
                fputc(')', writer->out);
                writer->previous = &methodName->token;
                writer->needSpace = 0;
                child = args;
                previousChild = child;
                continue;
            }
        }

        switch (child->type) {
            case AST_TOKEN:
                WriteTokenNode(writer, child);
                break;
            case AST_BLOCK:
                WriteBlock(writer, child);
                break;
            case AST_GROUP:
                WriteDelimited(writer, child, '(', ')');
                break;
            case AST_INDEX:
                WriteDelimited(writer, child, '[', ']');
                break;
            case AST_GENERIC:
                WriteDelimited(writer, child, '<', '>');
                break;
            case AST_STATEMENT:
            case AST_FILE:
                WriteStatementList(writer, child, child->type == AST_FILE);
                break;
        }

        previousChild = child;
    }
}

static int TryWriteAlias(struct CWriter* writer, struct AstNode* statement) {
    struct AstNode* alias = statement->firstChild;
    struct AstNode* name = alias ? alias->nextSibling : NULL;
    if (IsTokenNode(name) && IsPunctuationToken(&name->token, PUNCTUATION_COLON)) {
        name = name->nextSibling;
    }
    struct AstNode* equals = name ? name->nextSibling : NULL;
    struct AstNode* type = equals ? equals->nextSibling : NULL;

    if (!IsTokenNode(alias)
        || !IsKeywordToken(&alias->token, KEYWORD_ALIAS)
        || !IsTokenNode(name)
        || name->token.type != TOKEN_IDENTIFIER
        || !IsTokenNode(equals)
        || !IsOperatorToken(&equals->token, OPERATOR_ASSIGN)
        || !IsWordTokenNode(type)) {
        return 0;
    }

    char nameBuffer[256];
    char typeBuffer[256];
    fprintf(writer->out, "typedef %s %s",
        CTokenText(&type->token, writer->file, typeBuffer, sizeof(typeBuffer)),
        CTokenText(&name->token, writer->file, nameBuffer, sizeof(nameBuffer)));
    ResetStatementState(writer);
    return 1;
}

static int TryWriteSwitch(struct CWriter* writer, struct AstNode* statement) {
    struct AstNode* keyword = statement->firstChild;
    if (!IsTokenNode(keyword) || !IsKeywordToken(&keyword->token, KEYWORD_SWITCH)) {
        return 0;
    }

    struct AstNode* block = NULL;
    for (struct AstNode* child = keyword->nextSibling; child; child = child->nextSibling) {
        if (child->type == AST_BLOCK) {
            block = child;
            break;
        }
    }

    if (!block || block == keyword->nextSibling) {
        return 0;
    }

    fputs("switch (", writer->out);
    writer->previous = NULL;
    writer->needSpace = 0;
    for (struct AstNode* child = keyword->nextSibling; child && child != block; child = child->nextSibling) {
        switch (child->type) {
            case AST_TOKEN:
                WriteTokenNode(writer, child);
                break;
            case AST_GROUP:
                WriteDelimited(writer, child, '(', ')');
                break;
            case AST_INDEX:
                WriteDelimited(writer, child, '[', ']');
                break;
            case AST_GENERIC:
                WriteDelimited(writer, child, '<', '>');
                break;
            case AST_BLOCK:
                break;
            case AST_STATEMENT:
            case AST_FILE:
                WriteStatementList(writer, child, child->type == AST_FILE);
                break;
        }
    }
    fputc(')', writer->out);
    writer->previous = &keyword->token;
    writer->needSpace = 0;
    WriteBlock(writer, block);
    return 1;
}

static int TryWriteExternC(struct CWriter* writer, struct AstNode* statement) {
    struct AstNode* externNode = statement->firstChild;
    struct AstNode* abi = externNode ? externNode->nextSibling : NULL;
    struct AstNode* block = abi ? abi->nextSibling : NULL;

    if (!IsTokenNode(externNode)
        || !IsKeywordToken(&externNode->token, KEYWORD_EXTERN)
        || !IsTokenNode(abi)
        || abi->token.type != TOKEN_STRING
        || !TokenTextEquals(&abi->token, writer->file, "\"C\"")
        || !block
        || block->type != AST_BLOCK) {
        return 0;
    }

    size_t start = block->location.offset + 1;
    size_t end = block->location.offset + block->location.length;
    if (end > start) {
        end--;
    }

    if (end > start) {
        fwrite(writer->file->content + start, 1, end - start, writer->out);
        if (writer->file->content[end - 1] != '\n') {
            fputc('\n', writer->out);
        }
    }

    ResetStatementState(writer);
    return 1;
}

static int WriteStatement(struct CWriter* writer, struct AstNode* statement) {
    if (!statement || statement->type != AST_STATEMENT || statement->childCount == 0) {
        return 0;
    }

    struct AstNode* first = statement->firstChild;
    if (IsTokenNode(first)
        && IsPunctuationToken(&first->token, PUNCTUATION_HASH)
        && IsTokenNode(first->nextSibling)
        && TokenTextEquals(&first->nextSibling->token, writer->file, "import")) {
        ResetStatementState(writer);
        return 0;
    }

    if (IsTokenNode(first) && IsKeywordToken(&first->token, KEYWORD_USING)) {
        ResetStatementState(writer);
        return 0;
    }

    if (TryWriteAlias(writer, statement)
        || TryWriteExternC(writer, statement)
        || TryWriteEnum(writer, statement)
        || TryWriteUnion(writer, statement)
        || TryWriteStruct(writer, statement)
        || TryWriteStructFunction(writer, statement)
        || TryWriteSwitch(writer, statement)
        || TryWriteFunction(writer, statement)) {
        return 1;
    }

    WriteStatementChildren(writer, statement->firstChild);

    return 1;
}

void WriteC(FILE* out, struct AstNode* ast, struct SourceFile* file) {
    struct CWriter writer = {0};
    writer.out = out;
    writer.file = file;
    writer.atLineStart = 1;

    WritePrelude(out);
    WriteStatementList(&writer, ast, 1);
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

int WriteCFileForFiles(const char* path, struct AstNode** asts, struct SourceFile** files, size_t count) {
    FILE* out = fopen(path, "w");
    if (!out) {
        return -1;
    }

    struct CWriter writer = {0};
    writer.out = out;
    writer.atLineStart = 1;

    WritePrelude(out);
    for (size_t i = 0; i < count; i++) {
        writer.file = files[i];
        if (i + 1 < count) {
            PackagePrefixFromPath(files[i]->path, writer.namespacePrefix, sizeof(writer.namespacePrefix));
        } else {
            writer.namespacePrefix[0] = '\0';
        }
        WriteStatementList(&writer, asts[i], 1);
    }

    fclose(out);
    return 0;
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

    char prefix[64] = {0};
    if (declFile) {
        PackagePrefixFromPath(declFile->path, prefix, sizeof(prefix));
    }
    char base[128];
    MangleGenericNameWithFiles(writer, declFile, decl->name, argsFile, args, base, sizeof(base));
    char instanceName[128];
    snprintf(instanceName, sizeof(instanceName), "%s", prefix);
    AppendSanitized(instanceName, sizeof(instanceName), base);
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
                WriteTypedKnownCallArgs(writer, expr, function, expr->callee->left, 0);
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
        CopyTypedNodeText(writer, base->name, typeBuffer, sizeof(typeBuffer));
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
    return decl
        && decl->kind == KEK_DECL_FUNCTION
        && decl->type
        && NextSibling(NextSibling(decl->type))
        && IsTokenNode(NextSibling(NextSibling(decl->type)))
        && NextSibling(NextSibling(NextSibling(decl->type)))
        && IsTokenNode(NextSibling(NextSibling(NextSibling(decl->type))))
        && IsOperatorToken(&NextSibling(NextSibling(NextSibling(decl->type)))->token, OPERATOR_SCOPE);
}

static struct AstNode* TypedDeclReceiverName(struct KekDecl* decl) {
    return TypedDeclIsMethod(decl) ? NextSibling(NextSibling(decl->type)) : NULL;
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

    WriteTypedBaseType(writer, decl->parsedType);
    fprintf(writer->out, " %s(", instance->name);
    WriteTypedParams(writer, decl);
    fputs(") ", writer->out);
    size_t previousLocalCount = writer->localCount;
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
    WriteTypedBlock(writer, decl->firstStmt);
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
            char returnName[128];
            TypedFunctionName(writer, decl, functionName, sizeof(functionName));
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
            fputs(") ", writer->out);
            int previousThisIsPointer = writer->thisIsPointer;
            size_t previousLocalCount = writer->localCount;
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
    writer.atLineStart = 1;

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
                ResetStatementState(&writer);
            }
        }
    }

    for (size_t i = 0; i < writer.genericFunctionCount; i++) {
        writer.file = writer.genericFunctions[i].declFile ? writer.genericFunctions[i].declFile : writer.file;
        RegisterTypedFunctionWithName(&writer, writer.genericFunctions[i].name, writer.genericFunctions[i].decl, 0);
    }

    for (size_t i = 0; i < writer.genericStructCount; i++) {
        WriteTypedGenericStructInstance(&writer, &writer.genericStructs[i]);
        fputc('\n', writer.out);
        ResetStatementState(&writer);
    }

    for (size_t i = 0; i < writer.genericFunctionCount; i++) {
        WriteTypedGenericFunctionInstance(&writer, &writer.genericFunctions[i]);
        fputc('\n', writer.out);
        ResetStatementState(&writer);
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
                ResetStatementState(&writer);
            }
        }
    }

    fclose(out);
    return 0;
}

int WriteCFile(const char* path, struct AstNode* ast, struct SourceFile* file) {
    FILE* out = fopen(path, "w");
    if (!out) {
        return -1;
    }
    WriteC(out, ast, file);
    fclose(out);
    return 0;
}
