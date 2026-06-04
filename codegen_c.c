#include "kek.h"

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

static int IsMainReturnType(struct TokenArray* tokens, struct SourceFile* file, size_t index) {
    return index + 2 < tokens->count
        && TokenTextEquals(&tokens->items[index], file, "i64")
        && IsPunctuationToken(&tokens->items[index + 1], PUNCTUATION_COLON)
        && TokenTextEquals(&tokens->items[index + 2], file, "main");
}

static int ShouldSkipKekColon(struct TokenArray* tokens, size_t index) {
    if (index == 0 || index + 1 >= tokens->count) {
        return 0;
    }

    struct Token* token = &tokens->items[index];
    struct Token* previous = &tokens->items[index - 1];
    struct Token* next = &tokens->items[index + 1];

    return IsPunctuationToken(token, PUNCTUATION_COLON)
        && IsWordToken(previous)
        && next->type == TOKEN_IDENTIFIER;
}

static size_t FindStatementEnd(struct TokenArray* tokens, size_t index) {
    int parenDepth = 0;
    int braceDepth = 0;
    int bracketDepth = 0;

    for (size_t i = index; i < tokens->count; i++) {
        struct Token* token = &tokens->items[i];
        if (token->type == TOKEN_EOF) {
            return i;
        }

        if (IsPunctuationToken(token, PUNCTUATION_LEFT_PAREN)) parenDepth++;
        if (IsPunctuationToken(token, PUNCTUATION_RIGHT_PAREN) && parenDepth > 0) parenDepth--;
        if (IsPunctuationToken(token, PUNCTUATION_LEFT_BRACE)) braceDepth++;
        if (IsPunctuationToken(token, PUNCTUATION_RIGHT_BRACE) && braceDepth > 0) braceDepth--;
        if (IsPunctuationToken(token, PUNCTUATION_LEFT_BRACKET)) bracketDepth++;
        if (IsPunctuationToken(token, PUNCTUATION_RIGHT_BRACKET) && bracketDepth > 0) bracketDepth--;

        if (parenDepth == 0 && braceDepth == 0 && bracketDepth == 0 && IsPunctuationToken(token, PUNCTUATION_SEMICOLON)) {
            return i;
        }
    }

    return tokens->count;
}

static int TryWriteAlias(FILE* out, struct TokenArray* tokens, struct SourceFile* file, size_t index, size_t* nextIndex) {
    size_t end = FindStatementEnd(tokens, index);
    if (end >= tokens->count || index + 3 >= end) {
        return 0;
    }

    struct Token* name = &tokens->items[index + 1];
    struct Token* equals = &tokens->items[index + 2];
    struct Token* type = &tokens->items[index + 3];
    if (name->type != TOKEN_IDENTIFIER || !IsOperatorToken(equals, OPERATOR_ASSIGN) || !IsWordToken(type)) {
        return 0;
    }

    char nameBuffer[256];
    char typeBuffer[256];
    fprintf(out, "typedef %s %s;\n",
        CTokenText(type, file, typeBuffer, sizeof(typeBuffer)),
        CTokenText(name, file, nameBuffer, sizeof(nameBuffer)));
    *nextIndex = end;
    return 1;
}

static int TryWriteExternC(FILE* out, struct TokenArray* tokens, struct SourceFile* file, size_t index, size_t* nextIndex) {
    if (index + 2 >= tokens->count
        || tokens->items[index + 1].type != TOKEN_STRING
        || !TokenTextEquals(&tokens->items[index + 1], file, "\"C\"")
        || !IsPunctuationToken(&tokens->items[index + 2], PUNCTUATION_LEFT_BRACE)) {
        return 0;
    }

    int depth = 0;
    size_t closeIndex = tokens->count;
    for (size_t i = index + 2; i < tokens->count; i++) {
        if (IsPunctuationToken(&tokens->items[i], PUNCTUATION_LEFT_BRACE)) {
            depth++;
        } else if (IsPunctuationToken(&tokens->items[i], PUNCTUATION_RIGHT_BRACE)) {
            depth--;
            if (depth == 0) {
                closeIndex = i;
                break;
            }
        }
    }

    if (closeIndex >= tokens->count) {
        return 0;
    }

    size_t start = tokens->items[index + 2].location.offset + tokens->items[index + 2].location.length;
    size_t end = tokens->items[closeIndex].location.offset;
    if (end > start) {
        fwrite(file->content + start, 1, end - start, out);
        if (file->content[end - 1] != '\n') {
            fputc('\n', out);
        }
    }

    *nextIndex = closeIndex;
    return 1;
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

void WriteC(FILE* out, struct TokenArray* tokens, struct SourceFile* file) {
    int indent = 0;
    int atLineStart = 1;
    int needSpace = 0;
    struct Token* previous = NULL;

    WritePrelude(out);

    for (size_t i = 0; i < tokens->count; i++) {
        struct Token* token = &tokens->items[i];
        if (token->type == TOKEN_EOF || ShouldSkipKekColon(tokens, i)) {
            continue;
        }

        if (IsKeywordToken(token, KEYWORD_USING)) {
            i = FindStatementEnd(tokens, i);
            atLineStart = 1;
            needSpace = 0;
            previous = NULL;
            continue;
        }

        if (IsKeywordToken(token, KEYWORD_ALIAS)) {
            if (TryWriteAlias(out, tokens, file, i, &i)) {
                atLineStart = 1;
                needSpace = 0;
                previous = NULL;
                continue;
            }
        }

        if (IsKeywordToken(token, KEYWORD_EXTERN)) {
            if (TryWriteExternC(out, tokens, file, i, &i)) {
                atLineStart = 1;
                needSpace = 0;
                previous = NULL;
                continue;
            }
        }

        if (IsKeywordToken(token, KEYWORD_EXPORT)) {
            previous = token;
            continue;
        }

        if (IsOperatorToken(token, OPERATOR_SCOPE)) {
            int isLeadingScope = previous == NULL
                || IsPunctuationToken(previous, PUNCTUATION_SEMICOLON)
                || IsPunctuationToken(previous, PUNCTUATION_LEFT_BRACE)
                || IsPunctuationToken(previous, PUNCTUATION_RIGHT_BRACE);
            if (!isLeadingScope) {
                fputc('_', out);
            }
            needSpace = 0;
            previous = token;
            continue;
        }

        char buffer[256];
        const char* text = IsMainReturnType(tokens, file, i) ? "int" : CTokenText(token, file, buffer, sizeof(buffer));

        if (IsPunctuationToken(token, PUNCTUATION_RIGHT_BRACE)) {
            if (!atLineStart) {
                fputc('\n', out);
            }
            if (indent > 0) {
                indent--;
            }
            WriteIndent(out, indent);
            fputs(text, out);
            fputc('\n', out);
            atLineStart = 1;
            needSpace = 0;
            previous = token;
            continue;
        }

        if (atLineStart) {
            WriteIndent(out, indent);
            atLineStart = 0;
        }

        if (needSpace && IsWordToken(token)) {
            fputc(' ', out);
        }

        if (IsPunctuationToken(token, PUNCTUATION_LEFT_BRACE)) {
            if (previous && !IsPunctuationToken(previous, PUNCTUATION_LEFT_PAREN)) {
                fputc(' ', out);
            }
            fputs(text, out);
            fputc('\n', out);
            indent++;
            atLineStart = 1;
            needSpace = 0;
            previous = token;
            continue;
        }

        if (IsPunctuationToken(token, PUNCTUATION_LEFT_PAREN)
            && previous
            && TokenTextEquals(previous, file, "main")
            && i + 1 < tokens->count
            && IsPunctuationToken(&tokens->items[i + 1], PUNCTUATION_RIGHT_PAREN)) {
            fputs("(void", out);
        } else {
            fputs(text, out);
        }

        if (IsPunctuationToken(token, PUNCTUATION_SEMICOLON)) {
            fputc('\n', out);
            atLineStart = 1;
            needSpace = 0;
        } else {
            needSpace = IsWordToken(token);
        }

        previous = token;
    }

    if (!atLineStart) {
        fputc('\n', out);
    }
}

int WriteCFile(const char* path, struct TokenArray* tokens, struct SourceFile* file) {
    FILE* out = fopen(path, "w");
    if (!out) {
        fprintf(stderr, "Error: Could not open %s for writing.\n", path);
        return -1;
    }
    WriteC(out, tokens, file);
    fclose(out);
    return 0;
}
