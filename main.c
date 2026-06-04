#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"

#define VERSION "0.1.0"

#define MAX_PATH_LENGTH 256

#define MAX_FILES 10

struct SourceFile {
    char path[MAX_PATH_LENGTH];
    char* content;
    size_t length;
};


struct SourceLocation {
    size_t line;
    size_t column;
    size_t offset;
    size_t length;
};

struct FileTable {
    struct SourceFile files[MAX_FILES];
    size_t count;
};

int ReadFile(const char* path, struct FileTable* table) {
    if (table->count >= MAX_FILES) {
        fprintf(stderr, "Error: File table is full.\n");
        return -1;
    }

    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", path);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = malloc(length + 1);
    if (!content) {
        fprintf(stderr, "Error: Could not allocate memory for file content.\n");
        fclose(file);
        return -1;
    }

    fread(content, 1, length, file);
    content[length] = '\0';
    fclose(file);

    struct SourceFile* sourceFile = &table->files[table->count++];
    strncpy(sourceFile->path, path, MAX_PATH_LENGTH);
    sourceFile->content = content;
    sourceFile->length = length;

    return table->count - 1;
}

enum TokenType {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_KEYWORD,
    TOKEN_PUNCTUATION,
};

const char* TokenTypeNames[] = {
    "EOF",
    "IDENTIFIER",
    "NUMBER",
    "STRING",
    "OPERATOR",
    "KEYWORD",
    "PUNCTUATION",
};

enum OperatorType {
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_ASSIGN,
    OPERATOR_EQUAL,
    OPERATOR_NOT_EQUAL,
    OPERATOR_LESS,
    OPERATOR_GREATER,
    OPERATOR_LESS_EQUAL,
    OPERATOR_GREATER_EQUAL,
    OPERATOR_LOGICAL_AND,
    OPERATOR_LOGICAL_OR,

    OPERATOR_COUNT
};

const char* OperatorNames[] = {
    "+", "-", "*", "/", "=", "==", "!=", "<", ">", "<=", ">=", "&&", "||",
};

enum KeywordType {
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_WHILE,
    KEYWORD_FOR,
    KEYWORD_RETURN,
    KEYWORD_DO,
    KEYWORD_BREAK,
    KEYWORD_CONTINUE,
    
    KEYWORD_COUNT
};

const char* KeywordNames[] = {
    "if", "else", "while", "for", "return", "do", "break", "continue"
};


enum PunctuationType {
    PUNCTUATION_LEFT_PAREN,
    PUNCTUATION_RIGHT_PAREN,
    PUNCTUATION_LEFT_BRACE,
    PUNCTUATION_RIGHT_BRACE,
    PUNCTUATION_LEFT_BRACKET,
    PUNCTUATION_RIGHT_BRACKET,
    PUNCTUATION_SEMICOLON,
    PUNCTUATION_COMMA,
    PUNCTUATION_COLON,

    PUNCTIUATION_COUNT
};

const char* PunctuationNames[] = {
    "(", ")", "{", "}", "[", "]", ";", ",", ":"
};


union TokenValue {
    struct {
        size_t offset;
        size_t length;
    } text;
    enum OperatorType operator;
    enum KeywordType keyword;
    enum PunctuationType punctuation;
};

struct Token {
    enum TokenType type;
    union TokenValue value;
    struct SourceLocation location;
};

void PrintToken(struct Token* token, struct SourceFile* file) {
    printf("Token: %s at line %zu, column %zu\n", TokenTypeNames[token->type], token->location.line, token->location.column);
    printf("- Value: ");
    switch (token->type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_NUMBER:
        case TOKEN_STRING: {
            size_t off = token->value.text.offset;
            size_t len = token->value.text.length;
            if (file && off + len <= file->length) {
                printf("%.*s\n", (int)len, file->content + off);
            } else {
                printf("<invalid range>\n");
            }
            break;
        }
        case TOKEN_OPERATOR:
            printf("%s\n", OperatorNames[token->value.operator]);
            break;
        case TOKEN_KEYWORD:
            printf("%s\n", KeywordNames[token->value.keyword]);
            break;
        case TOKEN_PUNCTUATION:
            printf("%s\n", PunctuationNames[token->value.punctuation]);
            break;
        default:
            printf("None\n");
            break;
    }

    return;
}

struct Tokenizer {
    struct SourceFile* file;
    size_t position;
    size_t line;
    size_t column;
};

struct Tokenizer CreateTokenizer(int fileIndex, struct FileTable* table) {
    struct Tokenizer tokenizer = {0};
    tokenizer.file = &table->files[fileIndex];
    tokenizer.position = 0;
    tokenizer.line = 1;
    tokenizer.column = 1;
    return tokenizer;
}

struct Token CreateIdentifierToken(struct Tokenizer* tokenizer, size_t start, size_t length) {
    struct Token token = {0};
    token.type = TOKEN_IDENTIFIER;
    token.value.text.offset = start;
    token.value.text.length = length;
    token.location.line = tokenizer->line;
    token.location.column = tokenizer->column - length;
    token.location.offset = start;
    token.location.length = length;
    return token;
}

struct Token CreateNumberToken(struct Tokenizer* tokenizer, size_t start, size_t length) {
    struct Token token = {0};
    token.type = TOKEN_NUMBER;
    token.value.text.offset = start;
    token.value.text.length = length;
    token.location.line = tokenizer->line;
    token.location.column = tokenizer->column - length;
    token.location.offset = start;
    token.location.length = length;
    return token;
}

struct Token CreateStringToken(struct Tokenizer* tokenizer, size_t start, size_t length) {
    struct Token token = {0};
    token.type = TOKEN_STRING;
    token.value.text.offset = start;
    token.value.text.length = length;
    token.location.line = tokenizer->line;
    token.location.column = tokenizer->column - length;
    token.location.offset = start;
    token.location.length = length;
    return token;
}

struct Token CreateOperatorToken(struct Tokenizer* tokenizer, enum OperatorType opType) {
    struct Token token = {0};
    token.type = TOKEN_OPERATOR;
    token.value.operator = opType;
    token.location.line = tokenizer->line;
    token.location.column = tokenizer->column;
    return token;
}

struct Token CreateKeywordToken(struct Tokenizer* tokenizer, enum KeywordType kwType) {
    struct Token token = {0};
    token.type = TOKEN_KEYWORD;
    token.value.keyword = kwType;
    token.location.line = tokenizer->line;
    token.location.column = tokenizer->column;
    return token;
}

struct Token CreatePunctuationToken(struct Tokenizer* tokenizer, enum PunctuationType puncType) {
    struct Token token = {0};
    token.type = TOKEN_PUNCTUATION;
    token.value.punctuation = puncType;
    token.location.line = tokenizer->line;
    token.location.column = tokenizer->column;
    return token;
}


char PeekChar(struct Tokenizer* tokenizer) {
    if (tokenizer->position >= tokenizer->file->length) {
        return '\0';
    }
    return tokenizer->file->content[tokenizer->position];
}

void Advance(struct Tokenizer* tokenizer, size_t count) {
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

int IsPunctuation(struct Tokenizer* tokenizer) {
    for (size_t i = 0; i < PUNCTIUATION_COUNT; i++) {
        const char* punc = PunctuationNames[i];
        size_t len = strlen(punc);
        if (strncmp(&tokenizer->file->content[tokenizer->position], punc, len) == 0) {
            return i;
        }
    }
    return -1;
}

int IsOperator(struct Tokenizer* tokenizer) {
    for (size_t i = 0; i < OPERATOR_COUNT; i++) {
        const char* op = OperatorNames[i];
        size_t len = strlen(op);
        if (strncmp(&tokenizer->file->content[tokenizer->position], op, len) == 0) {
            return i;
        }
    }
    return -1;
}

int IsKeyword(struct Tokenizer* tokenizer) {
    for (size_t i = 0; i < KEYWORD_COUNT; i++) {
        const char* kw = KeywordNames[i];
        size_t len = strlen(kw);
        if (strncmp(&tokenizer->file->content[tokenizer->position], kw, len) == 0) {
            return i;
        }
    }
    return -1;
}

struct Token GetNextToken(struct Tokenizer* tokenizer) {
    struct Token token = {0};

    char c = PeekChar(tokenizer);
    if (c == '\0') {
        token.type = TOKEN_EOF;
        token.location.line = tokenizer->line;
        token.location.column = tokenizer->column;
        return token;
    }

    while (isblank(c) || iscntrl(c)) {
        Advance(tokenizer, 1);
        c = PeekChar(tokenizer);
    }

    int puncIndex = IsPunctuation(tokenizer);
    if (puncIndex >= 0) {
        Advance(tokenizer, strlen(PunctuationNames[puncIndex]));
        return CreatePunctuationToken(tokenizer, puncIndex);
    }

    int opIndex = IsOperator(tokenizer);
    if (opIndex >= 0) {
        Advance(tokenizer, strlen(OperatorNames[opIndex]));
        return CreateOperatorToken(tokenizer, opIndex);
    }

    int kwIndex = IsKeyword(tokenizer);
    if (kwIndex >= 0) {
        Advance(tokenizer, strlen(KeywordNames[kwIndex]));
        return CreateKeywordToken(tokenizer, kwIndex);
    }

    if (isalpha(c) || c == '_') {
        size_t start = tokenizer->position;
        while (isalnum(PeekChar(tokenizer)) || PeekChar(tokenizer) == '_') {
            Advance(tokenizer, 1);
        }
        size_t length = tokenizer->position - start;
        return CreateIdentifierToken(tokenizer, start, length);
    }
    
    if (isdigit(c)) {
        size_t start = tokenizer->position;
        while (isdigit(PeekChar(tokenizer)) || PeekChar(tokenizer) == '.' || PeekChar(tokenizer) == '_' || PeekChar(tokenizer) == 'e' || PeekChar(tokenizer) == 'E' || PeekChar(tokenizer) == '+' || PeekChar(tokenizer) == '-') {
            Advance(tokenizer, 1);
        }
        size_t length = tokenizer->position - start;
        return CreateNumberToken(tokenizer, start, length);
    }

    printf("Warning: Unrecognized character '%c' at line %zu, column %zu\n", c, tokenizer->line, tokenizer->column);
    Advance(tokenizer, 1);

    return token;
}

int main(int argc, char** argv) {
    printf("%s - %s\n", "kek", VERSION);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source.kek>\n", argv[0]);
        return 1;
    }

    struct FileTable fileTable = {0};

    int fileIndex = ReadFile(argv[1], &fileTable);
    if (fileIndex < 0) {
        return 1;
    }

    struct Tokenizer tokenizer = CreateTokenizer(fileIndex, &fileTable);

    struct Token token;
    do {
        token = GetNextToken(&tokenizer);
        PrintToken(&token, tokenizer.file);
    } while (token.type != TOKEN_EOF);

    if (tokenizer.position < tokenizer.file->length) {
        fprintf(stderr, "Warning: Unprocessed content at end of file.\n");
    }

    return 0;
}