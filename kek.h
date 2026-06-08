#ifndef KEK_H
#define KEK_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION "0.2.0"

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

enum TokenType {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_KEYWORD,
    TOKEN_PUNCTUATION,
};

extern const char* TokenTypeNames[];

enum OperatorType {
    OPERATOR_SCOPE,
    OPERATOR_EQUAL,
    OPERATOR_NOT_EQUAL,
    OPERATOR_LESS_EQUAL,
    OPERATOR_GREATER_EQUAL,
    OPERATOR_LOGICAL_AND,
    OPERATOR_LOGICAL_OR,
    OPERATOR_PLUS_ASSIGN,
    OPERATOR_MINUS_ASSIGN,
    OPERATOR_ARROW,
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_MODULO,
    OPERATOR_ASSIGN,
    OPERATOR_LESS,
    OPERATOR_GREATER,
    OPERATOR_LOGICAL_NOT,
    OPERATOR_BITWISE_AND,
    OPERATOR_BITWISE_OR,
    OPERATOR_BITWISE_NOT,

    OPERATOR_COUNT
};

extern const char* OperatorNames[];

enum KeywordType {
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_WHILE,
    KEYWORD_FOR,
    KEYWORD_RETURN,
    KEYWORD_DO,
    KEYWORD_BREAK,
    KEYWORD_CONTINUE,
    KEYWORD_USING,
    KEYWORD_ALIAS,
    KEYWORD_EXPORT,
    KEYWORD_EXTERN,
    KEYWORD_ENUM,
    KEYWORD_STRUCT,
    KEYWORD_UNION,
    KEYWORD_SWITCH,
    KEYWORD_CASE,
    KEYWORD_DEFAULT,
    KEYWORD_IN,
    KEYWORD_PACKED,
    KEYWORD_ALIGNED,
    KEYWORD_COMPTIME,
    KEYWORD_DEFER,
    KEYWORD_TAGGED,
    KEYWORD_TRUE,
    KEYWORD_FALSE,

    KEYWORD_COUNT
};

extern const char* KeywordNames[];

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
    PUNCTUATION_DOT,

    PUNCTUATION_COUNT
};

extern const char* PunctuationNames[];

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

struct Tokenizer {
    struct SourceFile* file;
    size_t position;
    size_t line;
    size_t column;
};

struct TokenArray {
    struct Token* items;
    size_t count;
    size_t capacity;
};

enum AstNodeType {
    AST_FILE,
    AST_STATEMENT,
    AST_BLOCK,
    AST_GROUP,
    AST_INDEX,
    AST_TOKEN,
};

extern const char* AstNodeTypeNames[];

struct AstNode {
    enum AstNodeType type;
    struct SourceLocation location;
    struct Token token;
    struct AstNode* firstChild;
    struct AstNode* lastChild;
    struct AstNode* nextSibling;
    size_t childCount;
};

struct Parser {
    struct Token* tokens;
    size_t count;
    size_t position;
    struct SourceFile* file;
    struct AstNode* astNodes;
    size_t astNodeCount;
    size_t astNodeCapacity;
    int errorCount;
};

int ReadFile(const char* path, struct FileTable* table);
void FreeFileTable(struct FileTable* table);

struct Tokenizer CreateTokenizer(int fileIndex, struct FileTable* table);
struct Token GetNextToken(struct Tokenizer* tokenizer);
struct TokenArray TokenizeFile(struct Tokenizer* tokenizer, struct Token* storage, size_t capacity);
const char* TokenLexeme(struct Token* token, struct SourceFile* file);
void PrintToken(struct Token* token, struct SourceFile* file);
void FreeTokenArray(struct TokenArray* array);

struct AstNode* ParseAst(struct Parser* parser);
void PrintAst(struct AstNode* node, struct SourceFile* file, int indent);
void FreeAst(struct AstNode* node);

int WriteAstJsonFile(const char* path, struct AstNode* ast, struct SourceFile* file);
void WriteAstJson(FILE* out, struct AstNode* node, struct SourceFile* file, int indent);
void WriteJsonEscaped(FILE* out, const char* text, size_t length);

int WriteCFile(const char* path, struct AstNode* ast, struct SourceFile* file);
void WriteC(FILE* out, struct AstNode* ast, struct SourceFile* file);

#endif
