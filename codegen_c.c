#include "kek.h"

struct CFunctionInfo {
    char name[64];
    char paramNames[16][64];
    char defaults[16][64];
    size_t paramCount;
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
    char structs[64][64];
    size_t structCount;
    char localNames[128][64];
    char localTypes[128][64];
    size_t localCount;
    int thisIsPointer;
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
        if (strcmp(writer->structs[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

static void AddKnownStruct(struct CWriter* writer, const char* name) {
    if (IsKnownStruct(writer, name) || writer->structCount >= sizeof(writer->structs) / sizeof(writer->structs[0])) {
        return;
    }
    snprintf(writer->structs[writer->structCount++], sizeof(writer->structs[0]), "%s", name);
}

static void AddLocalType(struct CWriter* writer, const char* name, const char* type) {
    for (size_t i = 0; i < writer->localCount; i++) {
        if (strcmp(writer->localNames[i], name) == 0) {
            snprintf(writer->localTypes[i], sizeof(writer->localTypes[i]), "%s", type);
            return;
        }
    }

    if (writer->localCount >= sizeof(writer->localNames) / sizeof(writer->localNames[0])) {
        return;
    }

    snprintf(writer->localNames[writer->localCount], sizeof(writer->localNames[0]), "%s", name);
    snprintf(writer->localTypes[writer->localCount], sizeof(writer->localTypes[0]), "%s", type);
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
    fputs("#include <stdint.h>\n", out);
    fputs("#include <stddef.h>\n\n", out);
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

static void RegisterFunctionDefaults(struct CWriter* writer, struct AstNode* nameNode, struct AstNode* params) {
    if (!IsTokenNode(nameNode) || nameNode->token.type != TOKEN_IDENTIFIER || !params || params->type != AST_GROUP) {
        return;
    }

    char name[64];
    CopyTokenText(&nameNode->token, writer->file, name, sizeof(name));
    struct CFunctionInfo* function = AddFunctionInfo(writer, name);
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
        if (IsTokenNode(nameNode) && TokenTextEquals(&nameNode->token, writer->file, "main")) {
            fputs("void", writer->out);
        }
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

    RegisterFunctionDefaults(writer, name, params);

    if (attributes) {
        WriteAttributeList(writer, attributes);
    }

    writer->previous = NULL;
    writer->needSpace = attributes != NULL;
    WriteTokenNode(writer, returnType);
    fputc(' ', writer->out);
    writer->needSpace = 0;
    WriteTokenNode(writer, name);
    fputc('(', writer->out);
    WriteParameterList(writer, params, name);
    fputc(')', writer->out);
    writer->previous = &name->token;
    writer->needSpace = 0;
    WriteBlock(writer, block);
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
    AddKnownStruct(writer, structBuffer);

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
    WriteBlock(writer, block);
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
            || IsPunctuationToken(writer->previous, PUNCTUATION_SEMICOLON)
            || IsPunctuationToken(writer->previous, PUNCTUATION_LEFT_BRACE)
            || IsPunctuationToken(writer->previous, PUNCTUATION_RIGHT_BRACE);
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

    for (struct AstNode* child = firstChild; child; child = child->nextSibling) {
        if (ShouldSkipKekColon(child, previousChild)) {
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
    if (IsTokenNode(first) && IsKeywordToken(&first->token, KEYWORD_USING)) {
        ResetStatementState(writer);
        return 0;
    }

    if (TryWriteAlias(writer, statement)
        || TryWriteExternC(writer, statement)
        || TryWriteStruct(writer, statement)
        || TryWriteStructFunction(writer, statement)
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

int WriteCFile(const char* path, struct AstNode* ast, struct SourceFile* file) {
    FILE* out = fopen(path, "w");
    if (!out) {
        fprintf(stderr, "Error: Could not open %s for writing.\n", path);
        return -1;
    }
    WriteC(out, ast, file);
    fclose(out);
    return 0;
}
