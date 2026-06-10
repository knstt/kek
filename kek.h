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
    int fileIndex;
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

enum KekDiagnosticSeverity {
    KEK_DIAGNOSTIC_NOTE,
    KEK_DIAGNOSTIC_WARNING,
    KEK_DIAGNOSTIC_ERROR,
};

enum KekDiagnosticPhase {
    KEK_PHASE_SOURCE,
    KEK_PHASE_LEX,
    KEK_PHASE_PARSE,
    KEK_PHASE_TYPED_PARSE,
    KEK_PHASE_SEMANTIC,
    KEK_PHASE_CODEGEN,
};

struct KekDiagnostic {
    enum KekDiagnosticSeverity severity;
    enum KekDiagnosticPhase phase;
    int fileIndex;
    struct SourceLocation location;
    char message[256];
};

struct KekDiagnosticBag {
    struct KekDiagnostic* items;
    size_t count;
    size_t capacity;
    size_t errorCount;
};

enum TokenType {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_COMMENT,
    TOKEN_DOC_COMMENT,
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
    KEYWORD_EACH,
    KEYWORD_PACKED,
    KEYWORD_ALIGNED,
    KEYWORD_COMPTIME,
    KEYWORD_DEFER,
    KEYWORD_TAGGED,
    KEYWORD_TRUE,
    KEYWORD_FALSE,
    KEYWORD_UNREACHABLE,
    KEYWORD_PANIC,

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
    PUNCTUATION_HASH,

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
    int fileIndex;
    size_t position;
    size_t line;
    size_t column;
    int emitComments;
    struct KekDiagnosticBag* diagnostics;
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
    AST_GENERIC,
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
    struct KekDiagnosticBag* diagnostics;
};

enum KekDeclKind {
    KEK_DECL_IMPORT,
    KEK_DECL_USING,
    KEK_DECL_ALIAS,
    KEK_DECL_EXTERN_C,
    KEK_DECL_STRUCT,
    KEK_DECL_ENUM,
    KEK_DECL_UNION,
    KEK_DECL_FUNCTION,
    KEK_DECL_VARIABLE,
    KEK_DECL_UNKNOWN,

    KEK_DECL_COUNT
};

enum KekStmtKind {
    KEK_STMT_BLOCK,
    KEK_STMT_DECL,
    KEK_STMT_EXPR,
    KEK_STMT_IF,
    KEK_STMT_ELSE,
    KEK_STMT_WHILE,
    KEK_STMT_DO_WHILE,
    KEK_STMT_FOR,
    KEK_STMT_EACH,
    KEK_STMT_SWITCH,
    KEK_STMT_CASE,
    KEK_STMT_DEFAULT,
    KEK_STMT_RETURN,
    KEK_STMT_BREAK,
    KEK_STMT_CONTINUE,
    KEK_STMT_UNREACHABLE,
    KEK_STMT_PANIC,
    KEK_STMT_UNKNOWN,

    KEK_STMT_COUNT
};

enum KekExprKind {
    KEK_EXPR_NAME,
    KEK_EXPR_NUMBER,
    KEK_EXPR_STRING,
    KEK_EXPR_BOOL,
    KEK_EXPR_CALL,
    KEK_EXPR_FIELD,
    KEK_EXPR_SCOPE,
    KEK_EXPR_INDEX,
    KEK_EXPR_GROUP,
    KEK_EXPR_STRUCT_LITERAL,
    KEK_EXPR_UNARY,
    KEK_EXPR_BINARY,
    KEK_EXPR_ASSIGN,
    KEK_EXPR_CAST,
    KEK_EXPR_SIZEOF,
    KEK_EXPR_ALIGNOF,
    KEK_EXPR_OFFSETOF,
    KEK_EXPR_LEN,
    KEK_EXPR_RANGE,
    KEK_EXPR_UNKNOWN,

    KEK_EXPR_COUNT
};

enum KekTypeKind {
    KEK_TYPE_BUILTIN,
    KEK_TYPE_NAME,
    KEK_TYPE_POINTER,
    KEK_TYPE_ARRAY,
    KEK_TYPE_FUNCTION,
    KEK_TYPE_UNKNOWN,

    KEK_TYPE_COUNT
};

enum KekSymbolKind {
    KEK_SYMBOL_TYPE,
    KEK_SYMBOL_FUNCTION,
    KEK_SYMBOL_GLOBAL,
    KEK_SYMBOL_PARAM,
    KEK_SYMBOL_LOCAL,
    KEK_SYMBOL_IMPORT,
    KEK_SYMBOL_UNKNOWN,

    KEK_SYMBOL_COUNT
};

enum KekScopeKind {
    KEK_SCOPE_PROGRAM,
    KEK_SCOPE_MODULE,
    KEK_SCOPE_FUNCTION,
    KEK_SCOPE_BLOCK,
    KEK_SCOPE_LOOP,

    KEK_SCOPE_COUNT
};

extern const char* KekDeclKindNames[];
extern const char* KekStmtKindNames[];
extern const char* KekExprKindNames[];
extern const char* KekTypeKindNames[];
extern const char* KekSymbolKindNames[];
extern const char* KekScopeKindNames[];

struct KekType;
struct KekExpr;
struct KekStmt;
struct KekParam;
struct KekField;
struct KekVariant;

struct KekType {
    enum KekTypeKind kind;
    struct SourceLocation location;
    struct AstNode* source;
    struct AstNode* name;
    struct AstNode* genericArgs;
    struct KekType* element;
    struct KekExpr* arraySize;
    struct KekType* next;
};

struct KekExpr {
    enum KekExprKind kind;
    struct SourceLocation location;
    struct AstNode* source;
    struct AstNode* token;
    struct KekExpr* left;
    struct KekExpr* right;
    struct KekExpr* callee;
    struct KekType* type;
    struct AstNode* genericArgs;
    struct KekExpr* firstArg;
    struct KekExpr* lastArg;
    struct KekExpr* next;
    struct KekExpr* step;         // Step expression (for range)
};

struct KekStmt {
    enum KekStmtKind kind;
    struct SourceLocation location;
    struct AstNode* source;
    struct KekType* declType;
    struct AstNode* declName;
    struct KekExpr* expr;
    struct KekExpr* condition;
    struct KekExpr* step;
    struct KekStmt* initStmt;
    struct KekStmt* firstChild;
    struct KekStmt* lastChild;
    struct KekStmt* next;
    struct KekType* indexType;    // Type of index variable (for each)
    struct AstNode* indexName;    // Name of index variable (for each)
};

struct KekParam {
    struct SourceLocation location;
    struct AstNode* source;
    struct KekType* type;
    struct AstNode* name;
    struct KekExpr* defaultValue;
    struct KekParam* next;
};

struct KekField {
    struct SourceLocation location;
    struct AstNode* source;
    struct KekType* type;
    struct AstNode* name;
    struct KekExpr* defaultValue;
    struct KekField* next;
    struct KekField* nestedFields;  // For nested struct definitions
    struct KekField* lastNestedField;
    int isNestedStruct;
};

struct KekVariant {
    struct SourceLocation location;
    struct AstNode* source;
    struct AstNode* name;
    struct KekExpr* value;
    struct KekVariant* next;
};

struct KekDecl {
    enum KekDeclKind kind;
    struct SourceLocation location;
    struct AstNode* source;
    struct AstNode* name;
    struct AstNode* type;
    struct AstNode* body;
    struct AstNode* genericParams;
    struct KekType* parsedType;
    struct KekParam* firstParam;
    struct KekParam* lastParam;
    struct KekField* firstField;
    struct KekField* lastField;
    struct KekVariant* firstVariant;
    struct KekVariant* lastVariant;
    struct KekStmt* firstStmt;
    struct KekStmt* lastStmt;
    struct KekDecl* next;
    int hasDocComment;
    struct SourceLocation docCommentLocation;
};

struct KekModule {
    struct SourceFile* file;
    struct KekDecl* firstDecl;
    struct KekDecl* lastDecl;
    size_t declCount;
    size_t declKindCounts[KEK_DECL_COUNT];
    size_t stmtKindCounts[KEK_STMT_COUNT];
    size_t exprKindCounts[KEK_EXPR_COUNT];
    size_t typeKindCounts[KEK_TYPE_COUNT];
    size_t paramCount;
    size_t fieldCount;
    size_t variantCount;
    size_t typedStmtCount;
    size_t typedExprCount;
    size_t typedTypeCount;
    int errorCount;
};

struct KekFrontend {
    struct SourceFile* file;
    struct KekDecl* decls;
    size_t declCount;
    size_t declCapacity;
    struct KekType* types;
    size_t typeCount;
    size_t typeCapacity;
    struct KekExpr* exprs;
    size_t exprCount;
    size_t exprCapacity;
    struct KekStmt* stmts;
    size_t stmtCount;
    size_t stmtCapacity;
    struct KekParam* params;
    size_t paramCount;
    size_t paramCapacity;
    struct KekField* fields;
    size_t fieldCount;
    size_t fieldCapacity;
    struct KekVariant* variants;
    size_t variantCount;
    size_t variantCapacity;
    int errorCount;
    struct KekDiagnosticBag* diagnostics;
};

struct KekSymbol {
    enum KekSymbolKind kind;
    struct SourceFile* file;
    struct KekDecl* decl;
    struct KekParam* param;
    struct KekStmt* stmt;
    struct AstNode* name;
    struct KekScope* scope;
    struct KekSymbol* nextInScope;
};

struct KekScope {
    enum KekScopeKind kind;
    struct SourceFile* file;
    struct KekDecl* decl;
    struct KekStmt* stmt;
    struct KekScope* parent;
    struct KekScope* next;
    struct KekSymbol* firstSymbol;
    struct KekSymbol* lastSymbol;
    size_t symbolCount;
    size_t symbolKindCounts[KEK_SYMBOL_COUNT];
};

struct KekProgram {
    struct KekSymbol* symbols;
    size_t symbolCount;
    size_t symbolCapacity;
    size_t symbolKindCounts[KEK_SYMBOL_COUNT];
    struct KekScope* scopes;
    size_t scopeCount;
    size_t scopeCapacity;
    size_t scopeKindCounts[KEK_SCOPE_COUNT];
    size_t semanticCheckCount;
    int errorCount;
    struct KekDiagnosticBag* diagnostics;
};

struct KekCompilationUnit {
    int fileIndex;
    struct Token* tokens;
    struct AstNode* astNodes;
    struct AstNode* ast;
    struct KekDecl* decls;
    struct KekType* types;
    struct KekExpr* exprs;
    struct KekStmt* stmts;
    struct KekParam* params;
    struct KekField* fields;
    struct KekVariant* variants;
    struct KekModule module;
};

struct KekCompilation {
    struct FileTable fileTable;
    struct KekCompilationUnit units[MAX_FILES];
    struct KekModule modules[MAX_FILES];
    size_t unitCount;
    int entryFileIndex;
    struct KekProgram program;
    struct KekSymbol* symbols;
    struct KekScope* scopes;
    struct KekDiagnosticBag diagnostics;
};

struct KekLexOptions {
    int emitComments;
    struct KekDiagnosticBag* diagnostics;
};

void InitKekDiagnosticBag(struct KekDiagnosticBag* bag, struct KekDiagnostic* storage, size_t capacity);
void KekAddDiagnostic(struct KekDiagnosticBag* bag, enum KekDiagnosticSeverity severity, enum KekDiagnosticPhase phase, int fileIndex, struct SourceLocation location, const char* message);
void KekAddDiagnosticFormat(struct KekDiagnosticBag* bag, enum KekDiagnosticSeverity severity, enum KekDiagnosticPhase phase, int fileIndex, struct SourceLocation location, const char* format, ...);
void PrintKekDiagnostics(FILE* out, struct KekDiagnosticBag* bag, struct FileTable* table);

int ReadFile(const char* path, struct FileTable* table);
int ReadFileWithDiagnostics(const char* path, struct FileTable* table, struct KekDiagnosticBag* diagnostics);
void FreeFileTable(struct FileTable* table);
const char* SourceLocationText(struct SourceFile* file, struct SourceLocation location, size_t* length);
const char* TokenText(struct Token* token, struct SourceFile* file, size_t* length);
const char* AstNodeText(struct AstNode* node, struct SourceFile* file, size_t* length);

struct Tokenizer CreateTokenizer(int fileIndex, struct FileTable* table);
struct Tokenizer CreateTokenizerWithOptions(int fileIndex, struct FileTable* table, struct KekLexOptions options);
struct Token GetNextToken(struct Tokenizer* tokenizer);
struct TokenArray TokenizeFile(struct Tokenizer* tokenizer, struct Token* storage, size_t capacity);
const char* TokenLexeme(struct Token* token, struct SourceFile* file);
void PrintToken(struct Token* token, struct SourceFile* file);
void FreeTokenArray(struct TokenArray* array);

struct AstNode* ParseAst(struct Parser* parser);
void PrintAst(struct AstNode* node, struct SourceFile* file, int indent);
void FreeAst(struct AstNode* node);

struct KekModule ParseKekModule(struct KekFrontend* frontend, struct AstNode* ast, struct SourceFile* file);
void PrintKekModuleSummary(struct KekModule* module);
int WriteKekModuleSummaryFile(const char* path, struct KekModule* modules, size_t moduleCount, struct KekProgram* program);
int BuildKekProgramSymbols(struct KekProgram* program, struct KekModule* modules, size_t moduleCount);

int WriteAstJsonFile(const char* path, struct AstNode* ast, struct SourceFile* file);
void WriteAstJson(FILE* out, struct AstNode* node, struct SourceFile* file, int indent);
void WriteJsonEscaped(FILE* out, const char* text, size_t length);

int WriteCFile(const char* path, struct AstNode* ast, struct SourceFile* file);
int WriteCFileForFiles(const char* path, struct AstNode** asts, struct SourceFile** files, size_t count);
int WriteTypedCFileForModules(const char* path, struct KekModule* modules, size_t count);
void WriteC(FILE* out, struct AstNode* ast, struct SourceFile* file);

void FreeKekCompilationUnit(struct KekCompilationUnit* unit);
void InitKekCompilation(struct KekCompilation* compilation, struct KekDiagnostic* diagnostics, size_t diagnosticCapacity);
void FreeKekCompilation(struct KekCompilation* compilation);
int LoadKekCompilation(struct KekCompilation* compilation, const char* entryPath);
int BuildKekCompilation(struct KekCompilation* compilation);
int WriteKekCompilationOutputs(struct KekCompilation* compilation, const char* cPath, const char* astJsonPath, const char* summaryPath);
int CompileKekSmoke(const char* entryPath, const char* cPath, const char* astJsonPath, const char* summaryPath, struct KekCompilation* compilation);
void AttachKekDocComments(struct KekModule* module, struct TokenArray* tokens, struct SourceFile* file);

#endif
