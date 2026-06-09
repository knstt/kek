#include "kek.h"

const char* TokenTypeNames[] = {
    "EOF",
    "IDENTIFIER",
    "NUMBER",
    "STRING",
    "OPERATOR",
    "KEYWORD",
    "PUNCTUATION",
};

const char* OperatorNames[] = {
    "::", "==", "!=", "<=", ">=", "&&", "||", "+=", "-=", "->",
    "+", "-", "*", "/", "%", "=", "<", ">", "!", "&", "|", "~",
};

const char* KeywordNames[] = {
    "if", "else", "while", "for", "return", "do", "break", "continue",
    "using", "alias", "export", "extern", "enum", "struct", "union",
    "switch", "case", "default", "in", "packed", "aligned", "comptime",
    "defer", "tagged", "true", "false"
};

const char* PunctuationNames[] = {
    "(", ")", "{", "}", "[", "]", ";", ",", ":", ".", "#"
};

struct Tokenizer CreateTokenizer(int fileIndex, struct FileTable* table) {
    struct Tokenizer tokenizer = {0};
    tokenizer.file = &table->files[fileIndex];
    tokenizer.position = 0;
    tokenizer.line = 1;
    tokenizer.column = 1;
    return tokenizer;
}

static char PeekChar(struct Tokenizer* tokenizer) {
    if (tokenizer->position >= tokenizer->file->length) {
        return '\0';
    }
    return tokenizer->file->content[tokenizer->position];
}

static char PeekCharAt(struct Tokenizer* tokenizer, size_t offset) {
    size_t position = tokenizer->position + offset;
    if (position >= tokenizer->file->length) {
        return '\0';
    }
    return tokenizer->file->content[position];
}

static void Advance(struct Tokenizer* tokenizer, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (tokenizer->position < tokenizer->file->length) {
            char c = tokenizer->file->content[tokenizer->position];
            if (c == '\n') {
                tokenizer->line++;
                tokenizer->column = 1;
            } else {
                tokenizer->column++;
            }
            tokenizer->position++;
        }
    }
}

static struct Token CreateTextToken(enum TokenType type, size_t start, size_t length, size_t line, size_t column) {
    struct Token token = {0};
    token.type = type;
    token.value.text.offset = start;
    token.value.text.length = length;
    token.location.line = line;
    token.location.column = column;
    token.location.offset = start;
    token.location.length = length;
    return token;
}

static struct Token CreateOperatorToken(enum OperatorType opType, size_t start, size_t length, size_t line, size_t column) {
    struct Token token = {0};
    token.type = TOKEN_OPERATOR;
    token.value.operator = opType;
    token.location.line = line;
    token.location.column = column;
    token.location.offset = start;
    token.location.length = length;
    return token;
}

static struct Token CreateKeywordToken(enum KeywordType kwType, size_t start, size_t length, size_t line, size_t column) {
    struct Token token = {0};
    token.type = TOKEN_KEYWORD;
    token.value.keyword = kwType;
    token.location.line = line;
    token.location.column = column;
    token.location.offset = start;
    token.location.length = length;
    return token;
}

static struct Token CreatePunctuationToken(enum PunctuationType puncType, size_t start, size_t length, size_t line, size_t column) {
    struct Token token = {0};
    token.type = TOKEN_PUNCTUATION;
    token.value.punctuation = puncType;
    token.location.line = line;
    token.location.column = column;
    token.location.offset = start;
    token.location.length = length;
    return token;
}

static int IsPunctuation(struct Tokenizer* tokenizer) {
    for (size_t i = 0; i < PUNCTUATION_COUNT; i++) {
        const char* punc = PunctuationNames[i];
        size_t len = strlen(punc);
        if (strncmp(&tokenizer->file->content[tokenizer->position], punc, len) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static int IsOperator(struct Tokenizer* tokenizer) {
    for (size_t i = 0; i < OPERATOR_COUNT; i++) {
        const char* op = OperatorNames[i];
        size_t len = strlen(op);
        if (strncmp(&tokenizer->file->content[tokenizer->position], op, len) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static int IsKeywordAt(struct Tokenizer* tokenizer, size_t start, size_t length) {
    for (size_t i = 0; i < KEYWORD_COUNT; i++) {
        const char* kw = KeywordNames[i];
        size_t kwLen = strlen(kw);
        if (length == kwLen && strncmp(&tokenizer->file->content[start], kw, kwLen) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static void SkipWhitespaceAndComments(struct Tokenizer* tokenizer) {
    for (;;) {
        char c = PeekChar(tokenizer);
        while (c != '\0' && isspace((unsigned char)c)) {
            Advance(tokenizer, 1);
            c = PeekChar(tokenizer);
        }

        if (PeekChar(tokenizer) == '/' && PeekCharAt(tokenizer, 1) == '/') {
            while (PeekChar(tokenizer) != '\0' && PeekChar(tokenizer) != '\n') {
                Advance(tokenizer, 1);
            }
            continue;
        }

        if (PeekChar(tokenizer) == '/' && PeekCharAt(tokenizer, 1) == '*') {
            Advance(tokenizer, 2);
            while (PeekChar(tokenizer) != '\0') {
                if (PeekChar(tokenizer) == '*' && PeekCharAt(tokenizer, 1) == '/') {
                    Advance(tokenizer, 2);
                    break;
                }
                Advance(tokenizer, 1);
            }
            continue;
        }

        break;
    }
}

struct Token GetNextToken(struct Tokenizer* tokenizer) {
    struct Token token = {0};
    SkipWhitespaceAndComments(tokenizer);

    char c = PeekChar(tokenizer);
    if (c == '\0') {
        token.type = TOKEN_EOF;
        token.location.line = tokenizer->line;
        token.location.column = tokenizer->column;
        token.location.offset = tokenizer->position;
        return token;
    }

    size_t start = tokenizer->position;
    size_t line = tokenizer->line;
    size_t column = tokenizer->column;

    if (isalpha((unsigned char)c) || c == '_') {
        while (isalnum((unsigned char)PeekChar(tokenizer)) || PeekChar(tokenizer) == '_') {
            Advance(tokenizer, 1);
        }
        size_t length = tokenizer->position - start;
        int kwIndex = IsKeywordAt(tokenizer, start, length);
        if (kwIndex >= 0) {
            return CreateKeywordToken((enum KeywordType)kwIndex, start, length, line, column);
        }
        return CreateTextToken(TOKEN_IDENTIFIER, start, length, line, column);
    }

    if (isdigit((unsigned char)c)) {
        while (isalnum((unsigned char)PeekChar(tokenizer)) || PeekChar(tokenizer) == '.' || PeekChar(tokenizer) == '_') {
            Advance(tokenizer, 1);
        }
        return CreateTextToken(TOKEN_NUMBER, start, tokenizer->position - start, line, column);
    }

    if (c == '"') {
        Advance(tokenizer, 1);
        while (PeekChar(tokenizer) != '\0') {
            if (PeekChar(tokenizer) == '\\') {
                Advance(tokenizer, 2);
                continue;
            }
            if (PeekChar(tokenizer) == '"') {
                Advance(tokenizer, 1);
                break;
            }
            Advance(tokenizer, 1);
        }
        return CreateTextToken(TOKEN_STRING, start, tokenizer->position - start, line, column);
    }

    int opIndex = IsOperator(tokenizer);
    if (opIndex >= 0) {
        size_t length = strlen(OperatorNames[opIndex]);
        Advance(tokenizer, length);
        return CreateOperatorToken((enum OperatorType)opIndex, start, length, line, column);
    }

    int puncIndex = IsPunctuation(tokenizer);
    if (puncIndex >= 0) {
        size_t length = strlen(PunctuationNames[puncIndex]);
        Advance(tokenizer, length);
        return CreatePunctuationToken((enum PunctuationType)puncIndex, start, length, line, column);
    }

    fprintf(stderr, "Warning: Unrecognized character '%c' at line %zu, column %zu\n", c, line, column);
    Advance(tokenizer, 1);
    return CreateTextToken(TOKEN_IDENTIFIER, start, 1, line, column);
}

const char* TokenLexeme(struct Token* token, struct SourceFile* file) {
    switch (token->type) {
        case TOKEN_OPERATOR:
            return OperatorNames[token->value.operator];
        case TOKEN_KEYWORD:
            return KeywordNames[token->value.keyword];
        case TOKEN_PUNCTUATION:
            return PunctuationNames[token->value.punctuation];
        default:
            (void)file;
            return "";
    }
}

void PrintToken(struct Token* token, struct SourceFile* file) {
    printf("Token: %s at line %zu, column %zu\n", TokenTypeNames[token->type], token->location.line, token->location.column);
    printf("- Value: ");
    switch (token->type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_NUMBER:
        case TOKEN_STRING:
            if (token->location.offset + token->location.length <= file->length) {
                printf("%.*s\n", (int)token->location.length, file->content + token->location.offset);
            } else {
                printf("<invalid range>\n");
            }
            break;
        case TOKEN_OPERATOR:
        case TOKEN_KEYWORD:
        case TOKEN_PUNCTUATION:
            printf("%s\n", TokenLexeme(token, file));
            break;
        default:
            printf("None\n");
            break;
    }
}

static void PushToken(struct TokenArray* array, struct Token token) {
    if (array->count >= array->capacity) {
        fprintf(stderr, "Error: Token storage capacity exceeded.\n");
        exit(1);
    }
    array->items[array->count++] = token;
}

struct TokenArray TokenizeFile(struct Tokenizer* tokenizer, struct Token* storage, size_t capacity) {
    struct TokenArray array = {0};
    array.items = storage;
    array.capacity = capacity;

    for (;;) {
        struct Token token = GetNextToken(tokenizer);
        PushToken(&array, token);
        if (token.type == TOKEN_EOF) {
            break;
        }
    }
    return array;
}

void FreeTokenArray(struct TokenArray* array) {
    array->items = NULL;
    array->count = 0;
    array->capacity = 0;
}
