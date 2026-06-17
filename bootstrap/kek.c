/* Generated bootstrap compiler artifact. Source of truth: self/kek.kek during self-hosted milestones. */
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>



#define VERSION "0.2.0"

#define MAX_PATH_LENGTH 256
#define MAX_FILES 32

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
    TOKEN_CHAR,
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
    KEK_STMT_DEFER,
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

void PrintAst(struct AstNode* node, struct SourceFile* file, int indent);
void FreeAst(struct AstNode* node);

void PrintKekModuleSummary(struct KekModule* module);
int WriteKekModuleSummaryFile(const char* path, struct KekModule* modules, size_t moduleCount, struct KekProgram* program);

int WriteAstJsonFile(const char* path, struct AstNode* ast, struct SourceFile* file);
void WriteAstJson(FILE* out, struct AstNode* node, struct SourceFile* file, int indent);
void WriteJsonEscaped(FILE* out, const char* text, size_t length);

int WriteTypedCFileForModules(const char* path, struct KekModule* modules, size_t count);

void FreeKekCompilationUnit(struct KekCompilationUnit* unit);
void InitKekCompilation(struct KekCompilation* compilation, struct KekDiagnostic* diagnostics, size_t diagnosticCapacity);
void FreeKekCompilation(struct KekCompilation* compilation);
int LoadKekCompilation(struct KekCompilation* compilation, const char* entryPath);
int BuildKekCompilation(struct KekCompilation* compilation);
int WriteKekCompilationOutputs(struct KekCompilation* compilation, const char* cPath, const char* astJsonPath, const char* summaryPath);
int CompileKekSmoke(const char* entryPath, const char* cPath, const char* astJsonPath, const char* summaryPath, struct KekCompilation* compilation);




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

struct KekNodePool {
    void* items;
    size_t count;
    size_t capacity;
    size_t itemSize;
};

struct KekFrontend {
    struct SourceFile* file;
    struct KekNodePool decls;
    struct KekNodePool types;
    struct KekNodePool exprs;
    struct KekNodePool stmts;
    struct KekNodePool params;
    struct KekNodePool fields;
    struct KekNodePool variants;
    int errorCount;
    struct KekDiagnosticBag* diagnostics;
};

struct AstNode* ParseAst(struct Parser* parser);
struct KekModule ParseKekModule(struct KekFrontend* frontend, struct AstNode* ast, struct SourceFile* file);
int BuildKekProgramSymbols(struct KekProgram* program, struct KekModule* modules, size_t moduleCount);
void AttachKekDocComments(struct KekModule* module, struct TokenArray* tokens, struct SourceFile* file);



/* BEGIN ast.c */
#define CreateAstNode ast_CreateAstNode
#define AddChild ast_AddChild
#define IsPunctuationToken ast_IsPunctuationToken
#define IsOperatorToken ast_IsOperatorToken
#define TokenTextEquals ast_TokenTextEquals
#define IsClosingPunctuation ast_IsClosingPunctuation
#define IsTriviaToken ast_IsTriviaToken
#define PunctuationName ast_PunctuationName
#define ReportParseError ast_ReportParseError
#define IsAstTerminator ast_IsAstTerminator
#define IsGenericTerminator ast_IsGenericTerminator
#define FinishLocationFromChildren ast_FinishLocationFromChildren
#define ParseStatement ast_ParseStatement
#define ShouldParseGenericList ast_ShouldParseGenericList
#define ParseTokenNode ast_ParseTokenNode
#define ParseChildrenInto ast_ParseChildrenInto
#define ParseDelimited ast_ParseDelimited
#define ParseGenericDelimited ast_ParseGenericDelimited
#define ParseList ast_ParseList
#define PrintIndent ast_PrintIndent

const char* AstNodeTypeNames[] = {
    "File",
    "Statement",
    "Block",
    "Group",
    "Index",
    "Generic",
    "Token",
};

static struct AstNode* CreateAstNode(struct Parser* parser, enum AstNodeType type, struct SourceLocation location) {
    if (parser->astNodeCount >= parser->astNodeCapacity) {
        KekAddDiagnostic(parser->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_PARSE,
            parser->file ? parser->file->fileIndex : -1, location, "AST node storage capacity exceeded");
        parser->errorCount++;
        if (parser->astNodeCapacity > 0) {
            return &parser->astNodes[parser->astNodeCapacity - 1];
        }
        return NULL;
    }

    struct AstNode* node = &parser->astNodes[parser->astNodeCount++];
    memset(node, 0, sizeof(*node));
    node->type = type;
    node->location = location;
    return node;
}

static void AddChild(struct AstNode* parent, struct AstNode* child) {
    child->nextSibling = NULL;
    if (parent->lastChild) {
        parent->lastChild->nextSibling = child;
    } else {
        parent->firstChild = child;
    }
    parent->lastChild = child;
    parent->childCount++;
}

static int IsPunctuationToken(struct Token* token, enum PunctuationType punctuation) {
    return token->type == TOKEN_PUNCTUATION && token->value.punctuation == punctuation;
}

static int IsOperatorToken(struct Token* token, enum OperatorType operator) {
    return token->type == TOKEN_OPERATOR && token->value.operator == operator;
}

static int TokenTextEquals(struct Parser* parser, struct Token* token, const char* text) {
    size_t length = strlen(text);
    return (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_NUMBER || token->type == TOKEN_STRING)
        && token->location.length == length
        && strncmp(parser->file->content + token->location.offset, text, length) == 0;
}

static int IsClosingPunctuation(struct Token* token) {
    return IsPunctuationToken(token, PUNCTUATION_RIGHT_PAREN)
        || IsPunctuationToken(token, PUNCTUATION_RIGHT_BRACE)
        || IsPunctuationToken(token, PUNCTUATION_RIGHT_BRACKET);
}

static int IsTriviaToken(struct Token* token) {
    return token->type == TOKEN_COMMENT || token->type == TOKEN_DOC_COMMENT;
}

static const char* PunctuationName(enum PunctuationType punctuation) {
    return punctuation < PUNCTUATION_COUNT ? PunctuationNames[punctuation] : "<end of file>";
}

static void ReportParseError(struct Parser* parser, struct Token* token, const char* message) {
    KekAddDiagnostic(parser->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_PARSE,
        parser->file ? parser->file->fileIndex : -1, token->location, message);
    parser->errorCount++;
}

static int IsAstTerminator(struct Token* token, enum PunctuationType closePunctuation) {
    if (token->type == TOKEN_EOF) {
        return 1;
    }
    if (closePunctuation < PUNCTUATION_COUNT && IsPunctuationToken(token, closePunctuation)) {
        return 1;
    }
    if (IsClosingPunctuation(token)) {
        return 1;
    }
    return 0;
}

static int IsGenericTerminator(struct Token* token) {
    return token->type == TOKEN_EOF || IsOperatorToken(token, OPERATOR_GREATER);
}

static void FinishLocationFromChildren(struct AstNode* node) {
    if (node->childCount == 0) {
        return;
    }

    struct SourceLocation first = node->firstChild->location;
    struct SourceLocation last = node->lastChild->location;
    node->location = first;
    if (last.offset + last.length >= first.offset) {
        node->location.length = (last.offset + last.length) - first.offset;
    }
}

static struct AstNode* ParseStatement(struct Parser* parser, enum PunctuationType closePunctuation);
static int ShouldParseGenericList(struct Parser* parser, struct AstNode* previousChild);

static struct AstNode* ParseTokenNode(struct Parser* parser) {
    struct Token token = parser->tokens[parser->position++];
    struct AstNode* node = CreateAstNode(parser, AST_TOKEN, token.location);
    node->token = token;
    return node;
}

static void ParseChildrenInto(struct Parser* parser, struct AstNode* parent, enum PunctuationType closePunctuation) {
    while (parser->position < parser->count && !IsAstTerminator(&parser->tokens[parser->position], closePunctuation)) {
        if (IsTriviaToken(&parser->tokens[parser->position])) {
            parser->position++;
            continue;
        }
        size_t previousPosition = parser->position;
        struct AstNode* statement = ParseStatement(parser, closePunctuation);
        if (statement->childCount > 0) {
            AddChild(parent, statement);
        } else if (parser->position == previousPosition) {
            break;
        } else {
            continue;
        }
    }
}

static struct AstNode* ParseDelimited(struct Parser* parser, enum AstNodeType type, enum PunctuationType closePunctuation) {
    struct Token open = parser->tokens[parser->position++];
    struct AstNode* node = CreateAstNode(parser, type, open.location);

    ParseChildrenInto(parser, node, closePunctuation);

    if (parser->position < parser->count && IsPunctuationToken(&parser->tokens[parser->position], closePunctuation)) {
        struct Token close = parser->tokens[parser->position++];
        node->location = open.location;
        node->location.length = (close.location.offset + close.location.length) - open.location.offset;
    } else if (parser->position < parser->count) {
        char message[128];
        snprintf(message, sizeof(message), "expected '%s'", PunctuationName(closePunctuation));
        ReportParseError(parser, &parser->tokens[parser->position], message);
        if (IsClosingPunctuation(&parser->tokens[parser->position])) {
            parser->position++;
        }
    } else {
        ReportParseError(parser, &open, "unterminated delimiter");
    }

    if (node->childCount > 0 && node->location.length == open.location.length) {
        FinishLocationFromChildren(node);
        node->location.offset = open.location.offset;
        node->location.line = open.location.line;
        node->location.column = open.location.column;
    }
    return node;
}

static struct AstNode* ParseGenericDelimited(struct Parser* parser) {
    struct Token open = parser->tokens[parser->position++];
    struct AstNode* node = CreateAstNode(parser, AST_GENERIC, open.location);

    while (parser->position < parser->count && !IsGenericTerminator(&parser->tokens[parser->position])) {
        if (IsTriviaToken(&parser->tokens[parser->position])) {
            parser->position++;
            continue;
        }
        struct AstNode* statement = CreateAstNode(parser, AST_STATEMENT, parser->tokens[parser->position].location);
        while (parser->position < parser->count && !IsGenericTerminator(&parser->tokens[parser->position])) {
            struct Token* token = &parser->tokens[parser->position];
            if (IsTriviaToken(token)) {
                parser->position++;
                continue;
            }
            if (IsPunctuationToken(token, PUNCTUATION_COMMA)) {
                parser->position++;
                break;
            }
            if (IsPunctuationToken(token, PUNCTUATION_LEFT_PAREN)) {
                AddChild(statement, ParseDelimited(parser, AST_GROUP, PUNCTUATION_RIGHT_PAREN));
                continue;
            }
            if (IsPunctuationToken(token, PUNCTUATION_LEFT_BRACKET)) {
                AddChild(statement, ParseDelimited(parser, AST_INDEX, PUNCTUATION_RIGHT_BRACKET));
                continue;
            }
            if (ShouldParseGenericList(parser, statement->lastChild)) {
                AddChild(statement, ParseGenericDelimited(parser));
                continue;
            }
            AddChild(statement, ParseTokenNode(parser));
        }
        FinishLocationFromChildren(statement);
        if (statement->childCount > 0) {
            AddChild(node, statement);
        }
    }

    if (parser->position < parser->count && IsOperatorToken(&parser->tokens[parser->position], OPERATOR_GREATER)) {
        struct Token close = parser->tokens[parser->position++];
        node->location = open.location;
        node->location.length = (close.location.offset + close.location.length) - open.location.offset;
    } else if (parser->position < parser->count) {
        ReportParseError(parser, &parser->tokens[parser->position], "expected '>'");
    } else {
        ReportParseError(parser, &open, "unterminated generic list");
    }

    if (node->childCount > 0 && node->location.length == open.location.length) {
        FinishLocationFromChildren(node);
        node->location.offset = open.location.offset;
        node->location.line = open.location.line;
        node->location.column = open.location.column;
    }
    return node;
}

static int ShouldParseGenericList(struct Parser* parser, struct AstNode* previousChild) {
    if (!previousChild || previousChild->type != AST_TOKEN) {
        return 0;
    }
    // Allow identifiers and keywords to have generic arguments
    if (previousChild->token.type != TOKEN_IDENTIFIER && previousChild->token.type != TOKEN_KEYWORD) {
        return 0;
    }
    if (TokenTextEquals(parser, &previousChild->token, "cast")) {
        return 0;
    }
    if (parser->position >= parser->count || !IsOperatorToken(&parser->tokens[parser->position], OPERATOR_LESS)) {
        return 0;
    }

    int depth = 0;
    for (size_t i = parser->position; i < parser->count; i++) {
        struct Token* token = &parser->tokens[i];
        if (IsOperatorToken(token, OPERATOR_LESS)) {
            depth++;
            continue;
        }
        if (IsOperatorToken(token, OPERATOR_GREATER)) {
            depth--;
            if (depth == 0) {
                return 1;
            }
            continue;
        }
        if (depth == 1
            && token->type == TOKEN_PUNCTUATION
            && token->value.punctuation == PUNCTUATION_DOT) {
            return 0;
        }
        if (token->type == TOKEN_EOF
            || IsPunctuationToken(token, PUNCTUATION_SEMICOLON)
            || IsPunctuationToken(token, PUNCTUATION_RIGHT_PAREN)
            || IsPunctuationToken(token, PUNCTUATION_LEFT_BRACE)
            || IsPunctuationToken(token, PUNCTUATION_RIGHT_BRACE)) {
            return 0;
        }
    }
    return 0;
}

static struct AstNode* ParseStatement(struct Parser* parser, enum PunctuationType closePunctuation) {
    struct AstNode* statement = CreateAstNode(parser, AST_STATEMENT, parser->tokens[parser->position].location);

    while (parser->position < parser->count && !IsAstTerminator(&parser->tokens[parser->position], closePunctuation)) {
        struct Token* token = &parser->tokens[parser->position];

        if (IsTriviaToken(token)) {
            parser->position++;
            continue;
        }

        if (IsPunctuationToken(token, PUNCTUATION_SEMICOLON) || IsPunctuationToken(token, PUNCTUATION_COMMA)) {
            parser->position++;
            break;
        }

        if (IsClosingPunctuation(token)) {
            break;
        }

        if (IsPunctuationToken(token, PUNCTUATION_LEFT_BRACE)) {
            AddChild(statement, ParseDelimited(parser, AST_BLOCK, PUNCTUATION_RIGHT_BRACE));
            break;
        }

        if (IsPunctuationToken(token, PUNCTUATION_LEFT_PAREN)) {
            AddChild(statement, ParseDelimited(parser, AST_GROUP, PUNCTUATION_RIGHT_PAREN));
            continue;
        }

        if (IsPunctuationToken(token, PUNCTUATION_LEFT_BRACKET)) {
            AddChild(statement, ParseDelimited(parser, AST_INDEX, PUNCTUATION_RIGHT_BRACKET));
            continue;
        }

        if (ShouldParseGenericList(parser, statement->lastChild)) {
            AddChild(statement, ParseGenericDelimited(parser));
            continue;
        }

        AddChild(statement, ParseTokenNode(parser));
    }

    FinishLocationFromChildren(statement);
    return statement;
}

static struct AstNode* ParseList(struct Parser* parser, enum AstNodeType listType, enum PunctuationType closePunctuation) {
    struct AstNode* list = CreateAstNode(parser, listType, parser->tokens[parser->position].location);

    ParseChildrenInto(parser, list, closePunctuation);

    if (closePunctuation == PUNCTUATION_COUNT
        && parser->position < parser->count
        && IsClosingPunctuation(&parser->tokens[parser->position])) {
        ReportParseError(parser, &parser->tokens[parser->position], "unexpected closing delimiter");
        parser->position++;
    }

    FinishLocationFromChildren(list);
    return list;
}

struct AstNode* ParseAst(struct Parser* parser) {
    struct SourceLocation location = {1, 1, 0, parser->file->length};
    struct AstNode* root = ParseList(parser, AST_FILE, PUNCTUATION_COUNT);
    root->location = location;
    return root;
}

static void PrintIndent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void PrintAst(struct AstNode* node, struct SourceFile* file, int indent) {
    PrintIndent(indent);
    printf("%s", AstNodeTypeNames[node->type]);
    if (node->type == AST_TOKEN) {
        printf(" %s", TokenTypeNames[node->token.type]);
        if (node->token.type == TOKEN_IDENTIFIER || node->token.type == TOKEN_NUMBER || node->token.type == TOKEN_STRING || node->token.type == TOKEN_CHAR) {
            printf(" '%.*s'", (int)node->token.location.length, file->content + node->token.location.offset);
        } else if (node->token.type != TOKEN_EOF) {
            printf(" '%s'", TokenLexeme(&node->token, file));
        }
    }
    printf(" @ %zu:%zu len=%zu\n", node->location.line, node->location.column, node->location.length);

    for (struct AstNode* child = node->firstChild; child; child = child->nextSibling) {
        PrintAst(child, file, indent + 1);
    }
}

void FreeAst(struct AstNode* node) {
    (void)node;
}
#undef PrintIndent
#undef ParseList
#undef ParseGenericDelimited
#undef ParseDelimited
#undef ParseChildrenInto
#undef ParseTokenNode
#undef ShouldParseGenericList
#undef ParseStatement
#undef FinishLocationFromChildren
#undef IsGenericTerminator
#undef IsAstTerminator
#undef ReportParseError
#undef PunctuationName
#undef IsTriviaToken
#undef IsClosingPunctuation
#undef TokenTextEquals
#undef IsOperatorToken
#undef IsPunctuationToken
#undef AddChild
#undef CreateAstNode
/* END ast.c */

/* BEGIN parser.c */
#define IsTokenNode parser_IsTokenNode
#define IsKeywordNode parser_IsKeywordNode
#define IsPunctuationNode parser_IsPunctuationNode
#define IsGenericNode parser_IsGenericNode
#define IsOperatorNode parser_IsOperatorNode
#define IsAnyOverloadableOperatorNode parser_IsAnyOverloadableOperatorNode
#define TokenTextEquals parser_TokenTextEquals
#define Next parser_Next
#define AddFrontendNode parser_AddFrontendNode
#define AddType parser_AddType
#define AddExpr parser_AddExpr
#define AddStmt parser_AddStmt
#define AddParam parser_AddParam
#define AddField parser_AddField
#define AddVariant parser_AddVariant
#define AddDecl parser_AddDecl
#define ParseExprUntil parser_ParseExprUntil
#define ParseType parser_ParseType
#define AddTypedParseDiagnostic parser_AddTypedParseDiagnostic
#define ParsePointerElementType parser_ParsePointerElementType
#define AddExprArg parser_AddExprArg
#define AddStructLiteralFields parser_AddStructLiteralFields
#define ParseStructLiteralExpr parser_ParseStructLiteralExpr
#define ParseGroupExpr parser_ParseGroupExpr
#define IsExpressionEnd parser_IsExpressionEnd
#define IsAssignmentExprOperator parser_IsAssignmentExprOperator
#define ExprOperatorPrecedence parser_ExprOperatorPrecedence
#define IsRightAssociativeExprOperator parser_IsRightAssociativeExprOperator
#define IsUnaryExprOperator parser_IsUnaryExprOperator
#define ParseExprPrecedence parser_ParseExprPrecedence
#define AddCallArgs parser_AddCallArgs
#define ParsePrimaryExpr parser_ParsePrimaryExpr
#define ParsePostfixExpr parser_ParsePostfixExpr
#define IsBuiltinTypeName parser_IsBuiltinTypeName
#define AddChildStmt parser_AddChildStmt
#define LooksLikeDecl parser_LooksLikeDecl
#define DeclColonAfterType parser_DeclColonAfterType
#define DeclNameAfterType parser_DeclNameAfterType
#define AfterNameAndArraySuffixes parser_AfterNameAndArraySuffixes
#define ParseFirstGroupStatementExpr parser_ParseFirstGroupStatementExpr
#define ClassifyStatementKind parser_ClassifyStatementKind
#define ParseStatementList parser_ParseStatementList
#define ParseStatement parser_ParseStatement
#define IsDoWhileConditionStatement parser_IsDoWhileConditionStatement
#define ClassifyColonDecl parser_ClassifyColonDecl
#define ClassifyKeywordDecl parser_ClassifyKeywordDecl
#define ClassifyDecl parser_ClassifyDecl
#define AddDeclStmt parser_AddDeclStmt
#define AddDeclParam parser_AddDeclParam
#define AddDeclField parser_AddDeclField
#define AddDeclVariant parser_AddDeclVariant
#define BuildParamList parser_BuildParamList
#define IsNestedStructDecl parser_IsNestedStructDecl
#define BuildNestedStructFields parser_BuildNestedStructFields
#define BuildStructFields parser_BuildStructFields
#define BuildEnumVariants parser_BuildEnumVariants
#define BuildDeclDetails parser_BuildDeclDetails
#define WriteKekModuleSummary parser_WriteKekModuleSummary

const char* KekDeclKindNames[] = {
    "Import",
    "Using",
    "Alias",
    "ExternC",
    "Struct",
    "Enum",
    "Union",
    "Function",
    "Variable",
    "Unknown",
};

const char* KekStmtKindNames[] = {
    "Block",
    "Decl",
    "Expr",
    "If",
    "Else",
    "While",
    "DoWhile",
    "For",
    "Each",
    "Switch",
    "Case",
    "Default",
    "Defer",
    "Return",
    "Break",
    "Continue",
    "Unreachable",
    "Panic",
    "Unknown",
};

const char* KekExprKindNames[] = {
    "Name",
    "Number",
    "String",
    "Bool",
    "Call",
    "Field",
    "Scope",
    "Index",
    "Group",
    "StructLiteral",
    "Unary",
    "Binary",
    "Assign",
    "Cast",
    "Sizeof",
    "Alignof",
    "Offsetof",
    "Len",
    "Range",
    "Unknown",
};

const char* KekTypeKindNames[] = {
    "Builtin",
    "Name",
    "Pointer",
    "Array",
    "Function",
    "Unknown",
};

static int IsTokenNode(struct AstNode* node) {
    return node && node->type == AST_TOKEN;
}

static int IsKeywordNode(struct AstNode* node, enum KeywordType keyword) {
    return IsTokenNode(node)
        && node->token.type == TOKEN_KEYWORD
        && node->token.value.keyword == keyword;
}

static int IsPunctuationNode(struct AstNode* node, enum PunctuationType punctuation) {
    return IsTokenNode(node)
        && node->token.type == TOKEN_PUNCTUATION
        && node->token.value.punctuation == punctuation;
}

static int IsGenericNode(struct AstNode* node) {
    return node && node->type == AST_GENERIC;
}

static int IsOperatorNode(struct AstNode* node, enum OperatorType operator) {
    return IsTokenNode(node)
        && node->token.type == TOKEN_OPERATOR
        && node->token.value.operator == operator;
}

static int IsAnyOverloadableOperatorNode(struct AstNode* node) {
    return IsTokenNode(node)
        && node->token.type == TOKEN_OPERATOR
        && node->token.value.operator != OPERATOR_SCOPE
        && node->token.value.operator != OPERATOR_ASSIGN;
}

static int TokenTextEquals(struct AstNode* node, struct SourceFile* file, const char* text) {
    size_t length = strlen(text);
    return IsTokenNode(node)
        && (node->token.type == TOKEN_IDENTIFIER || node->token.type == TOKEN_NUMBER || node->token.type == TOKEN_STRING || node->token.type == TOKEN_CHAR)
        && node->token.location.length == length
        && strncmp(file->content + node->token.location.offset, text, length) == 0;
}

static struct AstNode* Next(struct AstNode* node) {
    return node ? node->nextSibling : NULL;
}

static void* AddFrontendNode(struct KekFrontend* frontend, struct KekModule* module, struct KekNodePool* pool, struct AstNode* source, const char* message) {
    if (pool->count >= pool->capacity) {
        KekAddDiagnostic(frontend->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_TYPED_PARSE,
            module->file ? module->file->fileIndex : -1,
            source ? source->location : (struct SourceLocation){0},
            message);
        frontend->errorCount++;
        module->errorCount++;
        return NULL;
    }

    char* items = pool->items;
    void* node = items + pool->itemSize * pool->count++;
    memset(node, 0, pool->itemSize);
    return node;
}

static struct KekType* AddType(struct KekFrontend* frontend, struct KekModule* module, enum KekTypeKind kind, struct AstNode* source) {
    struct KekType* type = AddFrontendNode(frontend, module, &frontend->types, source, "typed type storage capacity exceeded");
    if (!type) {
        return NULL;
    }
    type->kind = kind;
    type->source = source;
    type->location = source ? source->location : (struct SourceLocation){0};
    if (kind < KEK_TYPE_COUNT) {
        module->typeKindCounts[kind]++;
    }
    module->typedTypeCount++;
    return type;
}

static struct KekExpr* AddExpr(struct KekFrontend* frontend, struct KekModule* module, enum KekExprKind kind, struct AstNode* source) {
    struct KekExpr* expr = AddFrontendNode(frontend, module, &frontend->exprs, source, "typed expression storage capacity exceeded");
    if (!expr) {
        return NULL;
    }
    expr->kind = kind;
    expr->source = source;
    expr->location = source ? source->location : (struct SourceLocation){0};
    if (IsTokenNode(source)) {
        expr->token = source;
    }
    if (kind < KEK_EXPR_COUNT) {
        module->exprKindCounts[kind]++;
    }
    module->typedExprCount++;
    return expr;
}

static struct KekStmt* AddStmt(struct KekFrontend* frontend, struct KekModule* module, enum KekStmtKind kind, struct AstNode* source) {
    struct KekStmt* stmt = AddFrontendNode(frontend, module, &frontend->stmts, source, "typed statement storage capacity exceeded");
    if (!stmt) {
        return NULL;
    }
    stmt->kind = kind;
    stmt->source = source;
    stmt->location = source ? source->location : (struct SourceLocation){0};
    if (kind < KEK_STMT_COUNT) {
        module->stmtKindCounts[kind]++;
    }
    module->typedStmtCount++;
    return stmt;
}

static struct KekParam* AddParam(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* source) {
    struct KekParam* param = AddFrontendNode(frontend, module, &frontend->params, source, "typed parameter storage capacity exceeded");
    if (!param) {
        return NULL;
    }
    param->source = source;
    param->location = source ? source->location : (struct SourceLocation){0};
    module->paramCount++;
    return param;
}

static struct KekField* AddField(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* source) {
    struct KekField* field = AddFrontendNode(frontend, module, &frontend->fields, source, "typed field storage capacity exceeded");
    if (!field) {
        return NULL;
    }
    field->source = source;
    field->location = source ? source->location : (struct SourceLocation){0};
    module->fieldCount++;
    return field;
}

static struct KekVariant* AddVariant(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* source) {
    struct KekVariant* variant = AddFrontendNode(frontend, module, &frontend->variants, source, "typed variant storage capacity exceeded");
    if (!variant) {
        return NULL;
    }
    variant->source = source;
    variant->location = source ? source->location : (struct SourceLocation){0};
    module->variantCount++;
    return variant;
}

static struct KekDecl* AddDecl(struct KekFrontend* frontend, struct KekModule* module, enum KekDeclKind kind, struct AstNode* source) {
    struct KekDecl* decl = AddFrontendNode(frontend, module, &frontend->decls, source, "typed declaration storage capacity exceeded");
    if (!decl) {
        return NULL;
    }
    decl->kind = kind;
    decl->source = source;
    decl->location = source ? source->location : (struct SourceLocation){0};

    if (module->lastDecl) {
        module->lastDecl->next = decl;
    } else {
        module->firstDecl = decl;
    }
    module->lastDecl = decl;
    module->declCount++;
    if (kind < KEK_DECL_COUNT) {
        module->declKindCounts[kind]++;
    }
    return decl;
}

static struct KekExpr* ParseExprUntil(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* first, struct AstNode* stop);
static struct KekType* ParseType(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* typeNode, struct AstNode* afterName);

static void AddTypedParseDiagnostic(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* source, const char* message) {
    KekAddDiagnostic(frontend->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_TYPED_PARSE,
        module->file ? module->file->fileIndex : -1,
        source ? source->location : (struct SourceLocation){0},
        message);
    frontend->errorCount++;
    module->errorCount++;
}

static struct KekType* ParsePointerElementType(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* genericArgs) {
    if (!genericArgs) {
        return NULL;
    }
    if (!genericArgs->firstChild || genericArgs->childCount != 1) {
        AddTypedParseDiagnostic(frontend, module, genericArgs, "ptr<T> requires exactly one type argument");
        return AddType(frontend, module, KEK_TYPE_UNKNOWN, genericArgs);
    }

    struct AstNode* arg = genericArgs->firstChild;
    if (!arg || arg->type != AST_STATEMENT || !arg->firstChild) {
        AddTypedParseDiagnostic(frontend, module, genericArgs, "ptr<T> requires a type argument");
        return AddType(frontend, module, KEK_TYPE_UNKNOWN, genericArgs);
    }

    return ParseType(frontend, module, arg->firstChild, NULL);
}

static void AddExprArg(struct KekExpr* call, struct KekExpr* arg) {
    if (!call || !arg) {
        return;
    }
    if (call->lastArg) {
        call->lastArg->next = arg;
    } else {
        call->firstArg = arg;
    }
    call->lastArg = arg;
}

static void AddStructLiteralFields(struct KekFrontend* frontend, struct KekModule* module, struct KekExpr* literal, struct AstNode* block) {
    if (!literal || !block || block->type != AST_BLOCK) {
        return;
    }

    for (struct AstNode* field = block->firstChild; field; field = field->nextSibling) {
        if (field->firstChild) {
            AddExprArg(literal, ParseExprUntil(frontend, module, field->firstChild, NULL));
        }
    }
}

static struct KekExpr* ParseStructLiteralExpr(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* typeNode, struct AstNode* block) {
    struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_STRUCT_LITERAL, block ? block : typeNode);
    if (expr) {
        if (typeNode) {
            expr->type = ParseType(frontend, module, typeNode, NULL);
        }
        AddStructLiteralFields(frontend, module, expr, block);
    }
    return expr;
}

static struct KekExpr* ParseGroupExpr(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* group) {
    if (!group || (group->type != AST_GROUP && group->type != AST_INDEX) || !group->firstChild) {
        return AddExpr(frontend, module, KEK_EXPR_UNKNOWN, group);
    }
    struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_GROUP, group);
    if (expr) {
        expr->right = ParseExprUntil(frontend, module, group->firstChild->firstChild, NULL);
    }
    return expr;
}

static int IsExpressionEnd(struct AstNode* node, struct AstNode* stop) {
    return !node || node == stop;
}

static int IsAssignmentExprOperator(struct AstNode* node) {
    return IsOperatorNode(node, OPERATOR_ASSIGN)
        || IsOperatorNode(node, OPERATOR_PLUS_ASSIGN)
        || IsOperatorNode(node, OPERATOR_MINUS_ASSIGN);
}

static int ExprOperatorPrecedence(struct AstNode* node) {
    if (IsAssignmentExprOperator(node)) {
        return 1;
    }
    if (IsOperatorNode(node, OPERATOR_LOGICAL_OR)) {
        return 2;
    }
    if (IsOperatorNode(node, OPERATOR_LOGICAL_AND)) {
        return 3;
    }
    if (IsOperatorNode(node, OPERATOR_BITWISE_OR)) {
        return 4;
    }
    if (IsOperatorNode(node, OPERATOR_BITWISE_AND)) {
        return 5;
    }
    if (IsOperatorNode(node, OPERATOR_EQUAL) || IsOperatorNode(node, OPERATOR_NOT_EQUAL)) {
        return 6;
    }
    if (IsOperatorNode(node, OPERATOR_LESS)
        || IsOperatorNode(node, OPERATOR_LESS_EQUAL)
        || IsOperatorNode(node, OPERATOR_GREATER)
        || IsOperatorNode(node, OPERATOR_GREATER_EQUAL)) {
        return 7;
    }
    if (IsOperatorNode(node, OPERATOR_PLUS) || IsOperatorNode(node, OPERATOR_MINUS)) {
        return 8;
    }
    if (IsOperatorNode(node, OPERATOR_MULTIPLY)
        || IsOperatorNode(node, OPERATOR_DIVIDE)
        || IsOperatorNode(node, OPERATOR_MODULO)) {
        return 9;
    }
    return 0;
}

static int IsRightAssociativeExprOperator(struct AstNode* node) {
    return IsAssignmentExprOperator(node);
}

static int IsUnaryExprOperator(struct AstNode* node) {
    return IsOperatorNode(node, OPERATOR_LOGICAL_NOT)
        || IsOperatorNode(node, OPERATOR_BITWISE_NOT)
        || IsOperatorNode(node, OPERATOR_MINUS)
        || IsOperatorNode(node, OPERATOR_BITWISE_AND)
        || IsOperatorNode(node, OPERATOR_MULTIPLY);
}

static struct KekExpr* ParseExprPrecedence(struct KekFrontend* frontend, struct KekModule* module, struct AstNode** current, struct AstNode* stop, int minPrecedence);

static void AddCallArgs(struct KekFrontend* frontend, struct KekModule* module, struct KekExpr* call, struct AstNode* group) {
    if (!call || !group || group->type != AST_GROUP) {
        return;
    }

    for (struct AstNode* arg = group->firstChild; arg; arg = arg->nextSibling) {
        if (arg->firstChild) {
            AddExprArg(call, ParseExprUntil(frontend, module, arg->firstChild, NULL));
        }
    }
}

static struct KekExpr* ParsePrimaryExpr(struct KekFrontend* frontend, struct KekModule* module, struct AstNode** current, struct AstNode* stop) {
    struct AstNode* node = *current;
    if (IsExpressionEnd(node, stop)) {
        return AddExpr(frontend, module, KEK_EXPR_UNKNOWN, node);
    }

    if (node->type == AST_GROUP) {
        *current = Next(node);
        return ParseGroupExpr(frontend, module, node);
    }

    if (node->type == AST_BLOCK) {
        *current = Next(node);
        return ParseStructLiteralExpr(frontend, module, NULL, node);
    }

    if (IsTokenNode(node)
        && node->token.type == TOKEN_IDENTIFIER
        && IsPunctuationNode(Next(node), PUNCTUATION_COLON)
        && Next(Next(node))
        && Next(Next(node))->type == AST_BLOCK) {
        struct AstNode* block = Next(Next(node));
        *current = Next(block);
        return ParseStructLiteralExpr(frontend, module, node, block);
    }

    if (IsOperatorNode(node, OPERATOR_SCOPE)) {
        struct AstNode* name = Next(node);
        struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_SCOPE, node);
        if (expr) {
            expr->right = AddExpr(frontend, module, KEK_EXPR_NAME, name);
        }
        *current = name ? Next(name) : Next(node);
        return expr;
    }

    if (IsTokenNode(node) && TokenTextEquals(node, module->file, "cast")) {
        struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_CAST, node);
        struct AstNode* less = Next(node);
        struct AstNode* typeNode = Next(less);
        struct AstNode* greater = Next(typeNode);
        struct AstNode* valueGroup = Next(greater);
        if (expr
            && IsOperatorNode(less, OPERATOR_LESS)
            && typeNode
            && IsOperatorNode(greater, OPERATOR_GREATER)
            && valueGroup
            && valueGroup->type == AST_GROUP) {
            expr->type = ParseType(frontend, module, typeNode, NULL);
            expr->right = ParseGroupExpr(frontend, module, valueGroup);
            *current = Next(valueGroup);
        } else {
            *current = Next(node);
        }
        return expr;
    }

    // sizeof(Type) or sizeof(expr)
    if (IsTokenNode(node) && TokenTextEquals(node, module->file, "sizeof")) {
        struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_SIZEOF, node);
        struct AstNode* argGroup = Next(node);
        if (expr && argGroup && argGroup->type == AST_GROUP && argGroup->firstChild) {
            // AST_GROUP contains AST_STATEMENT, which contains the actual tokens
            struct AstNode* arg = argGroup->firstChild->firstChild;
            // Try to parse as type first, fallback to expression
            expr->type = ParseType(frontend, module, arg, NULL);
            if (!expr->type || expr->type->kind == KEK_TYPE_UNKNOWN) {
                expr->right = ParseGroupExpr(frontend, module, argGroup);
                expr->type = NULL;
            }
            *current = Next(argGroup);
        } else {
            *current = Next(node);
        }
        return expr;
    }

    // alignof(Type)
    if (IsTokenNode(node) && TokenTextEquals(node, module->file, "alignof")) {
        struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_ALIGNOF, node);
        struct AstNode* argGroup = Next(node);
        if (expr && argGroup && argGroup->type == AST_GROUP && argGroup->firstChild) {
            struct AstNode* arg = argGroup->firstChild->firstChild;
            expr->type = ParseType(frontend, module, arg, NULL);
            *current = Next(argGroup);
        } else {
            *current = Next(node);
        }
        return expr;
    }

    // offsetof(Type, field)
    if (IsTokenNode(node) && TokenTextEquals(node, module->file, "offsetof")) {
        struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_OFFSETOF, node);
        struct AstNode* argGroup = Next(node);
        if (expr && argGroup && argGroup->type == AST_GROUP && argGroup->firstChild) {
            struct AstNode* stmt = argGroup->firstChild;
            struct AstNode* arg = stmt->firstChild;
            // First arg is type
            expr->type = ParseType(frontend, module, arg, NULL);
            // Skip comma (which separates statements in the group), field is in next statement
            struct AstNode* fieldStmt = stmt->nextSibling;
            if (fieldStmt && fieldStmt->firstChild && IsTokenNode(fieldStmt->firstChild)) {
                expr->right = AddExpr(frontend, module, KEK_EXPR_NAME, fieldStmt->firstChild);
            }
            *current = Next(argGroup);
        } else {
            *current = Next(node);
        }
        return expr;
    }

    // len(array)
    if (IsTokenNode(node) && TokenTextEquals(node, module->file, "len") && Next(node) && Next(node)->type == AST_GROUP) {
        struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_LEN, node);
        struct AstNode* argGroup = Next(node);
        if (expr && argGroup && argGroup->type == AST_GROUP) {
            expr->right = ParseGroupExpr(frontend, module, argGroup);
            *current = Next(argGroup);
        } else {
            *current = Next(node);
        }
        return expr;
    }

    // range<Type>(start, end[, step])
    if (IsTokenNode(node) && TokenTextEquals(node, module->file, "range")) {
        struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_RANGE, node);
        struct AstNode* generic = Next(node);
        struct AstNode* argGroup = NULL;

        // Parse generic type argument <Type>
        if (expr && IsGenericNode(generic)) {
            struct AstNode* typeArg = generic->firstChild ? generic->firstChild->firstChild : NULL;
            if (typeArg) {
                expr->type = ParseType(frontend, module, typeArg, NULL);
            }
            argGroup = Next(generic);
        } else {
            argGroup = generic;
        }

        // Parse arguments (start, end[, step])
        if (expr && argGroup && argGroup->type == AST_GROUP) {
            struct AstNode* startStmt = argGroup->firstChild;
            struct AstNode* endStmt = startStmt ? startStmt->nextSibling : NULL;
            struct AstNode* stepStmt = endStmt ? endStmt->nextSibling : NULL;

            if (startStmt && startStmt->firstChild) {
                expr->left = ParseExprUntil(frontend, module, startStmt->firstChild, NULL);
            }
            if (endStmt && endStmt->firstChild) {
                expr->right = ParseExprUntil(frontend, module, endStmt->firstChild, NULL);
            }
            if (stepStmt && stepStmt->firstChild) {
                expr->step = ParseExprUntil(frontend, module, stepStmt->firstChild, NULL);
            }
            *current = Next(argGroup);
        } else {
            *current = Next(node);
        }
        return expr;
    }

    if (IsUnaryExprOperator(node)) {
        struct KekExpr* expr = AddExpr(frontend, module, KEK_EXPR_UNARY, node);
        *current = Next(node);
        if (expr) {
            expr->right = ParseExprPrecedence(frontend, module, current, stop, 10);
        }
        return expr;
    }

    enum KekExprKind kind = KEK_EXPR_UNKNOWN;
    if (IsTokenNode(node) && node->token.type == TOKEN_IDENTIFIER) {
        kind = KEK_EXPR_NAME;
    } else if (IsTokenNode(node) && (node->token.type == TOKEN_NUMBER || node->token.type == TOKEN_CHAR)) {
        kind = KEK_EXPR_NUMBER;
    } else if (IsTokenNode(node) && node->token.type == TOKEN_STRING) {
        kind = KEK_EXPR_STRING;
    } else if (IsKeywordNode(node, KEYWORD_TRUE) || IsKeywordNode(node, KEYWORD_FALSE)) {
        kind = KEK_EXPR_BOOL;
    }

    struct KekExpr* expr = AddExpr(frontend, module, kind, node);
    *current = Next(node);
    return expr;
}

static struct KekExpr* ParsePostfixExpr(struct KekFrontend* frontend, struct KekModule* module, struct AstNode** current, struct AstNode* stop) {
    struct KekExpr* expr = ParsePrimaryExpr(frontend, module, current, stop);

    while (!IsExpressionEnd(*current, stop)) {
        struct AstNode* node = *current;

        if (node->type == AST_GROUP) {
            struct KekExpr* call = AddExpr(frontend, module, KEK_EXPR_CALL, node);
            if (call) {
                call->callee = expr;
                AddCallArgs(frontend, module, call, node);
            }
            expr = call;
            *current = Next(node);
            continue;
        }

        if (node->type == AST_INDEX) {
            struct KekExpr* index = AddExpr(frontend, module, KEK_EXPR_INDEX, node);
            if (index) {
                index->left = expr;
                index->right = ParseGroupExpr(frontend, module, node);
            }
            expr = index;
            *current = Next(node);
            continue;
        }

        if (IsGenericNode(node)) {
            if (expr) {
                expr->genericArgs = node;
            }
            *current = Next(node);
            continue;
        }

        if (IsPunctuationNode(node, PUNCTUATION_DOT)) {
            struct AstNode* name = Next(node);
            struct KekExpr* field = AddExpr(frontend, module, KEK_EXPR_FIELD, node);
            if (field) {
                field->left = expr;
                field->right = AddExpr(frontend, module, KEK_EXPR_NAME, name);
            }
            expr = field;
            *current = name ? Next(name) : Next(node);
            continue;
        }

        if (IsOperatorNode(node, OPERATOR_SCOPE)) {
            struct AstNode* name = Next(node);
            struct KekExpr* scoped = AddExpr(frontend, module, KEK_EXPR_SCOPE, node);
            if (scoped) {
                scoped->left = expr;
                scoped->right = AddExpr(frontend, module, KEK_EXPR_NAME, name);
            }
            expr = scoped;
            *current = name ? Next(name) : Next(node);
            continue;
        }

        break;
    }

    return expr;
}

static struct KekExpr* ParseExprPrecedence(struct KekFrontend* frontend, struct KekModule* module, struct AstNode** current, struct AstNode* stop, int minPrecedence) {
    struct KekExpr* left = ParsePostfixExpr(frontend, module, current, stop);

    while (!IsExpressionEnd(*current, stop)) {
        struct AstNode* op = *current;
        int precedence = ExprOperatorPrecedence(op);
        if (precedence == 0 || precedence < minPrecedence) {
            break;
        }

        *current = Next(op);
        struct KekExpr* right = ParseExprPrecedence(frontend, module, current, stop,
            precedence + (IsRightAssociativeExprOperator(op) ? 0 : 1));
        struct KekExpr* combined = AddExpr(frontend, module,
            IsAssignmentExprOperator(op) ? KEK_EXPR_ASSIGN : KEK_EXPR_BINARY, op);
        if (combined) {
            combined->left = left;
            combined->right = right;
        }
        left = combined;
    }

    return left;
}

static struct KekExpr* ParseExprUntil(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* first, struct AstNode* stop) {
    struct AstNode* current = first;
    if (!current) {
        return AddExpr(frontend, module, KEK_EXPR_UNKNOWN, NULL);
    }

    return ParseExprPrecedence(frontend, module, &current, stop, 1);
}

static int IsBuiltinTypeName(struct AstNode* typeNode, struct SourceFile* file) {
    return TokenTextEquals(typeNode, file, "void")
        || TokenTextEquals(typeNode, file, "bool")
        || TokenTextEquals(typeNode, file, "char")
        || TokenTextEquals(typeNode, file, "int")
        || TokenTextEquals(typeNode, file, "u8")
        || TokenTextEquals(typeNode, file, "u16")
        || TokenTextEquals(typeNode, file, "u32")
        || TokenTextEquals(typeNode, file, "u64")
        || TokenTextEquals(typeNode, file, "i8")
        || TokenTextEquals(typeNode, file, "i16")
        || TokenTextEquals(typeNode, file, "i32")
        || TokenTextEquals(typeNode, file, "i64")
        || TokenTextEquals(typeNode, file, "f32")
        || TokenTextEquals(typeNode, file, "f64");
}

static struct KekType* ParseType(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* typeNode, struct AstNode* afterName) {
    if (!typeNode) {
        return AddType(frontend, module, KEK_TYPE_UNKNOWN, NULL);
    }

    enum KekTypeKind baseKind = KEK_TYPE_NAME;
    if (TokenTextEquals(typeNode, module->file, "ptr")) {
        baseKind = KEK_TYPE_POINTER;
    } else if (IsBuiltinTypeName(typeNode, module->file)) {
        baseKind = KEK_TYPE_BUILTIN;
    }

    struct KekType* base = AddType(frontend, module, baseKind, typeNode);
    if (!base) {
        return NULL;
    }
    base->name = typeNode;
    if (baseKind == KEK_TYPE_POINTER && IsGenericNode(Next(typeNode))) {
        base->element = ParsePointerElementType(frontend, module, Next(typeNode));
    } else if (IsGenericNode(Next(typeNode))) {
        base->genericArgs = Next(typeNode);
    }

    struct AstNode* arrayNode = NULL;
    if (afterName && afterName->type == AST_INDEX) {
        arrayNode = afterName;
    } else if (afterName && Next(afterName) && Next(afterName)->type == AST_INDEX) {
        arrayNode = Next(afterName);
    }

    while (arrayNode) {
        struct KekType* array = AddType(frontend, module, KEK_TYPE_ARRAY, arrayNode);
        if (array) {
            array->element = base;
            array->arraySize = ParseGroupExpr(frontend, module, arrayNode);
            base = array;
        }
        arrayNode = Next(arrayNode) && Next(arrayNode)->type == AST_INDEX ? Next(arrayNode) : NULL;
    }

    return base;
}

static void AddChildStmt(struct KekStmt* parent, struct KekStmt* child) {
    if (!parent || !child) {
        return;
    }
    if (parent->lastChild) {
        parent->lastChild->next = child;
    } else {
        parent->firstChild = child;
    }
    parent->lastChild = child;
}

static int LooksLikeDecl(struct AstNode* first) {
    struct AstNode* colon = Next(first);
    if (IsGenericNode(colon)) {
        colon = Next(colon);
    }
    return IsTokenNode(first)
        && colon
        && IsPunctuationNode(colon, PUNCTUATION_COLON)
        && Next(colon)
        && IsTokenNode(Next(colon));
}

static struct AstNode* DeclColonAfterType(struct AstNode* type) {
    struct AstNode* colon = Next(type);
    if (IsGenericNode(colon)) {
        colon = Next(colon);
    }
    return colon;
}

static struct AstNode* DeclNameAfterType(struct AstNode* type) {
    struct AstNode* colon = DeclColonAfterType(type);
    return colon ? Next(colon) : NULL;
}

static struct AstNode* AfterNameAndArraySuffixes(struct AstNode* name) {
    struct AstNode* node = Next(name);
    if (IsGenericNode(node)) {
        node = Next(node);
    }
    while (node && node->type == AST_INDEX) {
        node = Next(node);
    }
    return node;
}

static struct KekExpr* ParseFirstGroupStatementExpr(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* group) {
    if (!group || group->type != AST_GROUP || !group->firstChild) {
        return AddExpr(frontend, module, KEK_EXPR_UNKNOWN, group);
    }
    return ParseExprUntil(frontend, module, group->firstChild->firstChild, NULL);
}

static enum KekStmtKind ClassifyStatementKind(struct AstNode* first) {
    if (IsKeywordNode(first, KEYWORD_IF)) {
        return KEK_STMT_IF;
    }
    if (IsKeywordNode(first, KEYWORD_ELSE)) {
        return KEK_STMT_ELSE;
    }
    if (IsKeywordNode(first, KEYWORD_WHILE)) {
        return KEK_STMT_WHILE;
    }
    if (IsKeywordNode(first, KEYWORD_DO)) {
        return KEK_STMT_DO_WHILE;
    }
    if (IsKeywordNode(first, KEYWORD_FOR)) {
        return KEK_STMT_FOR;
    }
    if (IsKeywordNode(first, KEYWORD_EACH)) {
        return KEK_STMT_EACH;
    }
    if (IsKeywordNode(first, KEYWORD_SWITCH)) {
        return KEK_STMT_SWITCH;
    }
    if (IsKeywordNode(first, KEYWORD_CASE)) {
        return KEK_STMT_CASE;
    }
    if (IsKeywordNode(first, KEYWORD_DEFAULT)) {
        return KEK_STMT_DEFAULT;
    }
    if (IsKeywordNode(first, KEYWORD_DEFER)) {
        return KEK_STMT_DEFER;
    }
    if (IsKeywordNode(first, KEYWORD_RETURN)) {
        return KEK_STMT_RETURN;
    }
    if (IsKeywordNode(first, KEYWORD_BREAK)) {
        return KEK_STMT_BREAK;
    }
    if (IsKeywordNode(first, KEYWORD_CONTINUE)) {
        return KEK_STMT_CONTINUE;
    }
    if (IsKeywordNode(first, KEYWORD_UNREACHABLE)) {
        return KEK_STMT_UNREACHABLE;
    }
    if (IsKeywordNode(first, KEYWORD_PANIC)) {
        return KEK_STMT_PANIC;
    }
    if (LooksLikeDecl(first)) {
        return KEK_STMT_DECL;
    }
    return KEK_STMT_EXPR;
}

static struct KekStmt* ParseStatementList(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* block);

static struct KekStmt* ParseStatement(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* statement) {
    if (!statement || statement->type != AST_STATEMENT || !statement->firstChild) {
        return AddStmt(frontend, module, KEK_STMT_UNKNOWN, statement);
    }

    struct AstNode* first = statement->firstChild;
    enum KekStmtKind kind = ClassifyStatementKind(first);

    struct KekStmt* stmt = AddStmt(frontend, module, kind, statement);
    if (!stmt) {
        return NULL;
    }

    if (kind == KEK_STMT_DECL) {
        stmt->declName = DeclNameAfterType(first);
        stmt->declType = ParseType(frontend, module, first, stmt->declName);
        struct AstNode* afterDecl = AfterNameAndArraySuffixes(stmt->declName);
        if (afterDecl && IsOperatorNode(afterDecl, OPERATOR_ASSIGN)) {
            stmt->expr = ParseExprUntil(frontend, module, Next(afterDecl), NULL);
            if (stmt->expr && stmt->expr->kind == KEK_EXPR_STRUCT_LITERAL && !stmt->expr->type) {
                stmt->expr->type = stmt->declType;
            }
        }
    } else if (kind == KEK_STMT_FOR) {
        struct AstNode* group = Next(first);
        struct AstNode* afterGroup = group ? Next(group) : NULL;
        if (TokenTextEquals(afterGroup, module->file, "in")) {
            AddTypedParseDiagnostic(frontend, module, afterGroup, "for-in syntax is not supported; use each");
        }
        struct AstNode* init = group && group->type == AST_GROUP ? group->firstChild : NULL;
        struct AstNode* condition = init ? init->nextSibling : NULL;
        struct AstNode* step = condition ? condition->nextSibling : NULL;
        if (init && init->firstChild) {
            if (LooksLikeDecl(init->firstChild)) {
                stmt->initStmt = ParseStatement(frontend, module, init);
            } else {
                stmt->expr = ParseExprUntil(frontend, module, init->firstChild, NULL);
            }
        }
        if (condition && condition->firstChild) {
            stmt->condition = ParseExprUntil(frontend, module, condition->firstChild, NULL);
        }
        if (step && step->firstChild) {
            stmt->step = ParseExprUntil(frontend, module, step->firstChild, NULL);
        }
    } else if (kind == KEK_STMT_EACH) {
        // each<Type:varName>(iterable){ body }
        // each<IndexType:idx, Type:varName>(iterable){ body }
        struct AstNode* generic = Next(first);
        struct AstNode* iterableGroup = NULL;

        if (IsGenericNode(generic)) {
            // Parse variable declarations from generic args
            struct AstNode* firstDecl = generic->firstChild;
            struct AstNode* secondDecl = firstDecl ? firstDecl->nextSibling : NULL;

            if (secondDecl) {
                // Two declarations: index and value
                // Parse index variable (first declaration)
                struct AstNode* indexTypeNode = firstDecl->firstChild;
                if (indexTypeNode && LooksLikeDecl(indexTypeNode)) {
                    stmt->indexName = DeclNameAfterType(indexTypeNode);
                    stmt->indexType = ParseType(frontend, module, indexTypeNode, stmt->indexName);
                }
                // Parse value variable (second declaration)
                struct AstNode* valueTypeNode = secondDecl->firstChild;
                if (valueTypeNode && LooksLikeDecl(valueTypeNode)) {
                    stmt->declName = DeclNameAfterType(valueTypeNode);
                    stmt->declType = ParseType(frontend, module, valueTypeNode, stmt->declName);
                }
            } else if (firstDecl) {
                // Single declaration: value only
                struct AstNode* valueTypeNode = firstDecl->firstChild;
                if (valueTypeNode && LooksLikeDecl(valueTypeNode)) {
                    stmt->declName = DeclNameAfterType(valueTypeNode);
                    stmt->declType = ParseType(frontend, module, valueTypeNode, stmt->declName);
                }
            }

            iterableGroup = Next(generic);
        } else {
            iterableGroup = generic;
        }

        // Parse iterable expression from group
        if (iterableGroup && iterableGroup->type == AST_GROUP) {
            stmt->expr = ParseFirstGroupStatementExpr(frontend, module, iterableGroup);
        }
    } else if (kind == KEK_STMT_IF || kind == KEK_STMT_WHILE || kind == KEK_STMT_SWITCH || kind == KEK_STMT_CASE) {
        stmt->condition = ParseFirstGroupStatementExpr(frontend, module, Next(first));
        stmt->expr = stmt->condition;
    } else if (kind == KEK_STMT_RETURN) {
        struct AstNode* value = Next(first);
        if (value) {
            stmt->expr = ParseExprUntil(frontend, module, value, NULL);
        }
    } else if (kind == KEK_STMT_DEFAULT) {
        struct AstNode* colon = Next(first);
        if (IsPunctuationNode(colon, PUNCTUATION_COLON) && Next(colon)) {
            struct AstNode* afterColon = Next(colon);
            // If it's a block, don't parse as expression - it will be handled as child statements
            if (afterColon->type != AST_BLOCK) {
                stmt->expr = ParseExprUntil(frontend, module, afterColon, NULL);
            }
        }
    } else if (kind == KEK_STMT_PANIC) {
        // panic("message") - parse the argument from the group
        struct AstNode* argGroup = Next(first);
        if (argGroup && argGroup->type == AST_GROUP) {
            stmt->expr = ParseGroupExpr(frontend, module, argGroup);
        }
    } else if (kind == KEK_STMT_DEFER) {
        struct AstNode* deferred = Next(first);
        if (deferred && deferred->type != AST_BLOCK) {
            stmt->expr = ParseExprUntil(frontend, module, deferred, NULL);
        }
    } else if (kind == KEK_STMT_EXPR) {
        stmt->expr = ParseExprUntil(frontend, module, first, NULL);
    }

    for (struct AstNode* child = statement->firstChild; child; child = child->nextSibling) {
        if (child->type == AST_BLOCK) {
            if (kind == KEK_STMT_DECL && stmt->expr && stmt->expr->source == child) {
                continue;
            }
            AddChildStmt(stmt, ParseStatementList(frontend, module, child));
        }
    }

    return stmt;
}

static int IsDoWhileConditionStatement(struct AstNode* statement) {
    return statement
        && statement->type == AST_STATEMENT
        && IsKeywordNode(statement->firstChild, KEYWORD_WHILE)
        && Next(statement->firstChild)
        && Next(statement->firstChild)->type == AST_GROUP
        && !Next(Next(statement->firstChild));
}

static struct KekStmt* ParseStatementList(struct KekFrontend* frontend, struct KekModule* module, struct AstNode* block) {
    if (!block || block->type != AST_BLOCK) {
        return NULL;
    }

    struct KekStmt* blockStmt = AddStmt(frontend, module, KEK_STMT_BLOCK, block);
    for (struct AstNode* statement = block->firstChild; statement; statement = statement->nextSibling) {
        struct KekStmt* parsed = ParseStatement(frontend, module, statement);
        if (parsed && parsed->kind == KEK_STMT_DO_WHILE && IsDoWhileConditionStatement(statement->nextSibling)) {
            parsed->condition = ParseFirstGroupStatementExpr(frontend, module, Next(statement->nextSibling->firstChild));
            parsed->expr = parsed->condition;
            statement = statement->nextSibling;
        }
        AddChildStmt(blockStmt, parsed);
    }
    return blockStmt;
}

static enum KekDeclKind ClassifyColonDecl(struct AstNode* first, struct SourceFile* file, struct KekDecl* decl) {
    struct AstNode* colon = Next(first);
    if (IsGenericNode(colon)) {
        colon = Next(colon);
    }
    struct AstNode* name = Next(colon);
    if (!IsPunctuationNode(colon, PUNCTUATION_COLON) || !IsTokenNode(name) || name->token.type != TOKEN_IDENTIFIER) {
        return KEK_DECL_UNKNOWN;
    }

    decl->type = first;
    decl->name = name;

    struct AstNode* afterName = Next(name);
    if (IsTokenNode(name)
        && name->token.type == TOKEN_IDENTIFIER
        && afterName
        && IsAnyOverloadableOperatorNode(afterName)
        && TokenTextEquals(name, file, "operator")) {
        decl->name = afterName;
        afterName = Next(afterName);
    }
    if (IsGenericNode(afterName)) {
        decl->genericParams = afterName;
        afterName = Next(afterName);
    }
    if (afterName && afterName->type == AST_GROUP) {
        decl->body = Next(afterName);
        return KEK_DECL_FUNCTION;
    }

    decl->body = afterName;
    return KEK_DECL_VARIABLE;
}

static enum KekDeclKind ClassifyKeywordDecl(struct AstNode* first, struct SourceFile* file, struct KekDecl* decl) {
    if (IsKeywordNode(first, KEYWORD_USING)) {
        decl->name = Next(first);
        return KEK_DECL_USING;
    }

    if (IsKeywordNode(first, KEYWORD_ALIAS)) {
        struct AstNode* colon = Next(first);
        struct AstNode* name = Next(colon);
        struct AstNode* equals = Next(name);
        if (IsPunctuationNode(colon, PUNCTUATION_COLON)
            && IsTokenNode(name)
            && IsOperatorNode(equals, OPERATOR_ASSIGN)) {
            decl->name = name;
            decl->type = Next(equals);
        }
        return KEK_DECL_ALIAS;
    }

    if (IsKeywordNode(first, KEYWORD_EXTERN)
        && TokenTextEquals(Next(first), file, "\"C\"")
        && Next(Next(first))
        && Next(Next(first))->type == AST_BLOCK) {
        decl->body = Next(Next(first));
        return KEK_DECL_EXTERN_C;
    }

    if (IsKeywordNode(first, KEYWORD_STRUCT)
        || IsKeywordNode(first, KEYWORD_UNION)) {
        struct AstNode* colon = Next(first);
        struct AstNode* name = Next(colon);
        if (IsPunctuationNode(colon, PUNCTUATION_COLON) && IsTokenNode(name)) {
            decl->name = name;
            decl->genericParams = IsGenericNode(Next(name)) ? Next(name) : NULL;
            decl->body = decl->genericParams ? Next(decl->genericParams) : Next(name);
        }
        return IsKeywordNode(first, KEYWORD_STRUCT) ? KEK_DECL_STRUCT : KEK_DECL_UNION;
    }

    if (IsKeywordNode(first, KEYWORD_ENUM)) {
        struct AstNode* firstColon = Next(first);
        struct AstNode* underlyingType = Next(firstColon);
        struct AstNode* secondColon = Next(underlyingType);
        struct AstNode* name = Next(secondColon);
        if (IsPunctuationNode(firstColon, PUNCTUATION_COLON)
            && IsPunctuationNode(secondColon, PUNCTUATION_COLON)
            && IsTokenNode(name)) {
            decl->type = underlyingType;
            decl->name = name;
            decl->body = Next(name);
        }
        return KEK_DECL_ENUM;
    }

    (void)file;
    return KEK_DECL_UNKNOWN;
}

static enum KekDeclKind ClassifyDecl(struct KekDecl* decl, struct SourceFile* file) {
    struct AstNode* first = decl->source ? decl->source->firstChild : NULL;
    if (!first) {
        return KEK_DECL_UNKNOWN;
    }

    if (IsPunctuationNode(first, PUNCTUATION_HASH) && TokenTextEquals(Next(first), file, "import")) {
        decl->body = Next(Next(first));
        return KEK_DECL_IMPORT;
    }

    if (first->type == AST_INDEX) {
        first = Next(first);
    }

    if (IsKeywordNode(first, KEYWORD_USING)
        || IsKeywordNode(first, KEYWORD_ALIAS)
        || IsKeywordNode(first, KEYWORD_EXTERN)
        || IsKeywordNode(first, KEYWORD_STRUCT)
        || IsKeywordNode(first, KEYWORD_ENUM)
        || IsKeywordNode(first, KEYWORD_UNION)) {
        return ClassifyKeywordDecl(first, file, decl);
    }

    struct AstNode* colon = IsTokenNode(first) ? Next(first) : NULL;
    if (IsGenericNode(colon)) {
        colon = Next(colon);
    }
    if (IsTokenNode(first) && colon && IsPunctuationNode(colon, PUNCTUATION_COLON)) {
        struct AstNode* name = Next(colon);
        struct AstNode* afterName = Next(name);
        if (IsGenericNode(afterName)) {
            afterName = Next(afterName);
        }
        if (IsTokenNode(name) && afterName && IsOperatorNode(afterName, OPERATOR_SCOPE)) {
            decl->type = first;
            decl->name = Next(afterName);
            if (IsTokenNode(decl->name)
                && decl->name->token.type == TOKEN_IDENTIFIER
                && IsAnyOverloadableOperatorNode(Next(decl->name))
                && TokenTextEquals(decl->name, file, "operator")) {
                decl->name = Next(decl->name);
            }
            struct AstNode* receiverGenericParams = IsGenericNode(Next(name)) ? Next(name) : NULL;
            decl->genericParams = IsGenericNode(Next(decl->name)) ? Next(decl->name) : receiverGenericParams;
            struct AstNode* params = Next(decl->genericParams ? decl->genericParams : decl->name);
            if (receiverGenericParams && decl->genericParams == receiverGenericParams) {
                params = Next(decl->name);
            }
            decl->body = params ? Next(params) : NULL;
            return KEK_DECL_FUNCTION;
        }
        return ClassifyColonDecl(first, file, decl);
    }

    return KEK_DECL_UNKNOWN;
}

static void AddDeclStmt(struct KekDecl* decl, struct KekStmt* stmt) {
    if (!decl || !stmt) {
        return;
    }
    if (decl->lastStmt) {
        decl->lastStmt->next = stmt;
    } else {
        decl->firstStmt = stmt;
    }
    decl->lastStmt = stmt;
}

static void AddDeclParam(struct KekDecl* decl, struct KekParam* param) {
    if (!decl || !param) {
        return;
    }
    if (decl->lastParam) {
        decl->lastParam->next = param;
    } else {
        decl->firstParam = param;
    }
    decl->lastParam = param;
}

static void AddDeclField(struct KekDecl* decl, struct KekField* field) {
    if (!decl || !field) {
        return;
    }
    if (decl->lastField) {
        decl->lastField->next = field;
    } else {
        decl->firstField = field;
    }
    decl->lastField = field;
}

static void AddDeclVariant(struct KekDecl* decl, struct KekVariant* variant) {
    if (!decl || !variant) {
        return;
    }
    if (decl->lastVariant) {
        decl->lastVariant->next = variant;
    } else {
        decl->firstVariant = variant;
    }
    decl->lastVariant = variant;
}

static void BuildParamList(struct KekFrontend* frontend, struct KekModule* module, struct KekDecl* decl, struct AstNode* group) {
    if (!group || group->type != AST_GROUP) {
        return;
    }

    for (struct AstNode* paramNode = group->firstChild; paramNode; paramNode = paramNode->nextSibling) {
        struct AstNode* typeNode = paramNode->firstChild;
        if (!LooksLikeDecl(typeNode)) {
            continue;
        }

        struct AstNode* name = DeclNameAfterType(typeNode);
        struct KekParam* param = AddParam(frontend, module, paramNode);
        if (!param) {
            return;
        }
        param->type = ParseType(frontend, module, typeNode, name);
        param->name = name;
        struct AstNode* afterParam = AfterNameAndArraySuffixes(name);
        if (afterParam && IsOperatorNode(afterParam, OPERATOR_ASSIGN)) {
            param->defaultValue = ParseExprUntil(frontend, module, Next(afterParam), NULL);
        }
        AddDeclParam(decl, param);
    }
}

// Check if a node is a nested struct definition: struct:Name { ... }
static int IsNestedStructDecl(struct AstNode* first) {
    if (!IsKeywordNode(first, KEYWORD_STRUCT)) {
        return 0;
    }
    struct AstNode* colon = Next(first);
    if (!IsPunctuationNode(colon, PUNCTUATION_COLON)) {
        return 0;
    }
    struct AstNode* name = Next(colon);
    if (!IsTokenNode(name) || name->token.type != TOKEN_IDENTIFIER) {
        return 0;
    }
    struct AstNode* block = Next(name);
    return block && block->type == AST_BLOCK;
}

static void BuildNestedStructFields(struct KekFrontend* frontend, struct KekModule* module, struct KekField* parentField, struct AstNode* block);

static void BuildStructFields(struct KekFrontend* frontend, struct KekModule* module, struct KekDecl* decl) {
    if (!decl->body || decl->body->type != AST_BLOCK) {
        return;
    }

    for (struct AstNode* field = decl->body->firstChild; field; field = field->nextSibling) {
        // Check for nested struct FIRST (before LooksLikeDecl, since struct:Name also matches Type:Name pattern)
        if (IsNestedStructDecl(field->firstChild)) {
            // Nested struct: struct:Name { ... }
            struct AstNode* keyword = field->firstChild;
            struct AstNode* colon = Next(keyword);
            struct AstNode* name = Next(colon);
            struct AstNode* block = Next(name);

            struct KekField* nestedField = AddField(frontend, module, field);
            if (!nestedField) {
                return;
            }
            nestedField->name = name;
            nestedField->isNestedStruct = 1;

            // Recursively parse nested struct fields
            BuildNestedStructFields(frontend, module, nestedField, block);

            AddDeclField(decl, nestedField);
        } else if (LooksLikeDecl(field->firstChild)) {
            struct AstNode* fieldName = DeclNameAfterType(field->firstChild);
            struct KekField* typedField = AddField(frontend, module, field);
            if (!typedField) {
                return;
            }
            typedField->type = ParseType(frontend, module, field->firstChild, fieldName);
            typedField->name = fieldName;
            struct AstNode* afterField = AfterNameAndArraySuffixes(fieldName);
            if (afterField && IsOperatorNode(afterField, OPERATOR_ASSIGN)) {
                typedField->defaultValue = ParseExprUntil(frontend, module, Next(afterField), NULL);
            }
            AddDeclField(decl, typedField);
        }
    }
}

static void BuildNestedStructFields(struct KekFrontend* frontend, struct KekModule* module, struct KekField* parentField, struct AstNode* block) {
    if (!block || block->type != AST_BLOCK) {
        return;
    }

    for (struct AstNode* field = block->firstChild; field; field = field->nextSibling) {
        if (LooksLikeDecl(field->firstChild)) {
            struct AstNode* fieldName = DeclNameAfterType(field->firstChild);
            struct KekField* typedField = AddField(frontend, module, field);
            if (!typedField) {
                return;
            }
            typedField->type = ParseType(frontend, module, field->firstChild, fieldName);
            typedField->name = fieldName;
            struct AstNode* afterField = AfterNameAndArraySuffixes(fieldName);
            if (afterField && IsOperatorNode(afterField, OPERATOR_ASSIGN)) {
                typedField->defaultValue = ParseExprUntil(frontend, module, Next(afterField), NULL);
            }
            // Add to nested fields list
            if (parentField->lastNestedField) {
                parentField->lastNestedField->next = typedField;
            } else {
                parentField->nestedFields = typedField;
            }
            parentField->lastNestedField = typedField;
            typedField->next = NULL;
        }
    }
}

static void BuildEnumVariants(struct KekFrontend* frontend, struct KekModule* module, struct KekDecl* decl) {
    if (!decl->body || decl->body->type != AST_BLOCK) {
        return;
    }

    for (struct AstNode* variantNode = decl->body->firstChild; variantNode; variantNode = variantNode->nextSibling) {
        struct AstNode* name = variantNode->firstChild;
        if (!IsTokenNode(name) || name->token.type != TOKEN_IDENTIFIER) {
            continue;
        }
        struct KekVariant* variant = AddVariant(frontend, module, variantNode);
        if (!variant) {
            return;
        }
        variant->name = name;
        if (Next(name) && IsOperatorNode(Next(name), OPERATOR_ASSIGN)) {
            variant->value = ParseExprUntil(frontend, module, Next(Next(name)), NULL);
        }
        AddDeclVariant(decl, variant);
    }
}

static void BuildDeclDetails(struct KekFrontend* frontend, struct KekModule* module, struct KekDecl* decl) {
    if (!decl) {
        return;
    }

    if (decl->type) {
        decl->parsedType = ParseType(frontend, module, decl->type, decl->name);
    }

    if (decl->kind == KEK_DECL_FUNCTION) {
        struct AstNode* params = decl->name ? Next(decl->name) : NULL;
        if (IsGenericNode(params)) {
            params = Next(params);
        }
        if (params && params->type == AST_GROUP) {
            BuildParamList(frontend, module, decl, params);
        }
        if (decl->body && decl->body->type == AST_BLOCK) {
            AddDeclStmt(decl, ParseStatementList(frontend, module, decl->body));
        }
    }

    if (decl->kind == KEK_DECL_STRUCT || decl->kind == KEK_DECL_UNION) {
        BuildStructFields(frontend, module, decl);
    }

    if (decl->kind == KEK_DECL_ENUM) {
        BuildEnumVariants(frontend, module, decl);
    }
}

struct KekModule ParseKekModule(struct KekFrontend* frontend, struct AstNode* ast, struct SourceFile* file) {
    struct KekModule module = {0};
    module.file = file;

    if (!frontend || !ast || ast->type != AST_FILE || !file) {
        if (frontend) {
            frontend->errorCount++;
        }
        module.errorCount++;
        return module;
    }

    frontend->file = file;

    for (struct AstNode* statement = ast->firstChild; statement; statement = statement->nextSibling) {
        if (statement->type != AST_STATEMENT || statement->childCount == 0) {
            continue;
        }

        struct KekDecl* decl = AddDecl(frontend, &module, KEK_DECL_UNKNOWN, statement);
        if (!decl) {
            break;
        }

        decl->kind = ClassifyDecl(decl, file);
        module.declKindCounts[KEK_DECL_UNKNOWN]--;
        module.declKindCounts[decl->kind]++;
        if (decl->kind == KEK_DECL_UNKNOWN) {
            KekAddDiagnostic(frontend->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_TYPED_PARSE,
                file ? file->fileIndex : -1, decl->location, "unknown top-level declaration");
            module.errorCount++;
            frontend->errorCount++;
        }
        BuildDeclDetails(frontend, &module, decl);
    }

    return module;
}

static void WriteKekModuleSummary(FILE* out, struct KekModule* module) {
    if (!module) {
        return;
    }

    fprintf(out, "Typed module %s: %zu declarations", module->file ? module->file->path : "<unknown>", module->declCount);
    if (module->errorCount > 0) {
        fprintf(out, ", %d errors", module->errorCount);
    }
    fprintf(out, "\n");

    for (size_t i = 0; i < KEK_DECL_COUNT; i++) {
        if (module->declKindCounts[i] > 0) {
            fprintf(out, "  %s: %zu\n", KekDeclKindNames[i], module->declKindCounts[i]);
        }
    }

    fprintf(out, "Typed nodes\n");
    fprintf(out, "  Params: %zu\n", module->paramCount);
    fprintf(out, "  Fields: %zu\n", module->fieldCount);
    fprintf(out, "  Variants: %zu\n", module->variantCount);
    fprintf(out, "  Statements: %zu\n", module->typedStmtCount);
    fprintf(out, "  Expressions: %zu\n", module->typedExprCount);
    fprintf(out, "  Types: %zu\n", module->typedTypeCount);

    fprintf(out, "Statements\n");
    for (size_t i = 0; i < KEK_STMT_COUNT; i++) {
        if (module->stmtKindCounts[i] > 0) {
            fprintf(out, "  %s: %zu\n", KekStmtKindNames[i], module->stmtKindCounts[i]);
        }
    }

    fprintf(out, "Expressions\n");
    for (size_t i = 0; i < KEK_EXPR_COUNT; i++) {
        if (module->exprKindCounts[i] > 0) {
            fprintf(out, "  %s: %zu\n", KekExprKindNames[i], module->exprKindCounts[i]);
        }
    }

    fprintf(out, "Types\n");
    for (size_t i = 0; i < KEK_TYPE_COUNT; i++) {
        if (module->typeKindCounts[i] > 0) {
            fprintf(out, "  %s: %zu\n", KekTypeKindNames[i], module->typeKindCounts[i]);
        }
    }
}

void PrintKekModuleSummary(struct KekModule* module) {
    WriteKekModuleSummary(stdout, module);
}

int WriteKekModuleSummaryFile(const char* path, struct KekModule* modules, size_t moduleCount, struct KekProgram* program) {
    FILE* out = fopen(path, "w");
    if (!out) {
        return -1;
    }

    for (size_t i = 0; i < moduleCount; i++) {
        if (i > 0) {
            fputc('\n', out);
        }
        WriteKekModuleSummary(out, &modules[i]);
    }

    if (program) {
        fprintf(out, "\nTyped program symbols: %zu", program->symbolCount);
        if (program->errorCount > 0) {
            fprintf(out, ", %d errors", program->errorCount);
        }
        fputc('\n', out);
        for (size_t i = 0; i < KEK_SYMBOL_COUNT; i++) {
            if (program->symbolKindCounts[i] > 0) {
                fprintf(out, "  %s: %zu\n", KekSymbolKindNames[i], program->symbolKindCounts[i]);
            }
        }
        fprintf(out, "Typed scopes: %zu\n", program->scopeCount);
        for (size_t i = 0; i < KEK_SCOPE_COUNT; i++) {
            if (program->scopeKindCounts[i] > 0) {
                fprintf(out, "  %s: %zu\n", KekScopeKindNames[i], program->scopeKindCounts[i]);
            }
        }
        fprintf(out, "Semantic checks: %zu\n", program->semanticCheckCount);
    }

    fclose(out);
    return 0;
}
#undef WriteKekModuleSummary
#undef BuildDeclDetails
#undef BuildEnumVariants
#undef BuildStructFields
#undef BuildNestedStructFields
#undef IsNestedStructDecl
#undef BuildParamList
#undef AddDeclVariant
#undef AddDeclField
#undef AddDeclParam
#undef AddDeclStmt
#undef ClassifyDecl
#undef ClassifyKeywordDecl
#undef ClassifyColonDecl
#undef IsDoWhileConditionStatement
#undef ParseStatement
#undef ParseStatementList
#undef ClassifyStatementKind
#undef ParseFirstGroupStatementExpr
#undef AfterNameAndArraySuffixes
#undef DeclNameAfterType
#undef DeclColonAfterType
#undef LooksLikeDecl
#undef AddChildStmt
#undef IsBuiltinTypeName
#undef ParsePostfixExpr
#undef ParsePrimaryExpr
#undef AddCallArgs
#undef ParseExprPrecedence
#undef IsUnaryExprOperator
#undef IsRightAssociativeExprOperator
#undef ExprOperatorPrecedence
#undef IsAssignmentExprOperator
#undef IsExpressionEnd
#undef ParseGroupExpr
#undef ParseStructLiteralExpr
#undef AddStructLiteralFields
#undef AddExprArg
#undef ParsePointerElementType
#undef AddTypedParseDiagnostic
#undef ParseType
#undef ParseExprUntil
#undef AddDecl
#undef AddVariant
#undef AddField
#undef AddParam
#undef AddStmt
#undef AddExpr
#undef AddType
#undef AddFrontendNode
#undef Next
#undef TokenTextEquals
#undef IsAnyOverloadableOperatorNode
#undef IsOperatorNode
#undef IsGenericNode
#undef IsPunctuationNode
#undef IsKeywordNode
#undef IsTokenNode
/* END parser.c */

/* BEGIN sema.c */
#define SymbolKindForDecl sema_SymbolKindForDecl
#define SameSymbolNameInFile sema_SameSymbolNameInFile
#define SameSymbolNameAcrossFiles sema_SameSymbolNameAcrossFiles
#define IsOperatorDeclName sema_IsOperatorDeclName
#define DeclIsMethod sema_DeclIsMethod
#define DeclReceiverName sema_DeclReceiverName
#define DeclParamCount sema_DeclParamCount
#define SameFunctionSymbolSlot sema_SameFunctionSymbolSlot
#define AddScope sema_AddScope
#define ScopeFindDuplicate sema_ScopeFindDuplicate
#define AddSymbolWithFile sema_AddSymbolWithFile
#define AddSymbol sema_AddSymbol
#define IsTokenText sema_IsTokenText
#define IsBuiltinType sema_IsBuiltinType
#define LookupSymbol sema_LookupSymbol
#define ProgramScopeHasSymbol sema_ProgramScopeHasSymbol
#define AddExternCStructSymbols sema_AddExternCStructSymbols
#define IsAssignableExpr sema_IsAssignableExpr
#define CheckTypeSemantics sema_CheckTypeSemantics
#define CheckExprSemantics sema_CheckExprSemantics
#define BuildStmtSymbols sema_BuildStmtSymbols
#define BuildChildStmtSymbols sema_BuildChildStmtSymbols
#define BuildFunctionSymbols sema_BuildFunctionSymbols

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
#undef BuildFunctionSymbols
#undef BuildChildStmtSymbols
#undef BuildStmtSymbols
#undef CheckExprSemantics
#undef CheckTypeSemantics
#undef IsAssignableExpr
#undef AddExternCStructSymbols
#undef ProgramScopeHasSymbol
#undef LookupSymbol
#undef IsBuiltinType
#undef IsTokenText
#undef AddSymbol
#undef AddSymbolWithFile
#undef ScopeFindDuplicate
#undef AddScope
#undef SameFunctionSymbolSlot
#undef DeclParamCount
#undef DeclReceiverName
#undef DeclIsMethod
#undef IsOperatorDeclName
#undef SameSymbolNameAcrossFiles
#undef SameSymbolNameInFile
#undef SymbolKindForDecl
/* END sema.c */

/* BEGIN tokenizer.c */
#define PeekChar tokenizer_PeekChar
#define PeekCharAt tokenizer_PeekCharAt
#define Advance tokenizer_Advance
#define CreateTextToken tokenizer_CreateTextToken
#define CreateCommentToken tokenizer_CreateCommentToken
#define CreateOperatorToken tokenizer_CreateOperatorToken
#define CreateKeywordToken tokenizer_CreateKeywordToken
#define CreatePunctuationToken tokenizer_CreatePunctuationToken
#define IsPunctuation tokenizer_IsPunctuation
#define IsOperator tokenizer_IsOperator
#define IsKeywordAt tokenizer_IsKeywordAt
#define SkipWhitespace tokenizer_SkipWhitespace
#define SkipWhitespaceAndComments tokenizer_SkipWhitespaceAndComments
#define ReadLineComment tokenizer_ReadLineComment
#define ReadBlockComment tokenizer_ReadBlockComment
#define PushToken tokenizer_PushToken

const char* TokenTypeNames[] = {
    "EOF",
    "IDENTIFIER",
    "NUMBER",
    "STRING",
    "CHAR",
    "COMMENT",
    "DOC_COMMENT",
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
    "switch", "case", "default", "each", "packed", "aligned", "comptime",
    "defer", "tagged", "true", "false", "unreachable", "panic"
};

const char* PunctuationNames[] = {
    "(", ")", "{", "}", "[", "]", ";", ",", ":", ".", "#"
};

struct Tokenizer CreateTokenizer(int fileIndex, struct FileTable* table) {
    struct KekLexOptions options = {0};
    return CreateTokenizerWithOptions(fileIndex, table, options);
}

struct Tokenizer CreateTokenizerWithOptions(int fileIndex, struct FileTable* table, struct KekLexOptions options) {
    struct Tokenizer tokenizer = {0};
    tokenizer.file = &table->files[fileIndex];
    tokenizer.fileIndex = fileIndex;
    tokenizer.position = 0;
    tokenizer.line = 1;
    tokenizer.column = 1;
    tokenizer.emitComments = options.emitComments;
    tokenizer.diagnostics = options.diagnostics;
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

static struct Token CreateCommentToken(enum TokenType type, size_t start, size_t length, size_t line, size_t column) {
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

static void SkipWhitespace(struct Tokenizer* tokenizer) {
    char c = PeekChar(tokenizer);
    while (c != '\0' && isspace((unsigned char)c)) {
        Advance(tokenizer, 1);
        c = PeekChar(tokenizer);
    }
}

static void SkipWhitespaceAndComments(struct Tokenizer* tokenizer) {
    for (;;) {
        SkipWhitespace(tokenizer);

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

static struct Token ReadLineComment(struct Tokenizer* tokenizer) {
    size_t start = tokenizer->position;
    size_t line = tokenizer->line;
    size_t column = tokenizer->column;
    enum TokenType type = PeekCharAt(tokenizer, 2) == '/' ? TOKEN_DOC_COMMENT : TOKEN_COMMENT;
    while (PeekChar(tokenizer) != '\0' && PeekChar(tokenizer) != '\n') {
        Advance(tokenizer, 1);
    }
    return CreateCommentToken(type, start, tokenizer->position - start, line, column);
}

static struct Token ReadBlockComment(struct Tokenizer* tokenizer) {
    size_t start = tokenizer->position;
    size_t line = tokenizer->line;
    size_t column = tokenizer->column;
    Advance(tokenizer, 2);
    while (PeekChar(tokenizer) != '\0') {
        if (PeekChar(tokenizer) == '*' && PeekCharAt(tokenizer, 1) == '/') {
            Advance(tokenizer, 2);
            break;
        }
        Advance(tokenizer, 1);
    }
    return CreateCommentToken(TOKEN_COMMENT, start, tokenizer->position - start, line, column);
}

struct Token GetNextToken(struct Tokenizer* tokenizer) {
    struct Token token = {0};
    if (tokenizer->emitComments) {
        SkipWhitespace(tokenizer);
        if (PeekChar(tokenizer) == '/' && PeekCharAt(tokenizer, 1) == '/') {
            return ReadLineComment(tokenizer);
        }
        if (PeekChar(tokenizer) == '/' && PeekCharAt(tokenizer, 1) == '*') {
            return ReadBlockComment(tokenizer);
        }
    } else {
        SkipWhitespaceAndComments(tokenizer);
    }

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

    if (c == '\'') {
        Advance(tokenizer, 1);
        while (PeekChar(tokenizer) != '\0') {
            if (PeekChar(tokenizer) == '\\') {
                Advance(tokenizer, 2);
                continue;
            }
            if (PeekChar(tokenizer) == '\'') {
                Advance(tokenizer, 1);
                break;
            }
            Advance(tokenizer, 1);
        }
        return CreateTextToken(TOKEN_CHAR, start, tokenizer->position - start, line, column);
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

    KekAddDiagnosticFormat(tokenizer->diagnostics, KEK_DIAGNOSTIC_WARNING, KEK_PHASE_LEX, tokenizer->fileIndex,
        (struct SourceLocation){line, column, start, 1}, "unrecognized character '%c'", c);
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
        case TOKEN_CHAR:
        case TOKEN_COMMENT:
        case TOKEN_DOC_COMMENT:
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

static int PushToken(struct TokenArray* array, struct Tokenizer* tokenizer, struct Token token) {
    if (array->count >= array->capacity) {
        KekAddDiagnostic(tokenizer->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_LEX,
            tokenizer->fileIndex, token.location, "token storage capacity exceeded");
        return -1;
    }
    array->items[array->count++] = token;
    return 0;
}

struct TokenArray TokenizeFile(struct Tokenizer* tokenizer, struct Token* storage, size_t capacity) {
    struct TokenArray array = {0};
    array.items = storage;
    array.capacity = capacity;

    for (;;) {
        struct Token token = GetNextToken(tokenizer);
        if (PushToken(&array, tokenizer, token) != 0) {
            break;
        }
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
#undef PushToken
#undef ReadBlockComment
#undef ReadLineComment
#undef SkipWhitespaceAndComments
#undef SkipWhitespace
#undef IsKeywordAt
#undef IsOperator
#undef IsPunctuation
#undef CreatePunctuationToken
#undef CreateKeywordToken
#undef CreateOperatorToken
#undef CreateCommentToken
#undef CreateTextToken
#undef Advance
#undef PeekCharAt
#undef PeekChar
/* END tokenizer.c */

/* BEGIN ast_json.c */
#define WriteTokenTextJson ast_json_WriteTokenTextJson

void WriteJsonEscaped(FILE* out, const char* text, size_t length) {
    fputc('"', out);
    for (size_t i = 0; i < length; i++) {
        unsigned char c = (unsigned char)text[i];
        switch (c) {
            case '"': fputs("\\\"", out); break;
            case '\\': fputs("\\\\", out); break;
            case '\n': fputs("\\n", out); break;
            case '\r': fputs("\\r", out); break;
            case '\t': fputs("\\t", out); break;
            default:
                if (c < 32) {
                    fprintf(out, "\\u%04x", c);
                } else {
                    fputc(c, out);
                }
                break;
        }
    }
    fputc('"', out);
}

static void WriteTokenTextJson(FILE* out, struct Token* token, struct SourceFile* file) {
    if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_NUMBER || token->type == TOKEN_STRING || token->type == TOKEN_CHAR) {
        WriteJsonEscaped(out, file->content + token->location.offset, token->location.length);
    } else if (token->type == TOKEN_EOF) {
        WriteJsonEscaped(out, "", 0);
    } else {
        const char* lexeme = TokenLexeme(token, file);
        WriteJsonEscaped(out, lexeme, strlen(lexeme));
    }
}

void WriteAstJson(FILE* out, struct AstNode* node, struct SourceFile* file, int indent) {
    for (int i = 0; i < indent; i++) fputs("  ", out);
    fputs("{\n", out);

    for (int i = 0; i < indent + 1; i++) fputs("  ", out);
    fputs("\"type\": ", out);
    WriteJsonEscaped(out, AstNodeTypeNames[node->type], strlen(AstNodeTypeNames[node->type]));
    fputs(",\n", out);

    for (int i = 0; i < indent + 1; i++) fputs("  ", out);
    fprintf(out, "\"line\": %zu, \"column\": %zu, \"offset\": %zu, \"length\": %zu",
        node->location.line, node->location.column, node->location.offset, node->location.length);

    if (node->type == AST_TOKEN) {
        fputs(",\n", out);
        for (int i = 0; i < indent + 1; i++) fputs("  ", out);
        fputs("\"tokenType\": ", out);
        WriteJsonEscaped(out, TokenTypeNames[node->token.type], strlen(TokenTypeNames[node->token.type]));
        fputs(",\n", out);
        for (int i = 0; i < indent + 1; i++) fputs("  ", out);
        fputs("\"text\": ", out);
        WriteTokenTextJson(out, &node->token, file);
    }

    fputs(",\n", out);
    for (int i = 0; i < indent + 1; i++) fputs("  ", out);
    fputs("\"children\": [", out);
    if (node->childCount > 0) {
        fputc('\n', out);
        size_t index = 0;
        for (struct AstNode* child = node->firstChild; child; child = child->nextSibling) {
            WriteAstJson(out, child, file, indent + 2);
            if (index + 1 < node->childCount) {
                fputc(',', out);
            }
            fputc('\n', out);
            index++;
        }
        for (int i = 0; i < indent + 1; i++) fputs("  ", out);
    }
    fputs("]\n", out);

    for (int i = 0; i < indent; i++) fputs("  ", out);
    fputc('}', out);
}

int WriteAstJsonFile(const char* path, struct AstNode* ast, struct SourceFile* file) {
    FILE* out = fopen(path, "w");
    if (!out) {
        return -1;
    }
    WriteAstJson(out, ast, file, 0);
    fputc('\n', out);
    fclose(out);
    return 0;
}
#undef WriteTokenTextJson
/* END ast_json.c */

/* BEGIN source.c */
#define AddSourceDiagnostic source_AddSourceDiagnostic

static void AddSourceDiagnostic(struct KekDiagnosticBag* diagnostics, const char* message) {
    KekAddDiagnostic(diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SOURCE, -1, (struct SourceLocation){0}, message);
}

int ReadFileWithDiagnostics(const char* path, struct FileTable* table, struct KekDiagnosticBag* diagnostics) {
    if (table->count >= MAX_FILES) {
        AddSourceDiagnostic(diagnostics, "file table is full");
        return -1;
    }

    if (strlen(path) >= MAX_PATH_LENGTH) {
        KekAddDiagnosticFormat(diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SOURCE, -1, (struct SourceLocation){0}, "path is too long: %s", path);
        return -1;
    }

    FILE* file = fopen(path, "r");
    if (!file) {
        KekAddDiagnosticFormat(diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SOURCE, -1, (struct SourceLocation){0}, "could not open file %s", path);
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        KekAddDiagnosticFormat(diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SOURCE, -1, (struct SourceLocation){0}, "could not seek file %s", path);
        fclose(file);
        return -1;
    }
    long length = ftell(file);
    if (length < 0) {
        KekAddDiagnosticFormat(diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SOURCE, -1, (struct SourceLocation){0}, "could not measure file %s", path);
        fclose(file);
        return -1;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        KekAddDiagnosticFormat(diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SOURCE, -1, (struct SourceLocation){0}, "could not rewind file %s", path);
        fclose(file);
        return -1;
    }

    char* content = malloc((size_t)length + 1);
    if (!content) {
        AddSourceDiagnostic(diagnostics, "could not allocate memory for file content");
        fclose(file);
        return -1;
    }

    size_t bytesRead = fread(content, 1, (size_t)length, file);
    if (bytesRead != (size_t)length) {
        KekAddDiagnosticFormat(diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SOURCE, -1, (struct SourceLocation){0}, "could not read file %s", path);
        free(content);
        fclose(file);
        return -1;
    }
    content[length] = '\0';
    fclose(file);

    struct SourceFile* sourceFile = &table->files[table->count++];
    strcpy(sourceFile->path, path);
    sourceFile->content = content;
    sourceFile->length = (size_t)length;
    sourceFile->fileIndex = (int)table->count - 1;

    return (int)table->count - 1;
}

int ReadFile(const char* path, struct FileTable* table) {
    struct KekDiagnostic diagnostics[8];
    struct KekDiagnosticBag bag;
    InitKekDiagnosticBag(&bag, diagnostics, sizeof(diagnostics) / sizeof(diagnostics[0]));
    int result = ReadFileWithDiagnostics(path, table, &bag);
    PrintKekDiagnostics(stderr, &bag, table);
    return result;
}

void FreeFileTable(struct FileTable* table) {
    for (size_t i = 0; i < table->count; i++) {
        free(table->files[i].content);
        table->files[i].content = NULL;
        table->files[i].length = 0;
    }
    table->count = 0;
}

const char* SourceLocationText(struct SourceFile* file, struct SourceLocation location, size_t* length) {
    if (length) {
        *length = 0;
    }
    if (!file || location.offset > file->length || location.length > file->length - location.offset) {
        return "";
    }
    if (length) {
        *length = location.length;
    }
    return file->content + location.offset;
}

const char* TokenText(struct Token* token, struct SourceFile* file, size_t* length) {
    if (!token) {
        if (length) {
            *length = 0;
        }
        return "";
    }
    return SourceLocationText(file, token->location, length);
}

const char* AstNodeText(struct AstNode* node, struct SourceFile* file, size_t* length) {
    if (!node) {
        if (length) {
            *length = 0;
        }
        return "";
    }
    return SourceLocationText(file, node->location, length);
}
#undef AddSourceDiagnostic
/* END source.c */

/* BEGIN diagnostics.c */
#define DiagnosticSeverityName diagnostics_DiagnosticSeverityName
#define DiagnosticPhaseName diagnostics_DiagnosticPhaseName
#define PrintSourceLine diagnostics_PrintSourceLine


static const char* DiagnosticSeverityName(enum KekDiagnosticSeverity severity) {
    switch (severity) {
        case KEK_DIAGNOSTIC_NOTE:
            return "note";
        case KEK_DIAGNOSTIC_WARNING:
            return "warning";
        case KEK_DIAGNOSTIC_ERROR:
            return "error";
    }
    return "diagnostic";
}

static const char* DiagnosticPhaseName(enum KekDiagnosticPhase phase) {
    switch (phase) {
        case KEK_PHASE_SOURCE:
            return "source";
        case KEK_PHASE_LEX:
            return "lex";
        case KEK_PHASE_PARSE:
            return "parse";
        case KEK_PHASE_TYPED_PARSE:
            return "typed-parse";
        case KEK_PHASE_SEMANTIC:
            return "semantic";
        case KEK_PHASE_CODEGEN:
            return "codegen";
    }
    return "unknown";
}

void InitKekDiagnosticBag(struct KekDiagnosticBag* bag, struct KekDiagnostic* storage, size_t capacity) {
    if (!bag) {
        return;
    }
    bag->items = storage;
    bag->count = 0;
    bag->capacity = capacity;
    bag->errorCount = 0;
}

void KekAddDiagnostic(struct KekDiagnosticBag* bag, enum KekDiagnosticSeverity severity, enum KekDiagnosticPhase phase, int fileIndex, struct SourceLocation location, const char* message) {
    if (!bag) {
        return;
    }
    if (severity == KEK_DIAGNOSTIC_ERROR) {
        bag->errorCount++;
    }
    if (bag->count >= bag->capacity || !bag->items) {
        return;
    }

    struct KekDiagnostic* diagnostic = &bag->items[bag->count++];
    memset(diagnostic, 0, sizeof(*diagnostic));
    diagnostic->severity = severity;
    diagnostic->phase = phase;
    diagnostic->fileIndex = fileIndex;
    diagnostic->location = location;
    snprintf(diagnostic->message, sizeof(diagnostic->message), "%s", message ? message : "");
}

void KekAddDiagnosticFormat(struct KekDiagnosticBag* bag, enum KekDiagnosticSeverity severity, enum KekDiagnosticPhase phase, int fileIndex, struct SourceLocation location, const char* format, ...) {
    if (!bag) {
        return;
    }

    char message[256];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format ? format : "", args);
    va_end(args);

    KekAddDiagnostic(bag, severity, phase, fileIndex, location, message);
}

static void PrintSourceLine(FILE* out, struct SourceFile* file, struct SourceLocation location) {
    if (!file || !file->content || location.offset > file->length) {
        return;
    }

    // Find the start of the line
    size_t lineStart = location.offset;
    while (lineStart > 0 && file->content[lineStart - 1] != '\n') {
        lineStart--;
    }

    // Find the end of the line
    size_t lineEnd = location.offset;
    while (lineEnd < file->length && file->content[lineEnd] != '\n') {
        lineEnd++;
    }

    // Print the source line
    size_t lineLength = lineEnd - lineStart;
    if (lineLength > 0) {
        fprintf(out, " %5zu | %.*s\n", location.line, (int)lineLength, file->content + lineStart);

        // Print the column marker
        fprintf(out, "       | ");
        size_t markerCol = location.offset - lineStart;
        for (size_t j = 0; j < markerCol; j++) {
            char c = file->content[lineStart + j];
            fputc(c == '\t' ? '\t' : ' ', out);
        }
        fputc('^', out);
        // Extend marker for multi-character tokens
        size_t tokenLen = location.length > 0 ? location.length : 1;
        for (size_t j = 1; j < tokenLen && (lineStart + markerCol + j) < lineEnd; j++) {
            fputc('~', out);
        }
        fputc('\n', out);
    }
}

void PrintKekDiagnostics(FILE* out, struct KekDiagnosticBag* bag, struct FileTable* table) {
    if (!out || !bag) {
        return;
    }

    for (size_t i = 0; i < bag->count; i++) {
        struct KekDiagnostic* diagnostic = &bag->items[i];
        const char* path = "<unknown>";
        struct SourceFile* file = NULL;
        if (table
            && diagnostic->fileIndex >= 0
            && (size_t)diagnostic->fileIndex < table->count) {
            file = &table->files[diagnostic->fileIndex];
            path = file->path;
        }
        fprintf(out, "%s:%zu:%zu: %s[%s]: %s\n",
            path,
            diagnostic->location.line,
            diagnostic->location.column,
            DiagnosticSeverityName(diagnostic->severity),
            DiagnosticPhaseName(diagnostic->phase),
            diagnostic->message);

        // Print the source line with column marker
        if (file && diagnostic->location.line > 0) {
            PrintSourceLine(out, file, diagnostic->location);
        }
    }
}

#undef PrintSourceLine
#undef DiagnosticPhaseName
#undef DiagnosticSeverityName
/* END diagnostics.c */

/* BEGIN compilation.c */
#define EndsWith compilation_EndsWith
#define FileAlreadyLoaded compilation_FileAlreadyLoaded
#define StdlibImportRank compilation_StdlibImportRank
#define CompareImportNames compilation_CompareImportNames
#define LoadImportDirectory compilation_LoadImportDirectory
#define LoadImports compilation_LoadImports
#define ParseUnit compilation_ParseUnit
#define IsOnlyWhitespace compilation_IsOnlyWhitespace


static int EndsWith(const char* text, const char* suffix) {
    size_t textLength = strlen(text);
    size_t suffixLength = strlen(suffix);
    return textLength >= suffixLength && strcmp(text + textLength - suffixLength, suffix) == 0;
}

static int FileAlreadyLoaded(struct FileTable* table, const char* path) {
    for (size_t i = 0; i < table->count; i++) {
        if (strcmp(table->files[i].path, path) == 0) {
            return 1;
        }
    }
    return 0;
}

static int StdlibImportRank(const char* name) {
    static const char* ordered[] = {
        "core.kek",
        "mem.kek",
        "file.kek",
        "io.kek",
        "string.kek",
        "scan.kek",
        "array.kek",
        "list.kek",
        "hash.kek",
        "collections.kek",
        "format.kek",
    };
    for (size_t i = 0; i < sizeof(ordered) / sizeof(ordered[0]); i++) {
        if (strcmp(name, ordered[i]) == 0) {
            return (int)i;
        }
    }
    return 1000;
}

static int CompareImportNames(const void* left, const void* right) {
    const char* const* leftString = left;
    const char* const* rightString = right;
    int leftRank = StdlibImportRank(*leftString);
    int rightRank = StdlibImportRank(*rightString);
    if (leftRank != rightRank) {
        return leftRank - rightRank;
    }
    return strcmp(*leftString, *rightString);
}

void FreeKekCompilationUnit(struct KekCompilationUnit* unit) {
    free(unit->tokens);
    free(unit->astNodes);
    free(unit->decls);
    free(unit->types);
    free(unit->exprs);
    free(unit->stmts);
    free(unit->params);
    free(unit->fields);
    free(unit->variants);
    unit->tokens = NULL;
    unit->astNodes = NULL;
    unit->decls = NULL;
    unit->types = NULL;
    unit->exprs = NULL;
    unit->stmts = NULL;
    unit->params = NULL;
    unit->fields = NULL;
    unit->variants = NULL;
    unit->ast = NULL;
    memset(&unit->module, 0, sizeof(unit->module));
}

void InitKekCompilation(struct KekCompilation* compilation, struct KekDiagnostic* diagnostics, size_t diagnosticCapacity) {
    if (!compilation) {
        return;
    }
    memset(compilation, 0, sizeof(*compilation));
    compilation->entryFileIndex = -1;
    InitKekDiagnosticBag(&compilation->diagnostics, diagnostics, diagnosticCapacity);
}

void FreeKekCompilation(struct KekCompilation* compilation) {
    if (!compilation) {
        return;
    }
    for (size_t i = 0; i < compilation->unitCount; i++) {
        FreeKekCompilationUnit(&compilation->units[i]);
    }
    free(compilation->symbols);
    free(compilation->scopes);
    compilation->symbols = NULL;
    compilation->scopes = NULL;
    FreeFileTable(&compilation->fileTable);
    compilation->unitCount = 0;
    compilation->entryFileIndex = -1;
    memset(&compilation->program, 0, sizeof(compilation->program));
}

static int LoadImportDirectory(struct KekCompilation* compilation, const char* path) {
    DIR* dir = opendir(path);
    if (!dir) {
        KekAddDiagnosticFormat(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SOURCE,
            -1, (struct SourceLocation){0}, "could not open import directory %s", path);
        return -1;
    }

    int result = 0;
    char names[256][MAX_PATH_LENGTH];
    const char* sortedNames[256];
    size_t nameCount = 0;
    struct dirent* entry = NULL;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' || !EndsWith(entry->d_name, ".kek")) {
            continue;
        }

        if (nameCount >= sizeof(names) / sizeof(names[0])) {
            KekAddDiagnosticFormat(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SOURCE,
                -1, (struct SourceLocation){0}, "too many import files in %s", path);
            result = -1;
            break;
        }

        int nameWritten = snprintf(names[nameCount], sizeof(names[nameCount]), "%s", entry->d_name);
        if (nameWritten < 0 || (size_t)nameWritten >= sizeof(names[nameCount])) {
            KekAddDiagnosticFormat(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SOURCE,
                -1, (struct SourceLocation){0}, "import file name is too long: %s", entry->d_name);
            result = -1;
            break;
        }
        sortedNames[nameCount] = names[nameCount];
        nameCount++;
    }
    closedir(dir);
    if (result != 0) {
        return result;
    }

    qsort(sortedNames, nameCount, sizeof(sortedNames[0]), CompareImportNames);

    for (size_t i = 0; i < nameCount; i++) {
        const char* name = sortedNames[i];

        char filePath[MAX_PATH_LENGTH];
        int written = snprintf(filePath, sizeof(filePath), "%s/%s", path, name);
        if (written < 0 || (size_t)written >= sizeof(filePath)) {
            KekAddDiagnosticFormat(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SOURCE,
                -1, (struct SourceLocation){0}, "import path is too long: %s/%s", path, name);
            result = -1;
            break;
        }

        if (!FileAlreadyLoaded(&compilation->fileTable, filePath)
            && ReadFileWithDiagnostics(filePath, &compilation->fileTable, &compilation->diagnostics) < 0) {
            result = -1;
            break;
        }
    }
    return result;
}

static int LoadImports(struct KekCompilation* compilation, struct SourceFile* file) {
    const char* importPrefix = "#import(";
    size_t importPrefixLength = strlen(importPrefix);
    const char* cursor = file->content;

    while ((cursor = strstr(cursor, importPrefix)) != NULL) {
        const char* start = cursor + importPrefixLength;
        const char* end = strchr(start, ')');
        struct SourceLocation location = {
            1,
            1,
            (size_t)(cursor - file->content),
            importPrefixLength
        };
        if (!end) {
            KekAddDiagnostic(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SOURCE,
                file->fileIndex, location, "unterminated import");
            return -1;
        }

        size_t length = (size_t)(end - start);
        if (length == 0 || length >= MAX_PATH_LENGTH) {
            KekAddDiagnostic(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SOURCE,
                file->fileIndex, location, "invalid import path");
            return -1;
        }

        char importPath[MAX_PATH_LENGTH];
        memcpy(importPath, start, length);
        importPath[length] = '\0';
        if (LoadImportDirectory(compilation, importPath) != 0) {
            return -1;
        }

        cursor = end + 1;
    }

    return 0;
}

int LoadKekCompilation(struct KekCompilation* compilation, const char* entryPath) {
    if (!compilation) {
        return -1;
    }
    int fileIndex = ReadFileWithDiagnostics(entryPath, &compilation->fileTable, &compilation->diagnostics);
    if (fileIndex < 0) {
        return -1;
    }
    compilation->entryFileIndex = fileIndex;
    return LoadImports(compilation, &compilation->fileTable.files[fileIndex]);
}

static int ParseUnit(struct KekCompilation* compilation, struct KekCompilationUnit* unit) {
    struct SourceFile* sourceFile = &compilation->fileTable.files[unit->fileIndex];
    size_t tokenCapacity = sourceFile->length + 1;
    unit->tokens = malloc(sizeof(*unit->tokens) * tokenCapacity);
    if (!unit->tokens) {
        KekAddDiagnostic(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_LEX,
            sourceFile->fileIndex, (struct SourceLocation){0}, "could not allocate token storage");
        goto fail;
    }

    struct KekLexOptions lexOptions = {0};
    lexOptions.diagnostics = &compilation->diagnostics;
    struct Tokenizer tokenizer = CreateTokenizerWithOptions(unit->fileIndex, &compilation->fileTable, lexOptions);
    struct TokenArray tokens = TokenizeFile(&tokenizer, unit->tokens, tokenCapacity);

    size_t astNodeCapacity = tokens.count * 4 + 1;
    unit->astNodes = malloc(sizeof(*unit->astNodes) * astNodeCapacity);
    if (!unit->astNodes) {
        KekAddDiagnostic(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_PARSE,
            sourceFile->fileIndex, (struct SourceLocation){0}, "could not allocate AST storage");
        goto fail;
    }

    struct Parser parser = {0};
    parser.tokens = tokens.items;
    parser.count = tokens.count;
    parser.file = sourceFile;
    parser.astNodes = unit->astNodes;
    parser.astNodeCapacity = astNodeCapacity;
    parser.diagnostics = &compilation->diagnostics;
    unit->ast = ParseAst(&parser);

    if (parser.errorCount > 0) {
        goto fail;
    }

    size_t declCapacity = unit->ast->childCount + 1;
    unit->decls = malloc(sizeof(*unit->decls) * declCapacity);
    if (!unit->decls) {
        KekAddDiagnostic(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_TYPED_PARSE,
            sourceFile->fileIndex, (struct SourceLocation){0}, "could not allocate typed declaration storage");
        goto fail;
    }

    size_t typeCapacity = tokens.count + 1;
    size_t exprCapacity = tokens.count * 2 + 1;
    size_t stmtCapacity = tokens.count + 1;
    size_t paramCapacity = tokens.count + 1;
    size_t fieldCapacity = tokens.count + 1;
    size_t variantCapacity = tokens.count + 1;
    unit->types = malloc(sizeof(*unit->types) * typeCapacity);
    unit->exprs = malloc(sizeof(*unit->exprs) * exprCapacity);
    unit->stmts = malloc(sizeof(*unit->stmts) * stmtCapacity);
    unit->params = malloc(sizeof(*unit->params) * paramCapacity);
    unit->fields = malloc(sizeof(*unit->fields) * fieldCapacity);
    unit->variants = malloc(sizeof(*unit->variants) * variantCapacity);
    if (!unit->types || !unit->exprs || !unit->stmts || !unit->params || !unit->fields || !unit->variants) {
        KekAddDiagnostic(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_TYPED_PARSE,
            sourceFile->fileIndex, (struct SourceLocation){0}, "could not allocate typed AST storage");
        goto fail;
    }

    struct KekFrontend frontend = {
        .decls = {unit->decls, 0, declCapacity, sizeof(*unit->decls)},
        .types = {unit->types, 0, typeCapacity, sizeof(*unit->types)},
        .exprs = {unit->exprs, 0, exprCapacity, sizeof(*unit->exprs)},
        .stmts = {unit->stmts, 0, stmtCapacity, sizeof(*unit->stmts)},
        .params = {unit->params, 0, paramCapacity, sizeof(*unit->params)},
        .fields = {unit->fields, 0, fieldCapacity, sizeof(*unit->fields)},
        .variants = {unit->variants, 0, variantCapacity, sizeof(*unit->variants)},
        .diagnostics = &compilation->diagnostics,
    };
    unit->module = ParseKekModule(&frontend, unit->ast, sourceFile);
    if (unit->module.errorCount > 0) {
        goto fail;
    }

    struct Token* docTokens = malloc(sizeof(*docTokens) * tokenCapacity);
    if (docTokens) {
        struct KekLexOptions docOptions = {0};
        docOptions.emitComments = 1;
        docOptions.diagnostics = &compilation->diagnostics;
        struct Tokenizer docTokenizer = CreateTokenizerWithOptions(unit->fileIndex, &compilation->fileTable, docOptions);
        struct TokenArray docTokenArray = TokenizeFile(&docTokenizer, docTokens, tokenCapacity);
        AttachKekDocComments(&unit->module, &docTokenArray, sourceFile);
        free(docTokens);
    }

    return 0;

fail:
    FreeKekCompilationUnit(unit);
    return -1;
}

int BuildKekCompilation(struct KekCompilation* compilation) {
    if (!compilation || compilation->entryFileIndex < 0) {
        return -1;
    }

    int result = 0;
    for (size_t i = 0; i < compilation->fileTable.count; i++) {
        if ((int)i == compilation->entryFileIndex) {
            continue;
        }
        struct KekCompilationUnit* unit = &compilation->units[compilation->unitCount];
        unit->fileIndex = (int)i;
        if (ParseUnit(compilation, unit) != 0) {
            result = -1;
            break;
        }
        compilation->modules[compilation->unitCount] = unit->module;
        compilation->unitCount++;
    }

    if (result == 0) {
        struct KekCompilationUnit* unit = &compilation->units[compilation->unitCount];
        unit->fileIndex = compilation->entryFileIndex;
        if (ParseUnit(compilation, unit) != 0) {
            result = -1;
        } else {
            compilation->modules[compilation->unitCount] = unit->module;
            compilation->unitCount++;
        }
    }

    if (result != 0) {
        return -1;
    }

    size_t symbolCapacity = 1;
    size_t scopeCapacity = 1 + compilation->unitCount;
    for (size_t i = 0; i < compilation->unitCount; i++) {
        symbolCapacity += compilation->modules[i].declCount * 2
            + compilation->modules[i].paramCount
            + compilation->modules[i].typedStmtCount
            + 64;
        scopeCapacity += compilation->modules[i].declKindCounts[KEK_DECL_FUNCTION]
            + compilation->modules[i].stmtKindCounts[KEK_STMT_BLOCK]
            + compilation->modules[i].stmtKindCounts[KEK_STMT_FOR]
            + compilation->modules[i].stmtKindCounts[KEK_STMT_EACH];
    }

    compilation->symbols = malloc(sizeof(*compilation->symbols) * symbolCapacity);
    compilation->scopes = malloc(sizeof(*compilation->scopes) * scopeCapacity);
    if (!compilation->symbols || !compilation->scopes) {
        KekAddDiagnostic(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_SEMANTIC,
            -1, (struct SourceLocation){0}, "could not allocate symbol storage");
        return -1;
    }

    compilation->program.symbols = compilation->symbols;
    compilation->program.symbolCapacity = symbolCapacity;
    compilation->program.scopes = compilation->scopes;
    compilation->program.scopeCapacity = scopeCapacity;
    compilation->program.diagnostics = &compilation->diagnostics;
    return BuildKekProgramSymbols(&compilation->program, compilation->modules, compilation->unitCount);
}

int WriteKekCompilationOutputs(struct KekCompilation* compilation, const char* cPath, const char* astJsonPath, const char* summaryPath) {
    if (!compilation || compilation->unitCount == 0) {
        return -1;
    }

    int result = WriteTypedCFileForModules(cPath, compilation->modules, compilation->unitCount);
    if (result != 0) {
        KekAddDiagnosticFormat(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_CODEGEN,
            -1, (struct SourceLocation){0}, "could not write C file %s", cPath);
        return result;
    }

    struct KekCompilationUnit* entryUnit = &compilation->units[compilation->unitCount - 1];
    result = WriteAstJsonFile(astJsonPath, entryUnit->ast, &compilation->fileTable.files[entryUnit->fileIndex]);
    if (result != 0) {
        KekAddDiagnosticFormat(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_CODEGEN,
            entryUnit->fileIndex, (struct SourceLocation){0}, "could not write AST JSON file %s", astJsonPath);
        return result;
    }

    result = WriteKekModuleSummaryFile(summaryPath, compilation->modules, compilation->unitCount, &compilation->program);
    if (result != 0) {
        KekAddDiagnosticFormat(&compilation->diagnostics, KEK_DIAGNOSTIC_ERROR, KEK_PHASE_CODEGEN,
            -1, (struct SourceLocation){0}, "could not write module summary file %s", summaryPath);
    }
    return result;
}

int CompileKekSmoke(const char* entryPath, const char* cPath, const char* astJsonPath, const char* summaryPath, struct KekCompilation* compilation) {
    if (LoadKekCompilation(compilation, entryPath) != 0) {
        return 1;
    }
    if (BuildKekCompilation(compilation) != 0) {
        return 1;
    }
    return WriteKekCompilationOutputs(compilation, cPath, astJsonPath, summaryPath) == 0 ? 0 : 1;
}

static int IsOnlyWhitespace(struct SourceFile* file, size_t start, size_t end) {
    if (!file || end > file->length || start > end) {
        return 0;
    }
    for (size_t i = start; i < end; i++) {
        if (!isspace((unsigned char)file->content[i])) {
            return 0;
        }
    }
    return 1;
}

void AttachKekDocComments(struct KekModule* module, struct TokenArray* tokens, struct SourceFile* file) {
    if (!module || !tokens || !file) {
        return;
    }

    for (struct KekDecl* decl = module->firstDecl; decl; decl = decl->next) {
        struct Token* lastDoc = NULL;
        size_t lastDocIndex = 0;
        for (size_t i = 0; i < tokens->count; i++) {
            struct Token* token = &tokens->items[i];
            if (token->location.offset >= decl->location.offset) {
                break;
            }
            if (token->type == TOKEN_DOC_COMMENT) {
                lastDoc = token;
                lastDocIndex = i;
            }
        }

        if (!lastDoc) {
            continue;
        }
        size_t lastEnd = lastDoc->location.offset + lastDoc->location.length;
        if (!IsOnlyWhitespace(file, lastEnd, decl->location.offset)) {
            continue;
        }

        size_t firstDocIndex = lastDocIndex;
        while (firstDocIndex > 0) {
            struct Token* previous = &tokens->items[firstDocIndex - 1];
            struct Token* current = &tokens->items[firstDocIndex];
            if (previous->type != TOKEN_DOC_COMMENT) {
                break;
            }
            size_t previousEnd = previous->location.offset + previous->location.length;
            if (!IsOnlyWhitespace(file, previousEnd, current->location.offset)) {
                break;
            }
            firstDocIndex--;
        }

        struct Token* firstDoc = &tokens->items[firstDocIndex];
        decl->hasDocComment = 1;
        decl->docCommentLocation = firstDoc->location;
        decl->docCommentLocation.length = lastEnd - firstDoc->location.offset;
    }
}
#undef IsOnlyWhitespace
#undef ParseUnit
#undef LoadImports
#undef LoadImportDirectory
#undef CompareImportNames
#undef StdlibImportRank
#undef FileAlreadyLoaded
#undef EndsWith
/* END compilation.c */

/* BEGIN codegen_c.c */
#define IsPunctuationToken codegen_c_IsPunctuationToken
#define IsOperatorToken codegen_c_IsOperatorToken
#define IsKeywordToken codegen_c_IsKeywordToken
#define IsTokenNode codegen_c_IsTokenNode
#define NextSibling codegen_c_NextSibling
#define DecodeCharLiteral codegen_c_DecodeCharLiteral
#define CTokenText codegen_c_CTokenText
#define TokenTextEquals codegen_c_TokenTextEquals
#define CopyTokenText codegen_c_CopyTokenText
#define OperatorMangleName codegen_c_OperatorMangleName
#define IsOperatorDeclName codegen_c_IsOperatorDeclName
#define TypedDeclBaseName codegen_c_TypedDeclBaseName
#define FindFunctionInfo codegen_c_FindFunctionInfo
#define AddFunctionInfo codegen_c_AddFunctionInfo
#define IsKnownStruct codegen_c_IsKnownStruct
#define FindStructInfo codegen_c_FindStructInfo
#define AddStructInfo codegen_c_AddStructInfo
#define IsCIdentifierStart codegen_c_IsCIdentifierStart
#define IsCIdentifierPart codegen_c_IsCIdentifierPart
#define RegisterExternCStructs codegen_c_RegisterExternCStructs
#define AddLocalTypeEx codegen_c_AddLocalTypeEx
#define FindLocalType codegen_c_FindLocalType
#define FindLocalIsPointer codegen_c_FindLocalIsPointer
#define WriteIndent codegen_c_WriteIndent
#define WritePrelude codegen_c_WritePrelude
#define PackagePrefixFromPath codegen_c_PackagePrefixFromPath
#define TypedNodeText codegen_c_TypedNodeText
#define CopyTypedNodeText codegen_c_CopyTypedNodeText
#define IsGenericNode codegen_c_IsGenericNode
#define TypedTokenTextEquals codegen_c_TypedTokenTextEquals
#define GenericArgNode codegen_c_GenericArgNode
#define GenericParamName codegen_c_GenericParamName
#define FindGenericSubstitution codegen_c_FindGenericSubstitution
#define AppendSanitized codegen_c_AppendSanitized
#define GenericArgTextFromFile codegen_c_GenericArgTextFromFile
#define GenericArgText codegen_c_GenericArgText
#define GenericArgMangleText codegen_c_GenericArgMangleText
#define MangleGenericNameWithFiles codegen_c_MangleGenericNameWithFiles
#define AppendGenericArgsToNameWithWriter codegen_c_AppendGenericArgsToNameWithWriter
#define MangleGenericName codegen_c_MangleGenericName
#define WriteSourceSlice codegen_c_WriteSourceSlice
#define CopyExprSource codegen_c_CopyExprSource
#define GenericParamIndex codegen_c_GenericParamIndex
#define TypeDefaultsToAggregate codegen_c_TypeDefaultsToAggregate
#define CopyTypedFieldDefaultWithContext codegen_c_CopyTypedFieldDefaultWithContext
#define CopyTypedFieldDefault codegen_c_CopyTypedFieldDefault
#define WriteTypedExpr codegen_c_WriteTypedExpr
#define FindGenericDecl codegen_c_FindGenericDecl
#define TypedDeclIsMethod codegen_c_TypedDeclIsMethod
#define TypedDeclReceiverName codegen_c_TypedDeclReceiverName
#define FindGenericInstance codegen_c_FindGenericInstance
#define FindGenericInstanceByName codegen_c_FindGenericInstanceByName
#define AddGenericStructInstance codegen_c_AddGenericStructInstance
#define AddGenericFunctionInstance codegen_c_AddGenericFunctionInstance
#define GenericStructInstanceName codegen_c_GenericStructInstanceName
#define AddGenericMethodInstancesForStructs codegen_c_AddGenericMethodInstancesForStructs
#define WriteTypedNonArrayTypeWithGenericContext codegen_c_WriteTypedNonArrayTypeWithGenericContext
#define WriteTypedBaseTypeWithGenericContext codegen_c_WriteTypedBaseTypeWithGenericContext
#define WriteTypedBaseType codegen_c_WriteTypedBaseType
#define WriteTypedArraySuffix codegen_c_WriteTypedArraySuffix
#define WriteTypedTypeAndNameWithGenericContext codegen_c_WriteTypedTypeAndNameWithGenericContext
#define WriteTypedTypeAndName codegen_c_WriteTypedTypeAndName
#define WriteTypedExprList codegen_c_WriteTypedExprList
#define ExprCalleeNameEquals codegen_c_ExprCalleeNameEquals
#define TypedCalleeName codegen_c_TypedCalleeName
#define TypedNamedArgument codegen_c_TypedNamedArgument
#define FindParameterIndex codegen_c_FindParameterIndex
#define ExprIsThisName codegen_c_ExprIsThisName
#define WriteTypedKnownCallArgs codegen_c_WriteTypedKnownCallArgs
#define WriteTypedCall codegen_c_WriteTypedCall
#define WriteTypedStructLiteral codegen_c_WriteTypedStructLiteral
#define TypedExprLocalType codegen_c_TypedExprLocalType
#define ExprOperatorMangle codegen_c_ExprOperatorMangle
#define TryWriteTypedUnaryOperatorCall codegen_c_TryWriteTypedUnaryOperatorCall
#define TryWriteTypedBinaryOperatorCall codegen_c_TryWriteTypedBinaryOperatorCall
#define WriteTypedStatement codegen_c_WriteTypedStatement
#define WriteTypedBlock codegen_c_WriteTypedBlock
#define FirstTypedBlockChild codegen_c_FirstTypedBlockChild
#define WriteDeferredStatement codegen_c_WriteDeferredStatement
#define WriteDeferredRange codegen_c_WriteDeferredRange
#define WriteAllActiveDefers codegen_c_WriteAllActiveDefers
#define WriteTypedDeclStatement codegen_c_WriteTypedDeclStatement
#define RegisterTypedStruct codegen_c_RegisterTypedStruct
#define RegisterTypedFunction codegen_c_RegisterTypedFunction
#define RegisterTypedFunctionWithName codegen_c_RegisterTypedFunctionWithName
#define TypedFunctionName codegen_c_TypedFunctionName
#define RegisterTypedDeclForCodegen codegen_c_RegisterTypedDeclForCodegen
#define AttributeListContains codegen_c_AttributeListContains
#define GetAlignedValue codegen_c_GetAlignedValue
#define TypedDeclHasAttribute codegen_c_TypedDeclHasAttribute
#define TypedDeclGetAlignedValue codegen_c_TypedDeclGetAlignedValue
#define WriteTypedParams codegen_c_WriteTypedParams
#define WriteTypedFunctionSignature codegen_c_WriteTypedFunctionSignature
#define WriteTypedGenericStructInstance codegen_c_WriteTypedGenericStructInstance
#define WriteTypedGenericFunctionSignature codegen_c_WriteTypedGenericFunctionSignature
#define WriteTypedGenericFunctionInstance codegen_c_WriteTypedGenericFunctionInstance
#define WriteTypedDecl codegen_c_WriteTypedDecl
#define CollectGenericTypeUse codegen_c_CollectGenericTypeUse
#define CollectGenericExprUses codegen_c_CollectGenericExprUses
#define CollectGenericStmtUses codegen_c_CollectGenericStmtUses
#define CollectGenericDeclUses codegen_c_CollectGenericDeclUses
#define IsTypedTypeDecl codegen_c_IsTypedTypeDecl

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
    int fieldIsAggregate[64];
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
    struct CFunctionInfo functions[256];
    size_t functionCount;
    struct CStructInfo structs[128];
    size_t structCount;
    struct CGenericInstance genericStructs[128];
    size_t genericStructCount;
    struct CGenericInstance genericFunctions[128];
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
    int eachIndexCounter;
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

static unsigned int DecodeCharLiteral(struct Token* token, struct SourceFile* file) {
    if (!token || !file || token->location.length < 2) {
        return 0;
    }

    size_t start = token->location.offset;
    size_t end = start + token->location.length;
    if (end > file->length || file->content[start] != '\'') {
        return 0;
    }

    size_t cursor = start + 1;
    if (cursor >= end) {
        return 0;
    }

    unsigned char value = (unsigned char)file->content[cursor];
    if (value == '\\' && cursor + 1 < end) {
        cursor++;
        switch (file->content[cursor]) {
            case '0':
                value = '\0';
                break;
            case 'n':
                value = '\n';
                break;
            case 'r':
                value = '\r';
                break;
            case 't':
                value = '\t';
                break;
            case '\'':
                value = '\'';
                break;
            case '"':
                value = '"';
                break;
            case '\\':
                value = '\\';
                break;
            default:
                value = (unsigned char)file->content[cursor];
                break;
        }
    }
    return (unsigned int)value;
}

static const char* CTokenText(struct Token* token, struct SourceFile* file, char* buffer, size_t bufferSize) {
    if (token->type == TOKEN_CHAR) {
        snprintf(buffer, bufferSize, "%u", DecodeCharLiteral(token, file));
        return buffer;
    }

    if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_NUMBER || token->type == TOKEN_STRING) {
        size_t length = token->location.length;
        if (length >= bufferSize) {
            length = bufferSize - 1;
        }
        memcpy(buffer, file->content + token->location.offset, length);
        buffer[length] = '\0';

        // Handle number literal normalization for C compatibility
        if (token->type == TOKEN_NUMBER && length > 0) {
            // Strip underscores from numeric literals
            char* src = buffer;
            char* dst = buffer;
            while (*src) {
                if (*src != '_') {
                    *dst++ = *src;
                }
                src++;
            }
            *dst = '\0';

            // Convert binary literals (0b...) to hex for C11 compatibility
            if (buffer[0] == '0' && (buffer[1] == 'b' || buffer[1] == 'B')) {
                unsigned long long value = 0;
                char* p = buffer + 2;
                while (*p == '0' || *p == '1') {
                    value = (value << 1) | (*p - '0');
                    p++;
                }
                snprintf(buffer, bufferSize, "0x%llX", value);
            }
        }

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
    const char* lexeme = TokenLexeme(token, file);
    return lexeme && strlen(lexeme) == length && strcmp(lexeme, text) == 0;
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
    fputs("typedef void* ptr;\n", out);
    fputs("typedef const char* str;\n\n", out);
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

static int GenericParamIndex(struct CWriter* writer, struct AstNode* params, struct AstNode* name) {
    if (!name || !IsGenericNode(params)) {
        return -1;
    }

    int index = 0;
    for (struct AstNode* param = params->firstChild; param; param = param->nextSibling, index++) {
        struct AstNode* paramName = GenericParamName(params, (size_t)index);
        char buffer[128];
        if (paramName && TypedTokenTextEquals(writer, name, TypedNodeText(writer, paramName, buffer, sizeof(buffer)))) {
            return index;
        }
    }

    return -1;
}

static int TypeDefaultsToAggregate(struct CWriter* writer, struct KekType* type, struct AstNode* params, struct AstNode* args, struct SourceFile* argsFile) {
    if (!type) {
        return 0;
    }
    if (type->kind == KEK_TYPE_ARRAY) {
        return 1;
    }
    if (type->kind == KEK_TYPE_POINTER) {
        return 0;
    }

    struct KekType* base = type;
    while (base && base->kind == KEK_TYPE_ARRAY) {
        base = base->element;
    }
    if (!base || !base->name) {
        return 0;
    }

    int paramIndex = GenericParamIndex(writer, params, base->name);
    if (paramIndex >= 0) {
        struct AstNode* arg = GenericArgNode(args, (size_t)paramIndex);
        if (!arg) {
            return 0;
        }
        char argName[128];
        if (arg && IsTokenNode(arg) && IsGenericNode(arg->nextSibling)) {
            MangleGenericNameWithFiles(writer, argsFile, arg, argsFile, arg->nextSibling, argName, sizeof(argName));
        } else {
            GenericArgTextFromFile(argsFile ? argsFile : writer->file, arg, argName, sizeof(argName));
        }
        return IsKnownStruct(writer, argName);
    }

    char typeName[128];
    CopyTypedNodeText(writer, base->name, typeName, sizeof(typeName));
    return IsKnownStruct(writer, typeName);
}

static void CopyTypedFieldDefaultWithContext(struct CWriter* writer, struct KekField* field, struct AstNode* params, struct AstNode* args, struct SourceFile* argsFile, char* buffer, size_t bufferSize, int* isAggregate) {
    if (isAggregate) {
        *isAggregate = 0;
    }
    if (field && field->isNestedStruct) {
        if (isAggregate) {
            *isAggregate = 1;
        }
        snprintf(buffer, bufferSize, "{0}");
        return;
    }

    if (field && field->defaultValue) {
        CopyExprSource(writer, field->defaultValue, buffer, bufferSize);
        return;
    }

    int aggregate = TypeDefaultsToAggregate(writer, field ? field->type : NULL, params, args, argsFile);
    if (isAggregate) {
        *isAggregate = aggregate;
    }
    snprintf(buffer, bufferSize, "%s", aggregate ? "{0}" : "0");
}

static void CopyTypedFieldDefault(struct CWriter* writer, struct KekField* field, char* buffer, size_t bufferSize, int* isAggregate) {
    CopyTypedFieldDefaultWithContext(writer, field, NULL, NULL, NULL, buffer, bufferSize, isAggregate);
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
            CopyTypedFieldDefaultWithContext(writer,
                field,
                decl->genericParams,
                args,
                argsFile,
                info->defaults[info->fieldCount],
                sizeof(info->defaults[info->fieldCount]),
                &info->fieldIsAggregate[info->fieldCount]);
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

static int ExprIsThisName(struct CWriter* writer, struct KekExpr* expr) {
    return expr
        && expr->kind == KEK_EXPR_NAME
        && expr->token
        && TokenTextEquals(&expr->token->token, writer->file, "this");
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
            if (i == 0 && implicitThis && (implicitThisIsPointer || (writer->thisIsPointer && ExprIsThisName(writer, implicitThis)))) {
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
                if (!(writer->thisIsPointer && ExprIsThisName(writer, expr->callee->left))) {
                    fputc('&', writer->out);
                }
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
        case KEK_EXPR_SIZEOF:
            fputs("sizeof(", writer->out);
            if (expr->type) {
                WriteTypedBaseType(writer, expr->type);
            } else if (expr->right) {
                WriteTypedExpr(writer, expr->right);
            }
            fputc(')', writer->out);
            break;
        case KEK_EXPR_ALIGNOF:
            fputs("_Alignof(", writer->out);
            if (expr->type) {
                WriteTypedBaseType(writer, expr->type);
            }
            fputc(')', writer->out);
            break;
        case KEK_EXPR_OFFSETOF:
            fputs("offsetof(", writer->out);
            if (expr->type) {
                WriteTypedBaseType(writer, expr->type);
            }
            fputc(',', writer->out);
            if (expr->right) {
                WriteTypedExpr(writer, expr->right);
            }
            fputc(')', writer->out);
            break;
        case KEK_EXPR_LEN:
            fputs("(sizeof(", writer->out);
            if (expr->right) {
                WriteTypedExpr(writer, expr->right);
            }
            fputs(")/sizeof((", writer->out);
            if (expr->right) {
                WriteTypedExpr(writer, expr->right);
            }
            fputs(")[0]))", writer->out);
            break;
        case KEK_EXPR_RANGE:
            // Range expressions are primarily handled inline by KEK_STMT_EACH codegen.
            // If range appears in other contexts (e.g., array init), emit source for now.
            if (expr->source) {
                WriteSourceSlice(writer->out, writer->file, expr->source->location);
            }
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
        case KEK_STMT_EACH: {
            // each<Type:val>(arr){ body } or each<IndexType:i, Type:val>(arr){ body }
            char idxName[64];
            char valName[64];

            // Determine index variable name
            if (stmt->indexName) {
                // User provided index variable
                CopyTypedNodeText(writer, stmt->indexName, idxName, sizeof(idxName));
            } else {
                // Generate hidden index variable
                snprintf(idxName, sizeof(idxName), "__kek_each_idx_%d", writer->eachIndexCounter++);
            }

            // Get value variable name
            CopyTypedNodeText(writer, stmt->declName, valName, sizeof(valName));

            // Check if iterating over a range expression
            if (stmt->expr && stmt->expr->kind == KEK_EXPR_RANGE) {
                // Optimized: direct for loop over range
                // for (Type var = start; var < end; var += step) { body }
                struct KekExpr* range = stmt->expr;

                fputs("for (", writer->out);
                WriteTypedBaseType(writer, stmt->declType);
                fprintf(writer->out, " %s = ", valName);
                WriteTypedExpr(writer, range->left);  // start
                fprintf(writer->out, "; %s < ", valName);
                WriteTypedExpr(writer, range->right); // end
                fprintf(writer->out, "; %s += ", valName);
                if (range->step) {
                    WriteTypedExpr(writer, range->step);
                } else {
                    fputc('1', writer->out);
                }
                fputs(") ", writer->out);
                WriteTypedBlock(writer, FirstTypedBlockChild(stmt));
            } else {
                // Array iteration
                // for (IndexType idx = 0; idx < (sizeof(arr)/sizeof(arr[0])); idx++) {
                //     Type val = arr[idx];
                //     body
                // }
                fputs("for (", writer->out);
                if (stmt->indexType) {
                    WriteTypedBaseType(writer, stmt->indexType);
                } else {
                    fputs("size_t", writer->out);
                }
                fprintf(writer->out, " %s = 0; %s < (sizeof(", idxName, idxName);
                WriteTypedExpr(writer, stmt->expr);
                fputs(")/sizeof((", writer->out);
                WriteTypedExpr(writer, stmt->expr);
                fprintf(writer->out, ")[0])); %s++) {\n", idxName);
                writer->indent++;

                // Declare value variable at the top of the block
                WriteIndent(writer->out, writer->indent);
                WriteTypedBaseType(writer, stmt->declType);
                fprintf(writer->out, " %s = (", valName);
                WriteTypedExpr(writer, stmt->expr);
                fprintf(writer->out, ")[%s];\n", idxName);

                // Write body statements from the block
                struct KekStmt* block = FirstTypedBlockChild(stmt);
                if (block) {
                    for (struct KekStmt* child = block->firstChild; child; child = child->next) {
                        WriteIndent(writer->out, writer->indent);
                        WriteTypedStatement(writer, child);
                        fputc('\n', writer->out);
                    }
                }

                writer->indent--;
                WriteIndent(writer->out, writer->indent);
                fputc('}', writer->out);
            }
            break;
        }
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
            if (FirstTypedBlockChild(stmt)) {
                fputc(' ', writer->out);
                WriteTypedBlock(writer, FirstTypedBlockChild(stmt));
            } else if (stmt->expr) {
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
        case KEK_STMT_UNREACHABLE:
            fputs("__builtin_unreachable();", writer->out);
            break;
        case KEK_STMT_PANIC:
            fputs("do { fprintf(stderr, \"panic: %s\\n\", ", writer->out);
            if (stmt->expr) {
                WriteTypedExpr(writer, stmt->expr);
            } else {
                fputs("\"\"", writer->out);
            }
            fputs("); abort(); } while(0);", writer->out);
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
        CopyTypedFieldDefault(writer,
            field,
            info->defaults[info->fieldCount],
            sizeof(info->defaults[info->fieldCount]),
            &info->fieldIsAggregate[info->fieldCount]);
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

static int GetAlignedValue(struct CWriter* writer, struct AstNode* attributes) {
    if (!attributes || attributes->type != AST_INDEX) {
        return 0;
    }

    for (struct AstNode* statement = attributes->firstChild; statement; statement = statement->nextSibling) {
        struct AstNode* first = statement->firstChild;
        if (!first || !IsTokenNode(first)) {
            continue;
        }
        char buffer[256];
        const char* text = CTokenText(&first->token, writer->file, buffer, sizeof(buffer));
        if (strcmp(text, "aligned") == 0) {
            struct AstNode* group = first->nextSibling;
            if (group && group->type == AST_GROUP && group->firstChild && group->firstChild->firstChild) {
                struct AstNode* valueNode = group->firstChild->firstChild;
                if (IsTokenNode(valueNode) && valueNode->token.type == TOKEN_NUMBER) {
                    CopyTokenText(&valueNode->token, writer->file, buffer, sizeof(buffer));
                    return atoi(buffer);
                }
            }
        }
    }

    return 0;
}

static int TypedDeclHasAttribute(struct CWriter* writer, struct KekDecl* decl, const char* name) {
    struct AstNode* first = decl && decl->source ? decl->source->firstChild : NULL;
    return first && first->type == AST_INDEX && AttributeListContains(writer, first, name);
}

static int TypedDeclGetAlignedValue(struct CWriter* writer, struct KekDecl* decl) {
    struct AstNode* first = decl && decl->source ? decl->source->firstChild : NULL;
    if (first && first->type == AST_INDEX) {
        return GetAlignedValue(writer, first);
    }
    return 0;
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
                fputs("#if defined(__GNUC__) || defined(__clang__)\n", writer->out);
                fputs("#pragma GCC diagnostic push\n", writer->out);
                fputs("#pragma GCC diagnostic ignored \"-Wunused-function\"\n", writer->out);
                fputs("#endif\n", writer->out);
                WriteSourceSlice(writer->out, writer->file, body);
                fputs("\n#if defined(__GNUC__) || defined(__clang__)\n", writer->out);
                fputs("#pragma GCC diagnostic pop\n", writer->out);
                fputs("#endif\n", writer->out);
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
            {
                int isPacked = TypedDeclHasAttribute(writer, decl, "packed");
                int alignedValue = TypedDeclGetAlignedValue(writer, decl);
                if (isPacked || alignedValue > 0) {
                    fputs(" __attribute__((", writer->out);
                    if (isPacked) {
                        fputs("packed", writer->out);
                        if (alignedValue > 0) {
                            fputc(',', writer->out);
                        }
                    }
                    if (alignedValue > 0) {
                        fprintf(writer->out, "aligned(%d)", alignedValue);
                    }
                    fputs("))", writer->out);
                }
            }
            fputc(' ', writer->out);
            fputs(TypedNodeText(writer, decl->name, name, sizeof(name)), writer->out);
            fputs(" {\n", writer->out);
            writer->indent++;
            for (struct KekField* field = decl->firstField; field; field = field->next) {
                WriteIndent(writer->out, writer->indent);
                if (field->isNestedStruct) {
                    // Nested struct: emit inline struct definition
                    char nestedName[128];
                    fputs("struct {\n", writer->out);
                    writer->indent++;
                    for (struct KekField* nested = field->nestedFields; nested; nested = nested->next) {
                        WriteIndent(writer->out, writer->indent);
                        WriteTypedTypeAndName(writer, nested->type, nested->name);
                        fputs(";\n", writer->out);
                    }
                    writer->indent--;
                    WriteIndent(writer->out, writer->indent);
                    fprintf(writer->out, "} %s;\n", TypedNodeText(writer, field->name, nestedName, sizeof(nestedName)));
                } else {
                    WriteTypedTypeAndName(writer, field->type, field->name);
                    fputs(";\n", writer->out);
                }
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
#undef IsTypedTypeDecl
#undef CollectGenericDeclUses
#undef CollectGenericStmtUses
#undef CollectGenericExprUses
#undef CollectGenericTypeUse
#undef WriteTypedDecl
#undef WriteTypedGenericFunctionInstance
#undef WriteTypedGenericFunctionSignature
#undef WriteTypedGenericStructInstance
#undef WriteTypedFunctionSignature
#undef WriteTypedParams
#undef TypedDeclGetAlignedValue
#undef TypedDeclHasAttribute
#undef GetAlignedValue
#undef AttributeListContains
#undef RegisterTypedDeclForCodegen
#undef TypedFunctionName
#undef RegisterTypedFunctionWithName
#undef RegisterTypedFunction
#undef RegisterTypedStruct
#undef WriteTypedDeclStatement
#undef WriteAllActiveDefers
#undef WriteDeferredRange
#undef WriteDeferredStatement
#undef FirstTypedBlockChild
#undef WriteTypedBlock
#undef WriteTypedStatement
#undef TryWriteTypedBinaryOperatorCall
#undef TryWriteTypedUnaryOperatorCall
#undef ExprOperatorMangle
#undef TypedExprLocalType
#undef WriteTypedStructLiteral
#undef WriteTypedCall
#undef WriteTypedKnownCallArgs
#undef ExprIsThisName
#undef FindParameterIndex
#undef TypedNamedArgument
#undef TypedCalleeName
#undef ExprCalleeNameEquals
#undef WriteTypedExprList
#undef WriteTypedTypeAndName
#undef WriteTypedTypeAndNameWithGenericContext
#undef WriteTypedArraySuffix
#undef WriteTypedBaseType
#undef WriteTypedBaseTypeWithGenericContext
#undef WriteTypedNonArrayTypeWithGenericContext
#undef AddGenericMethodInstancesForStructs
#undef GenericStructInstanceName
#undef AddGenericFunctionInstance
#undef AddGenericStructInstance
#undef FindGenericInstanceByName
#undef FindGenericInstance
#undef TypedDeclReceiverName
#undef TypedDeclIsMethod
#undef FindGenericDecl
#undef WriteTypedExpr
#undef CopyTypedFieldDefault
#undef CopyTypedFieldDefaultWithContext
#undef TypeDefaultsToAggregate
#undef GenericParamIndex
#undef CopyExprSource
#undef WriteSourceSlice
#undef MangleGenericName
#undef AppendGenericArgsToNameWithWriter
#undef MangleGenericNameWithFiles
#undef GenericArgMangleText
#undef GenericArgText
#undef GenericArgTextFromFile
#undef AppendSanitized
#undef FindGenericSubstitution
#undef GenericParamName
#undef GenericArgNode
#undef TypedTokenTextEquals
#undef IsGenericNode
#undef CopyTypedNodeText
#undef TypedNodeText
#undef PackagePrefixFromPath
#undef WritePrelude
#undef WriteIndent
#undef FindLocalIsPointer
#undef FindLocalType
#undef AddLocalTypeEx
#undef RegisterExternCStructs
#undef IsCIdentifierPart
#undef IsCIdentifierStart
#undef AddStructInfo
#undef FindStructInfo
#undef IsKnownStruct
#undef AddFunctionInfo
#undef FindFunctionInfo
#undef TypedDeclBaseName
#undef IsOperatorDeclName
#undef OperatorMangleName
#undef CopyTokenText
#undef TokenTextEquals
#undef CTokenText
#undef DecodeCharLiteral
#undef NextSibling
#undef IsTokenNode
#undef IsKeywordToken
#undef IsOperatorToken
#undef IsPunctuationToken
/* END codegen_c.c */

/* BEGIN main.c */
#define ArgEquals main_ArgEquals
#define CliPrintUsage main_CliPrintUsage
#define CliCopyDefaultPath main_CliCopyDefaultPath
#define CliSetOutDirDefaults main_CliSetOutDirDefaults
#define CliEnsureOutDir main_CliEnsureOutDir
#define CliReadValue main_CliReadValue
#define ParseKekBuildCli main_ParseKekBuildCli
#define ParseKekCli main_ParseKekCli
#define RunKekBuild main_RunKekBuild


static const char* SMOKE_SOURCE_PATH = "tmp.kek";
static const char* DEFAULT_OUT_DIR = "out";

enum KekCliCommand {
    KEK_CLI_COMMAND_BUILD,
    KEK_CLI_COMMAND_HELP,
    KEK_CLI_COMMAND_VERSION,
};

struct KekCliOptions {
    enum KekCliCommand command;
    const char* inputPath;
    const char* cPath;
    const char* astJsonPath;
    const char* summaryPath;
    const char* outDir;
    char cPathStorage[MAX_PATH_LENGTH];
    char astJsonPathStorage[MAX_PATH_LENGTH];
    char summaryPathStorage[MAX_PATH_LENGTH];
};

static int ArgEquals(const char* arg, const char* text) {
    return arg && text && strcmp(arg, text) == 0;
}

static void CliPrintUsage(FILE* out) {
    fprintf(out,
        "Usage:\n"
        "  kek build <input.kek> [options]\n"
        "  kek --help\n"
        "  kek --version\n"
        "\n"
        "Options:\n"
        "  -o <path>              Write generated C to path\n"
        "  --ast-json <path>      Write structural AST JSON to path\n"
        "  --summary <path>       Write typed module summary to path\n"
        "  --out-dir <dir>        Write default outputs under dir\n");
}

static int CliCopyDefaultPath(char* out, size_t outSize, const char* outDir, const char* fileName) {
    int written = snprintf(out, outSize, "%s/%s", outDir, fileName);
    return written >= 0 && (size_t)written < outSize ? 0 : -1;
}

static int CliSetOutDirDefaults(struct KekCliOptions* options) {
    const char* outDir = options->outDir ? options->outDir : DEFAULT_OUT_DIR;
    if (!options->cPath) {
        if (CliCopyDefaultPath(options->cPathStorage, sizeof(options->cPathStorage), outDir, "out.c") != 0) {
            fprintf(stderr, "kek: C output path is too long\n");
            return -1;
        }
        options->cPath = options->cPathStorage;
    }
    if (!options->astJsonPath) {
        if (CliCopyDefaultPath(options->astJsonPathStorage, sizeof(options->astJsonPathStorage), outDir, "ast.json") != 0) {
            fprintf(stderr, "kek: AST JSON output path is too long\n");
            return -1;
        }
        options->astJsonPath = options->astJsonPathStorage;
    }
    if (!options->summaryPath) {
        if (CliCopyDefaultPath(options->summaryPathStorage, sizeof(options->summaryPathStorage), outDir, "module.txt") != 0) {
            fprintf(stderr, "kek: module summary output path is too long\n");
            return -1;
        }
        options->summaryPath = options->summaryPathStorage;
    }
    return 0;
}

static int CliEnsureOutDir(const char* path) {
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "kek: could not create output directory %s: %s\n", path, strerror(errno));
        return -1;
    }
    return 0;
}

static int CliReadValue(int argc, char** argv, int* index, const char* flag, const char** out) {
    if (*index + 1 >= argc) {
        fprintf(stderr, "kek: missing value for %s\n", flag);
        return -1;
    }
    *index += 1;
    *out = argv[*index];
    return 0;
}

static int ParseKekBuildCli(int argc, char** argv, struct KekCliOptions* options) {
    if (argc < 3) {
        fprintf(stderr, "kek: missing input path\n");
        return -1;
    }

    for (int i = 2; i < argc; i++) {
        const char* arg = argv[i];
        if (ArgEquals(arg, "--help") || ArgEquals(arg, "-h")) {
            options->command = KEK_CLI_COMMAND_HELP;
            return 0;
        }
        if (ArgEquals(arg, "-o")) {
            if (CliReadValue(argc, argv, &i, arg, &options->cPath) != 0) {
                return -1;
            }
        } else if (ArgEquals(arg, "--ast-json")) {
            if (CliReadValue(argc, argv, &i, arg, &options->astJsonPath) != 0) {
                return -1;
            }
        } else if (ArgEquals(arg, "--summary")) {
            if (CliReadValue(argc, argv, &i, arg, &options->summaryPath) != 0) {
                return -1;
            }
        } else if (ArgEquals(arg, "--out-dir")) {
            if (CliReadValue(argc, argv, &i, arg, &options->outDir) != 0) {
                return -1;
            }
        } else if (arg[0] == '-') {
            fprintf(stderr, "kek: unknown flag: %s\n", arg);
            return -1;
        } else if (!options->inputPath) {
            options->inputPath = arg;
        } else {
            fprintf(stderr, "kek: unexpected argument: %s\n", arg);
            return -1;
        }
    }

    if (!options->inputPath) {
        fprintf(stderr, "kek: missing input path\n");
        return -1;
    }
    return CliSetOutDirDefaults(options);
}

static int ParseKekCli(int argc, char** argv, struct KekCliOptions* options) {
    memset(options, 0, sizeof(*options));
    options->command = KEK_CLI_COMMAND_BUILD;
    options->outDir = DEFAULT_OUT_DIR;

    if (argc == 1) {
        options->inputPath = SMOKE_SOURCE_PATH;
        return CliSetOutDirDefaults(options);
    }
    if (ArgEquals(argv[1], "--help") || ArgEquals(argv[1], "-h")) {
        options->command = KEK_CLI_COMMAND_HELP;
        return 0;
    }
    if (ArgEquals(argv[1], "--version")) {
        options->command = KEK_CLI_COMMAND_VERSION;
        return 0;
    }
    if (ArgEquals(argv[1], "build")) {
        return ParseKekBuildCli(argc, argv, options);
    }

    fprintf(stderr, "kek: unknown command: %s\n", argv[1]);
    return -1;
}

static int RunKekBuild(struct KekCliOptions* options) {
    const char* outDir = options->outDir ? options->outDir : DEFAULT_OUT_DIR;
    if (CliEnsureOutDir(outDir) != 0) {
        return 1;
    }

    struct KekDiagnostic diagnostics[256];
    struct KekCompilation compilation;
    InitKekCompilation(&compilation, diagnostics, sizeof(diagnostics) / sizeof(diagnostics[0]));

    int result = CompileKekSmoke(options->inputPath, options->cPath, options->astJsonPath, options->summaryPath, &compilation);
    PrintKekDiagnostics(stderr, &compilation.diagnostics, &compilation.fileTable);
    FreeKekCompilation(&compilation);
    return result;
}

int main(int argc, char** argv) {
    struct KekCliOptions options;
    if (ParseKekCli(argc, argv, &options) != 0) {
        CliPrintUsage(stderr);
        return 1;
    }

    if (options.command == KEK_CLI_COMMAND_HELP) {
        CliPrintUsage(stdout);
        return 0;
    }
    if (options.command == KEK_CLI_COMMAND_VERSION) {
        printf("%s\n", VERSION);
        return 0;
    }

    return RunKekBuild(&options);
}
#undef RunKekBuild
#undef ParseKekCli
#undef ParseKekBuildCli
#undef CliReadValue
#undef CliEnsureOutDir
#undef CliSetOutDirDefaults
#undef CliCopyDefaultPath
#undef CliPrintUsage
#undef ArgEquals
/* END main.c */
