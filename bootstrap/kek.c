#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef void* ptr;
typedef const char* str;

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include <stdlib.h>
#include <string.h>

static void* kek_std_alloc(size_t size) {
	return malloc(size);
}

static void* kek_std_resize(void* oldData, size_t size) {
	return realloc(oldData, size);
}

static void kek_std_free(void* data) {
	free(data);
}

static void* kek_std_mem_copy(void* dest, const void* src, size_t count) {
	return memcpy(dest, src, count);
}

static void* kek_std_mem_set(void* dest, int value, size_t count) {
	return memset(dest, value, count);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include <stdio.h>

static FILE* kek_std_file_open(const char* path, const char* mode) {
	return fopen(path, mode);
}

static size_t kek_std_file_read(void* data, size_t size, size_t count, FILE* file) {
	return fread(data, size, count, file);
}

static size_t kek_std_file_write(const void* data, size_t size, size_t count, FILE* file) {
	return fwrite(data, size, count, file);
}

static int kek_std_file_flush(FILE* file) {
	return fflush(file);
}

static int kek_std_file_close(FILE* file) {
	return fclose(file);
}

static FILE* kek_std_stdin(void) {
	return stdin;
}

static FILE* kek_std_stdout(void) {
	return stdout;
}

static FILE* kek_std_stderr(void) {
	return stderr;
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include <dirent.h>

static DIR* kek_std_dir_open(const char* path) {
	return opendir(path);
}

static const char* kek_std_dir_read_name(DIR* dir) {
	struct dirent* entry = readdir(dir);
	if (!entry) {
		return 0;
	}
	return entry->d_name;
}

static int kek_std_dir_close(DIR* dir) {
	return closedir(dir);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include <stdlib.h>

static int kek_std_system(const char* command) {
	return system(command);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include <pthread.h>
#include <stdint.h>

static int kek_std_thread_start(uint64_t* handle, void* entry, void* arg) {
	pthread_t thread;
	int result = pthread_create(&thread, 0, (void* (*)(void*))entry, arg);
	if (result != 0) {
		return result;
	}
	*handle = (uint64_t)(uintptr_t)thread;
	return 0;
}

static int kek_std_thread_join(uint64_t handle) {
	return pthread_join((pthread_t)(uintptr_t)handle, 0);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
typedef u8 byte;
typedef u64 usize;
typedef ptr RawHandle;
typedef enum Status {
    Status_Ok,
    Status_End,
    Status_Invalid,
    Status_NoMemory,
    Status_NotFound,
    Status_PermissionDenied,
    Status_Interrupted,
    Status_Unsupported,
    Status_IoError,
} Status;
struct Allocator {
    ptr context;
};
struct String {
    byte* data;
    usize len;
};
struct OwnedString {
    byte* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct StringBuilder {
    byte* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct ByteCursor {
    struct String input;
    usize pos;
    usize line;
    usize column;
};
struct MemoryReader {
    byte* data;
    usize len;
    usize pos;
};
struct MemoryWriter {
    byte* data;
    usize len;
    usize pos;
};
typedef enum FileMode {
    FileMode_Read,
    FileMode_Write,
    FileMode_Append,
    FileMode_ReadWrite,
} FileMode;
struct File {
    RawHandle handle;
    bool owned;
};
struct Directory {
    RawHandle handle;
};
struct TemplateArg {
    struct String name;
    struct String value;
};
struct TemplateContext {
    struct TemplateArg* named;
    usize namedLen;
    struct String* positional;
    usize positionalLen;
};
typedef enum DiagnosticSeverity {
    DiagnosticSeverity_Note,
    DiagnosticSeverity_Warning,
    DiagnosticSeverity_Error,
} DiagnosticSeverity;
typedef enum DiagnosticPhase {
    DiagnosticPhase_Source,
    DiagnosticPhase_Lex,
    DiagnosticPhase_Parse,
    DiagnosticPhase_TypedParse,
    DiagnosticPhase_Semantic,
    DiagnosticPhase_Codegen,
} DiagnosticPhase;
struct SourceLocation {
    usize line;
    usize column;
    usize offset;
    usize length;
};
struct Diagnostic {
    DiagnosticSeverity severity;
    DiagnosticPhase phase;
    i64 fileIndex;
    struct SourceLocation location;
    struct OwnedString message;
};
struct Array__Diagnostic {
    struct Diagnostic* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct DiagnosticBag {
    struct Array__Diagnostic items;
    usize errorCount;
};
struct SourceFile {
    struct OwnedString path;
    struct OwnedString content;
    usize fileIndex;
};
struct Array__SourceFile {
    struct SourceFile* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct FileTable {
    struct Array__SourceFile files;
    struct OwnedString importRoot;
    bool hasImportRoot;
};
typedef enum TokenKind {
    TokenKind_Eof,
    TokenKind_Identifier,
    TokenKind_Number,
    TokenKind_String,
    TokenKind_Char,
    TokenKind_Comment,
    TokenKind_DocComment,
    TokenKind_Operator,
    TokenKind_Keyword,
    TokenKind_Punctuation,
} TokenKind;
typedef enum OperatorKind {
    OperatorKind_Scope,
    OperatorKind_Equal,
    OperatorKind_NotEqual,
    OperatorKind_LessEqual,
    OperatorKind_GreaterEqual,
    OperatorKind_LogicalAnd,
    OperatorKind_LogicalOr,
    OperatorKind_PlusAssign,
    OperatorKind_MinusAssign,
    OperatorKind_Arrow,
    OperatorKind_Plus,
    OperatorKind_Minus,
    OperatorKind_Multiply,
    OperatorKind_Divide,
    OperatorKind_Modulo,
    OperatorKind_Assign,
    OperatorKind_Less,
    OperatorKind_Greater,
    OperatorKind_LogicalNot,
    OperatorKind_BitwiseAnd,
    OperatorKind_BitwiseOr,
    OperatorKind_BitwiseNot,
} OperatorKind;
typedef enum KeywordKind {
    KeywordKind_If,
    KeywordKind_Else,
    KeywordKind_While,
    KeywordKind_For,
    KeywordKind_Return,
    KeywordKind_Do,
    KeywordKind_Break,
    KeywordKind_Continue,
    KeywordKind_Using,
    KeywordKind_Alias,
    KeywordKind_Export,
    KeywordKind_Extern,
    KeywordKind_Enum,
    KeywordKind_Struct,
    KeywordKind_Union,
    KeywordKind_Switch,
    KeywordKind_Select,
    KeywordKind_Case,
    KeywordKind_Default,
    KeywordKind_Each,
    KeywordKind_Packed,
    KeywordKind_Aligned,
    KeywordKind_Comptime,
    KeywordKind_Defer,
    KeywordKind_Tagged,
    KeywordKind_True,
    KeywordKind_False,
    KeywordKind_Unreachable,
    KeywordKind_Panic,
} KeywordKind;
typedef enum PunctuationKind {
    PunctuationKind_LeftParen,
    PunctuationKind_RightParen,
    PunctuationKind_LeftBrace,
    PunctuationKind_RightBrace,
    PunctuationKind_LeftBracket,
    PunctuationKind_RightBracket,
    PunctuationKind_Semicolon,
    PunctuationKind_Comma,
    PunctuationKind_Colon,
    PunctuationKind_Dot,
    PunctuationKind_Hash,
} PunctuationKind;
typedef enum TokenPayloadTag {
    TokenPayloadTag_None,
    TokenPayloadTag_Operator,
    TokenPayloadTag_Keyword,
    TokenPayloadTag_Punctuation,
} TokenPayloadTag;
typedef struct TokenPayload {
    TokenPayloadTag tag;
    union {
    u64 None;
    OperatorKind Operator;
    KeywordKind Keyword;
    PunctuationKind Punctuation;
    } data;
} TokenPayload;
static inline TokenPayload TokenPayload_None(u64 value) {
    TokenPayload out;
    out.tag = TokenPayloadTag_None;
    out.data.None = value;
    return out;
}
static inline TokenPayload TokenPayload_Operator(OperatorKind value) {
    TokenPayload out;
    out.tag = TokenPayloadTag_Operator;
    out.data.Operator = value;
    return out;
}
static inline TokenPayload TokenPayload_Keyword(KeywordKind value) {
    TokenPayload out;
    out.tag = TokenPayloadTag_Keyword;
    out.data.Keyword = value;
    return out;
}
static inline TokenPayload TokenPayload_Punctuation(PunctuationKind value) {
    TokenPayload out;
    out.tag = TokenPayloadTag_Punctuation;
    out.data.Punctuation = value;
    return out;
}
struct Token {
    TokenKind kind;
    u64 subkind;
    TokenPayload payload;
    usize line;
    usize column;
    usize offset;
    usize length;
};
struct Tokenizer {
    struct ByteCursor cursor;
    bool emitComments;
};
typedef enum AstKind {
    AstKind_File,
    AstKind_Statement,
    AstKind_Block,
    AstKind_Group,
    AstKind_Index,
    AstKind_Generic,
    AstKind_Token,
} AstKind;
struct AstNode {
    AstKind kind;
    struct SourceLocation location;
    struct Token token;
    usize firstChild;
    usize lastChild;
    usize nextSibling;
    usize childCount;
};
struct Array__AstNode {
    struct AstNode* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct AstTree {
    struct Array__AstNode nodes;
    usize root;
};
struct Parser {
    struct Token* tokens;
    usize count;
    usize position;
    struct String source;
    i64 fileIndex;
    struct DiagnosticBag* diagnostics;
    struct AstTree tree;
    usize errorCount;
};
struct CodeWriter {
    struct StringBuilder* out;
    Status status;
};
struct Local {
    struct OwnedString name;
    struct OwnedString typeKey;
    struct OwnedString cType;
    struct OwnedString arrayLen;
    bool isArray;
    bool isPointer;
};
struct Array__Local {
    struct Local* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct CodegenEnv {
    struct Array__Local locals;
    struct OwnedString returnTypeKey;
    struct OwnedString returnCType;
    struct OwnedString thisTypeKey;
    struct OwnedString thisCType;
    struct ModuleDecl* genericDecl;
    struct FuncUse* genericFuncUse;
    bool hasThis;
    usize deferCounter;
    usize eachCounter;
    usize selectCounter;
};
struct Expr {
    struct OwnedString text;
    struct OwnedString typeKey;
    struct OwnedString cType;
    struct OwnedString arrayLen;
    bool isArray;
    bool isLvalue;
    bool isPointer;
};
struct ExprParser {
    struct CompilerContext* program;
    struct CodegenEnv* env;
    usize fileIndex;
    usize pos;
    usize end;
    struct OwnedString expectedTypeKey;
    struct OwnedString expectedCType;
    bool expectedIsArray;
};
struct DeferRange {
    usize start;
    usize end;
};
typedef enum DeclKind {
    DeclKind_Unknown,
    DeclKind_Alias,
    DeclKind_Struct,
    DeclKind_Union,
    DeclKind_Enum,
    DeclKind_Function,
    DeclKind_ExternC,
} DeclKind;
struct SyntaxFile {
    struct Token* tokens;
    usize tokenLen;
    usize tokenCap;
    struct String sourceText;
    struct String path;
    struct OwnedString packageName;
    struct OwnedString moduleName;
    usize fileIndex;
};
struct TokenizeJob {
    struct Allocator allocator;
    struct SourceFile* sourceFile;
    struct SyntaxFile* tokenFile;
    struct DiagnosticBag diagnostics;
    usize threadHandle;
    Status status;
    usize fileIndex;
    bool isRoot;
    bool threaded;
};
struct Array__OwnedString {
    struct OwnedString* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct TypeUse {
    struct OwnedString key;
    struct OwnedString cName;
    struct OwnedString baseName;
    struct Array__OwnedString args;
    bool emitted;
};
struct FuncUse {
    usize declIndex;
    struct OwnedString key;
    struct OwnedString cName;
    struct Array__OwnedString args;
    bool emitted;
};
struct ModuleParam {
    usize fileIndex;
    usize typeStart;
    usize typeEnd;
    usize nameIndex;
    usize defaultStart;
    usize defaultEnd;
    bool hasDefault;
};
struct ModuleField {
    usize fileIndex;
    usize typeStart;
    usize typeEnd;
    usize nameIndex;
    usize defaultStart;
    usize defaultEnd;
    usize arrayStart;
    usize arrayEnd;
    bool hasDefault;
    bool isArray;
    bool isNestedStruct;
    usize nestedBodyStart;
    usize nestedBodyEnd;
    usize nestedFirstField;
    usize nestedFieldCount;
};
struct ModuleDecl {
    DeclKind kind;
    usize fileIndex;
    usize start;
    usize end;
    usize nameIndex;
    usize returnStart;
    usize returnEnd;
    usize receiverStart;
    usize receiverEnd;
    bool hasReceiver;
    bool isOperator;
    u8 operatorCode;
    usize genericStart;
    usize genericEnd;
    bool isGeneric;
    usize paramStart;
    usize paramEnd;
    usize bodyStart;
    usize bodyEnd;
    bool hasBody;
    usize firstParam;
    usize paramCount;
    usize firstField;
    usize fieldCount;
    struct OwnedString packageName;
    struct OwnedString moduleName;
    usize docStart;
    usize docEnd;
    bool hasDocComment;
    bool emitted;
    bool reachable;
    bool isTagged;
};
struct DeclNameIndexEntry {
    struct OwnedString name;
    struct OwnedString scope;
    usize declIndex;
};
struct MethodDeclIndexEntry {
    struct OwnedString receiverKey;
    struct OwnedString name;
    usize declIndex;
    u8 operatorCode;
    usize paramCount;
    bool isOperator;
};
struct TypeInfo {
    struct OwnedString key;
    struct OwnedString cType;
    struct OwnedString baseName;
    struct Array__OwnedString args;
    bool isPointer;
};
struct Array__SyntaxFile {
    struct SyntaxFile* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct Array__ModuleDecl {
    struct ModuleDecl* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct Array__ModuleParam {
    struct ModuleParam* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct Array__ModuleField {
    struct ModuleField* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct Array__TypeUse {
    struct TypeUse* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct Array__FuncUse {
    struct FuncUse* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct Array__DeclNameIndexEntry {
    struct DeclNameIndexEntry* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct Array__MethodDeclIndexEntry {
    struct MethodDeclIndexEntry* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct CompilerContext {
    struct Allocator allocator;
    struct DiagnosticBag diagnostics;
    struct FileTable files;
    struct Array__SyntaxFile tokenFiles;
    struct Array__ModuleDecl decls;
    struct Array__ModuleParam params;
    struct Array__ModuleField fields;
    struct Array__TypeUse typeUses;
    struct Array__FuncUse funcUses;
    struct Array__DeclNameIndexEntry typeDeclIndex;
    struct Array__DeclNameIndexEntry functionDeclIndex;
    struct Array__MethodDeclIndexEntry methodDeclIndex;
};
struct Slice__byte {
    byte* data;
    usize len;
};
struct Result__usize {
    Status status;
    usize value;
};
struct Result__OwnedString {
    Status status;
    struct OwnedString value;
};
struct Span__byte {
    byte* data;
    usize len;
};
struct Result__File {
    Status status;
    struct File value;
};
struct Result__String {
    Status status;
    struct String value;
};
struct Array__Token {
    struct Token* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct Array__usize {
    usize* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct Array__DeferRange {
    struct DeferRange* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct Result__DeferRange {
    Status status;
    struct DeferRange value;
};
struct Array__TypeInfo {
    struct TypeInfo* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct Array__TokenizeJob {
    struct TokenizeJob* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};
struct Result__Diagnostic {
    Status status;
    struct Diagnostic value;
};
struct Slice__Diagnostic {
    struct Diagnostic* data;
    usize len;
};
struct Result__SourceFile {
    Status status;
    struct SourceFile value;
};
struct Slice__SourceFile {
    struct SourceFile* data;
    usize len;
};
struct Result__Token {
    Status status;
    struct Token value;
};
struct Slice__Token {
    struct Token* data;
    usize len;
};
struct Result__AstNode {
    Status status;
    struct AstNode value;
};
struct Slice__AstNode {
    struct AstNode* data;
    usize len;
};
struct Slice__usize {
    usize* data;
    usize len;
};
struct Slice__DeferRange {
    struct DeferRange* data;
    usize len;
};
struct Result__Local {
    Status status;
    struct Local value;
};
struct Slice__Local {
    struct Local* data;
    usize len;
};
struct Result__DeclNameIndexEntry {
    Status status;
    struct DeclNameIndexEntry value;
};
struct Slice__DeclNameIndexEntry {
    struct DeclNameIndexEntry* data;
    usize len;
};
struct Result__MethodDeclIndexEntry {
    Status status;
    struct MethodDeclIndexEntry value;
};
struct Slice__MethodDeclIndexEntry {
    struct MethodDeclIndexEntry* data;
    usize len;
};
struct Slice__OwnedString {
    struct OwnedString* data;
    usize len;
};
struct Result__TypeInfo {
    Status status;
    struct TypeInfo value;
};
struct Slice__TypeInfo {
    struct TypeInfo* data;
    usize len;
};
struct Result__SyntaxFile {
    Status status;
    struct SyntaxFile value;
};
struct Slice__SyntaxFile {
    struct SyntaxFile* data;
    usize len;
};
struct Result__ModuleDecl {
    Status status;
    struct ModuleDecl value;
};
struct Slice__ModuleDecl {
    struct ModuleDecl* data;
    usize len;
};
struct Result__ModuleParam {
    Status status;
    struct ModuleParam value;
};
struct Slice__ModuleParam {
    struct ModuleParam* data;
    usize len;
};
struct Result__ModuleField {
    Status status;
    struct ModuleField value;
};
struct Slice__ModuleField {
    struct ModuleField* data;
    usize len;
};
struct Result__TypeUse {
    Status status;
    struct TypeUse value;
};
struct Slice__TypeUse {
    struct TypeUse* data;
    usize len;
};
struct Result__FuncUse {
    Status status;
    struct FuncUse value;
};
struct Slice__FuncUse {
    struct FuncUse* data;
    usize len;
};
struct Result__TokenizeJob {
    Status status;
    struct TokenizeJob value;
};
struct Slice__TokenizeJob {
    struct TokenizeJob* data;
    usize len;
};

int main(int argc,str* argv);
struct Allocator std_mem_DefaultAllocator(void);
struct String std_string_StringFromCString(str text);
struct Slice__byte String_Bytes(struct String* this);
struct String String_Slice(struct String* this,usize start,usize length);
bool String_Equals(struct String* this,struct String other);
bool String_EqualsCString(struct String* this,str text);
int String_Compare(struct String* this,struct String other);
struct Result__usize String_FindByte(struct String* this,byte value);
bool String_ContainsByte(struct String* this,byte value);
bool String_StartsWith(struct String* this,struct String prefix);
bool String_EndsWith(struct String* this,struct String suffix);
bool std_string_IsAsciiSpace(byte c);
bool std_string_IsAsciiAlpha(byte c);
bool std_string_IsAsciiDigit(byte c);
bool std_string_IsAsciiWord(byte c);
struct String OwnedString_View(struct OwnedString* this);
struct String std_string_OwnedStringView(struct OwnedString* owned);
Status std_string_DestroyOwnedString(struct OwnedString* owned);
Status std_string_CloneString(struct String text,struct Allocator allocator,struct OwnedString* out);
Status std_string_CloneCString(str text,struct Allocator allocator,struct OwnedString* out);
Status OwnedString_Destroy(struct OwnedString* this);
struct StringBuilder std_string_StringBuilderNew(struct Allocator allocator);
Status StringBuilder_Destroy(struct StringBuilder* this);
Status StringBuilder_Reserve(struct StringBuilder* this,usize additional);
struct Result__usize StringBuilder_Write(struct StringBuilder* this,struct Slice__byte data);
Status StringBuilder_WriteByte(struct StringBuilder* this,byte value);
Status StringBuilder_WriteString(struct StringBuilder* this,struct String text);
Status StringBuilder_WriteCString(struct StringBuilder* this,str text);
Status StringBuilder_WriteRepeatByte(struct StringBuilder* this,byte value,usize count);
Status StringBuilder_WriteIndent(struct StringBuilder* this,usize count);
struct String StringBuilder_View(struct StringBuilder* this);
struct Result__OwnedString StringBuilder_Detach(struct StringBuilder* this);
struct ByteCursor std_scan_ByteCursorNew(struct String input);
bool ByteCursor_AtEnd(struct ByteCursor* this);
byte ByteCursor_Peek(struct ByteCursor* this);
byte ByteCursor_PeekAt(struct ByteCursor* this,usize offset);
byte ByteCursor_Advance(struct ByteCursor* this);
Status ByteCursor_SkipAsciiWhitespace(struct ByteCursor* this);
struct Result__usize MemoryReader_Read(struct MemoryReader* this,struct Span__byte out);
struct Result__usize MemoryWriter_Write(struct MemoryWriter* this,struct Slice__byte data);
struct Result__File std_file_FileOpen(str path,FileMode mode);
struct Result__usize File_Read(struct File* this,struct Span__byte out);
struct Result__usize File_Write(struct File* this,struct Slice__byte data);
Status std_file_WriteFile(str path,struct String text);
Status File_Close(struct File* this);
struct File std_file_Stdout(void);
struct File std_file_Stderr(void);
Status std_file_ReadAllToOwnedString(struct File file,struct Allocator allocator,struct OwnedString* out);
Status Directory_Close(struct Directory* this);
Status std_format_WriteByteToBuilder(struct StringBuilder* writer,byte value);
Status std_format_WriteStringToFile(struct File* writer,struct String text);
Status std_format_FormatU64ToBuilder(struct StringBuilder* writer,u64 value,u8 base);
Status std_format_FormatI64ToBuilder(struct StringBuilder* writer,i64 value);
struct TemplateContext std_template_TemplateContextNew(struct TemplateArg* named,usize namedLen,struct String* positional,usize positionalLen);
struct TemplateArg std_template_TemplateArgNew(struct String name,struct String value);
struct Result__String std_template_TemplateFindNamed(struct TemplateContext ctx,struct String name);
struct Result__usize std_template_TemplateParseIndex(struct String text);
Status std_template_TemplateWritePlaceholder(struct StringBuilder* out,struct String name,struct TemplateContext ctx);
Status std_template_RenderTemplateToBuilder(struct StringBuilder* out,struct String templateText,struct TemplateContext ctx);
struct SourceLocation kek_diagnostics_SourceLocationNew(usize line,usize column,usize offset,usize length);
void kek_diagnostics_DiagnosticBagInit(struct DiagnosticBag* bag,struct Allocator allocator);
Status kek_diagnostics_DiagnosticBagDestroy(struct DiagnosticBag* bag);
Status kek_diagnostics_DiagnosticBagReserve(struct DiagnosticBag* bag,usize additional);
Status kek_diagnostics_DiagnosticAdd(struct DiagnosticBag* bag,DiagnosticSeverity severity,DiagnosticPhase phase,i64 fileIndex,struct SourceLocation location,struct String message);
Status kek_diagnostics_DiagnosticAddCString(struct DiagnosticBag* bag,DiagnosticSeverity severity,DiagnosticPhase phase,i64 fileIndex,struct SourceLocation location,str message);
Status kek_diagnostics_DiagnosticAddPathMessage(struct DiagnosticBag* bag,DiagnosticSeverity severity,DiagnosticPhase phase,i64 fileIndex,struct SourceLocation location,str prefix,struct String path);
Status kek_diagnostics_WriteU64Field(struct StringBuilder* out,u64 value,bool separator);
Status kek_diagnostics_WriteI64Field(struct StringBuilder* out,i64 value,bool separator);
Status kek_diagnostics_WriteDiagnosticDump(struct DiagnosticBag* bag,struct StringBuilder* out);
void kek_source_FileTableInit(struct FileTable* table,struct Allocator allocator);
Status kek_source_FileTableDestroy(struct FileTable* table);
bool kek_source_StringEndsWithCString(struct String text,str suffixText);
bool kek_source_PathIsAbsolute(struct String path);
usize kek_source_LastPathSeparator(struct String path,usize end);
Status kek_source_DirectoryPrefix(struct String path,struct Allocator allocator,struct OwnedString* out);
Status kek_source_JoinPath(struct String root,struct String path,struct Allocator allocator,struct OwnedString* out);
Status kek_source_NormalizePath(struct String path,struct Allocator allocator,struct OwnedString* out);
bool kek_source_FileReadable(struct String path,struct Allocator allocator);
bool kek_source_FileAlreadyLoaded(struct FileTable* table,struct String path);
Status kek_source_FileTableReserve(struct FileTable* table,usize additional);
Status kek_source_ReadFile(struct String path,struct FileTable* table,struct DiagnosticBag* diagnostics);
Status kek_source_SetImportRoot(struct FileTable* table,struct String entryPath);
Status kek_source_ReadResolvedImportFile(struct FileTable* table,struct String path,struct DiagnosticBag* diagnostics);
bool kek_source_ImportDirectiveAt(struct String source,usize cursor);
Status kek_source_LoadImportFile(struct FileTable* table,struct String path,struct DiagnosticBag* diagnostics,i64 fileIndex,struct SourceLocation location);
Status kek_source_LoadImports(struct FileTable* table,struct SourceFile* file,struct DiagnosticBag* diagnostics);
Status kek_source_LoadCompilationSources(str entryPath,struct Allocator allocator,struct FileTable* table,struct DiagnosticBag* diagnostics);
struct Token kek_tokenizer_TokenNew(TokenKind kind,u64 subkind,usize line,usize column,usize offset,usize length);
struct Tokenizer kek_tokenizer_TokenizerNew(struct String source,bool emitComments);
bool Tokenizer_AtEnd(struct Tokenizer* this);
byte Tokenizer_Peek(struct Tokenizer* this);
byte Tokenizer_PeekAt(struct Tokenizer* this,usize offset);
byte Tokenizer_Advance(struct Tokenizer* this);
bool kek_tokenizer_TextAtEquals(str text,struct String source,usize start,usize length);
bool Tokenizer_StartsWith(struct Tokenizer* this,str text);
void Tokenizer_AdvanceMany(struct Tokenizer* this,usize count);
void Tokenizer_SkipWhitespace(struct Tokenizer* this);
void Tokenizer_SkipWhitespaceAndComments(struct Tokenizer* this);
struct Token Tokenizer_ReadLineComment(struct Tokenizer* this);
struct Token Tokenizer_ReadBlockComment(struct Tokenizer* this);
u64 kek_tokenizer_KeywordSubkind(struct String source,usize start,usize length);
struct Token Tokenizer_ReadIdentifierOrKeyword(struct Tokenizer* this);
struct Token Tokenizer_ReadNumber(struct Tokenizer* this);
struct Token Tokenizer_ReadDelimited(struct Tokenizer* this,TokenKind kind,byte delimiter);
struct Token Tokenizer_ReadTripleQuotedString(struct Tokenizer* this);
bool Tokenizer_ReadOperator(struct Tokenizer* this,struct Token* out,usize start,usize line,usize column);
bool Tokenizer_ReadPunctuation(struct Tokenizer* this,struct Token* out,usize start,usize line,usize column);
struct Token Tokenizer_Next(struct Tokenizer* this);
Status kek_tokenizer_TokenizeToArray(struct String source,bool emitComments,struct Allocator allocator,struct Array__Token* out);
struct SourceLocation kek_ast_TokenLocation(struct Token token);
Status kek_ast_AstTreeReserve(struct AstTree* tree,usize additional);
Status kek_ast_AstTreeInit(struct AstTree* tree,struct Allocator allocator);
Status kek_ast_AstTreeDestroy(struct AstTree* tree);
bool kek_ast_IsPunctuationToken(struct Token* token,PunctuationKind punctuation);
bool kek_ast_IsOperatorToken(struct Token* token,OperatorKind operatorKind);
bool kek_ast_IsTriviaToken(struct Token* token);
bool kek_ast_IsClosingPunctuation(struct Token* token);
bool kek_ast_IsAstTerminator(struct Token* token,u64 closePunctuation);
bool kek_ast_IsGenericTerminator(struct Token* token);
bool kek_ast_TokenTextEquals(struct Parser* parser,struct Token* token,str text);
usize kek_ast_CreateAstNode(struct Parser* parser,AstKind kind,struct SourceLocation location);
void kek_ast_AddAstChild(struct AstTree* tree,usize parentIndex,usize childIndex);
void kek_ast_FinishLocationFromChildren(struct AstTree* tree,usize nodeIndex);
usize kek_ast_ParseTokenNode(struct Parser* parser);
void kek_ast_ReportParseError(struct Parser* parser,struct Token* token,str message);
str kek_ast_PunctuationName(u64 punctuation);
void kek_ast_ReportExpected(struct Parser* parser,struct Token* token,u64 punctuation);
bool kek_ast_ShouldParseGenericList(struct Parser* parser,usize previousChildIndex);
void kek_ast_ParseChildrenInto(struct Parser* parser,usize parentIndex,u64 closePunctuation);
usize kek_ast_ParseDelimited(struct Parser* parser,AstKind kind,u64 closePunctuation);
usize kek_ast_ParseGenericDelimited(struct Parser* parser);
usize kek_ast_ParseStatement(struct Parser* parser,u64 closePunctuation);
usize kek_ast_ParseList(struct Parser* parser,AstKind listKind,u64 closePunctuation);
Status kek_ast_ParseTokens(struct Token* tokens,usize count,struct String source,i64 fileIndex,struct Allocator allocator,struct DiagnosticBag* diagnostics,struct AstTree* out);
int kek_compiler_CompilerFail(str text);
int kek_compiler_CompilerPrintHelp(void);
Status kek_compiler_CompileToCString(str inputPath,struct Allocator allocator,struct StringBuilder* out,struct DiagnosticBag* diagnostics);
Status kek_compiler_CompileToC(str inputPath,str outputPath,struct Allocator allocator,struct DiagnosticBag* diagnostics);
Status kek_compiler_CompilerRunBuild(str inputPath,str outputPath);
int kek_compiler_CompilerMain(int argc,str* argv);
Status kek_codegen_WriteProgram(struct CompilerContext* program,struct StringBuilder* out);
Status kek_codegen_GenerateC(struct CompilerContext* program,struct StringBuilder* out);
Status kek_codegen_funcs_WriteSubstTypeForFunc(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct FuncUse* funcUse,usize fileIndex,usize start,usize end);
Status kek_codegen_funcs_WriteFunctionSignature(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct String nameOverride,struct FuncUse* funcUse);
Status kek_codegen_funcs_WriteFunctionPrototype(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl);
Status kek_codegen_funcs_WriteGenericFunctionBody(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct FuncUse* use);
bool kek_codegen_funcs_GenericBodyMentionsGenericParam(struct CompilerContext* program,struct ModuleDecl* decl,struct FuncUse* use);
bool kek_codegen_funcs_CanLowerGenericFunctionBody(struct CompilerContext* program,struct ModuleDecl* decl,struct FuncUse* use);
Status kek_codegen_funcs_WriteFunctionBody(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl);
Status kek_codegen_funcs_WriteArrayMethodRef(struct StringBuilder* out,struct FuncUse* use,str name);
Status kek_codegen_funcs_WriteLinkedListMethodRef(struct StringBuilder* out,struct FuncUse* use,str name);
Status kek_codegen_funcs_WriteResultTypeRef(struct StringBuilder* out,struct FuncUse* use);
Status kek_codegen_funcs_WriteListNodeTypeRef(struct StringBuilder* out,struct FuncUse* use);
Status kek_codegen_funcs_WriteManualArrayGenericBody(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct FuncUse* use,struct String argC);
Status kek_codegen_funcs_WriteManualLinkedListGenericBody(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct FuncUse* use);
Status kek_codegen_funcs_WriteManualGenericBody(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct FuncUse* use);
Status kek_codegen_funcs_WriteFunctionDeclarations(struct CompilerContext* program,struct StringBuilder* out);
Status kek_codegen_funcs_WriteFunctionDefinitions(struct CompilerContext* program,struct StringBuilder* out);
usize kek_codegen_stmt_FindStatementSemicolon(struct CompilerContext* program,usize fileIndex,usize start,usize end);
Status kek_codegen_stmt_ArrayLenString(struct CompilerContext* program,usize fileIndex,usize start,usize end,struct OwnedString* out);
bool kek_codegen_stmt_ModuleFieldDefaultIsZero(struct CompilerContext* program,struct ModuleField* field);
Status kek_codegen_stmt_WriteDefaultInitializer(struct CompilerContext* program,struct StringBuilder* out,struct String typeKey);
Status kek_codegen_stmt_WriteVarDecl(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize semicolon,usize indent);
Status kek_codegen_stmt_WriteExprStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize semicolon,usize indent);
Status kek_codegen_stmt_WriteIfStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_codegen_stmt_WriteWhileStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_codegen_stmt_WriteDoStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_codegen_stmt_WriteForStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_codegen_stmt_WriteEachStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_codegen_stmt_WriteSwitchStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
struct ModuleField* kek_codegen_stmt_FindTaggedUnionVariant(struct CompilerContext* program,struct ModuleDecl* decl,struct String name,usize* outIndex);
bool kek_codegen_stmt_SelectMatchedVariant(struct Array__usize* matched,usize index);
Status kek_codegen_stmt_WriteSelectTempName(struct StringBuilder* out,usize index);
Status kek_codegen_stmt_WriteSelectStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_codegen_stmt_WriteReturnStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize semicolon,usize indent);
Status kek_codegen_stmt_WriteSingleStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_codegen_stmt_WriteDeferred(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize end,usize indent);
bool kek_codegen_stmt_IsBlockTransfer(struct CompilerContext* program,usize fileIndex,usize index);
Status kek_codegen_stmt_WriteDeferredRanges(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,struct Array__DeferRange* defers,usize indent);
Status kek_codegen_stmt_WriteBlock(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize end,usize indent);
Status kek_codegen_expr_ExprInitEmpty(struct CompilerContext* program,struct Expr* expr);
Status kek_codegen_expr_ExprDestroy(struct Expr* expr);
Status kek_codegen_expr_ExprSetText(struct CompilerContext* program,struct Expr* expr,struct String text);
Status kek_codegen_expr_ExprSetCString(struct CompilerContext* program,struct Expr* expr,str text);
bool kek_codegen_expr_IsTripleQuotedString(struct String text);
Status kek_codegen_expr_WriteOctalEscape(struct StringBuilder* out,byte value);
Status kek_codegen_expr_WriteCStringEscapedByte(struct StringBuilder* out,byte value);
Status kek_codegen_expr_WriteStringLiteralToken(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize index);
Status kek_codegen_expr_ExprSetStringLiteralToken(struct CompilerContext* program,struct Expr* expr,usize fileIndex,usize index);
Status kek_codegen_expr_ExprSetType(struct CompilerContext* program,struct Expr* expr,struct String key,struct String cType,bool isPointer);
Status kek_codegen_expr_RenderEnvTypeInfo(struct CompilerContext* program,struct CodegenEnv* env,usize fileIndex,usize start,usize end,struct TypeInfo* info);
Status kek_codegen_expr_ExprFromBuilder(struct Expr* expr,struct StringBuilder* builder);
u8 kek_codegen_expr_OperatorPrecedence(struct CompilerContext* program,usize fileIndex,usize index);
bool kek_codegen_expr_OperatorRightAssociative(struct CompilerContext* program,usize fileIndex,usize index);
Status kek_codegen_expr_WriteNumberToken(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize index);
struct ModuleDecl* kek_codegen_expr_FindFieldDecl(struct CompilerContext* program,struct String typeKey);
Status kek_codegen_expr_NestedFieldTypeKey(struct CompilerContext* program,usize fieldIndex,struct OwnedString* out);
bool kek_codegen_expr_NestedFieldIndexFromKey(struct String typeKey,usize* out);
struct TypeUse* kek_codegen_expr_FindTypeUse(struct CompilerContext* program,struct String typeKey);
Status kek_codegen_expr_ModuleFieldType(struct CompilerContext* program,struct String typeKey,struct String fieldName,struct TypeInfo* outInfo);
struct ModuleDecl* kek_codegen_expr_FindMethod(struct CompilerContext* program,struct String receiverType,struct String name,bool isOperator,u8 operatorCode,usize argCount);
Status kek_codegen_expr_ParserInit(struct CompilerContext* program,struct CodegenEnv* env,usize fileIndex,usize start,usize end,struct String expectedKey,struct String expectedCType,bool expectedIsArray,struct ExprParser* parser);
Status kek_codegen_expr_ParserDestroy(struct ExprParser* parser);
Status kek_codegen_expr_CompileExpressionRange(struct CompilerContext* program,struct CodegenEnv* env,usize fileIndex,usize start,usize end,struct String expectedKey,struct String expectedCType,bool expectedIsArray,struct Expr* out);
Status kek_codegen_expr_WriteInitializerList(struct ExprParser* parser,usize start,usize end,struct StringBuilder* out);
Status kek_codegen_expr_ExprFromLiteralBlock(struct ExprParser* parser,usize blockStart,usize blockEnd,struct Expr* out);
Status kek_codegen_expr_CompilePrimary(struct ExprParser* parser,struct Expr* out);
Status kek_codegen_expr_WriteCallArgs(struct ExprParser* parser,usize start,usize end,struct StringBuilder* out);
usize kek_codegen_expr_CountCallArgs(struct CompilerContext* program,usize fileIndex,usize start,usize end);
Status kek_codegen_expr_ApplyPostfix(struct ExprParser* parser,struct Expr* expr);
Status kek_codegen_expr_CompileUnary(struct ExprParser* parser,struct Expr* out);
Status kek_codegen_expr_CompileBinaryOperation(struct ExprParser* parser,struct Expr* left,usize operatorIndex,struct Expr* right,struct Expr* out);
Status kek_codegen_expr_CompileExpression(struct ExprParser* parser,u8 minPrecedence,struct Expr* out);
Status kek_codegen_types_FuncUseTempTypeUse(struct FuncUse* funcUse,struct TypeUse* out);
Status kek_codegen_types_WriteDeclarator(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize typeStart,usize typeEnd,usize nameIndex,bool isArray,usize arrayStart,usize arrayEnd);
Status kek_codegen_types_WriteFields(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl);
bool kek_codegen_types_GenericParamEquals(struct CompilerContext* program,struct ModuleDecl* decl,usize paramIndex,struct String name);
Status kek_codegen_types_WriteTypeSuffixSubst(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct TypeUse* use,usize fileIndex,usize start,usize end);
Status kek_codegen_types_MakeSubstTypeKey(struct CompilerContext* program,struct ModuleDecl* decl,struct TypeUse* use,usize fileIndex,usize start,usize end,struct OwnedString* out);
Status kek_codegen_types_MakeSubstTypeInfo(struct CompilerContext* program,struct ModuleDecl* decl,struct TypeUse* use,usize fileIndex,usize start,usize end,struct TypeInfo* info);
Status kek_codegen_types_MakeSubstTypeInfoForFunc(struct CompilerContext* program,struct ModuleDecl* decl,struct FuncUse* funcUse,usize fileIndex,usize start,usize end,struct OwnedString* outKey,struct OwnedString* outCType,bool* outIsPointer);
Status kek_codegen_types_WriteDeclaratorSubst(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct TypeUse* use,usize fileIndex,usize typeStart,usize typeEnd,usize nameIndex,bool isArray,usize arrayStart,usize arrayEnd);
Status kek_codegen_types_WriteFieldsSubst(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct TypeUse* use);
Status kek_codegen_types_WriteStructDecl(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct String specializedName);
Status kek_codegen_types_WriteUnionDecl(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl);
Status kek_codegen_types_WriteTaggedUnionTagName(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl);
Status kek_codegen_types_WriteTaggedUnionVariantName(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct ModuleField* field);
Status kek_codegen_types_WriteTaggedUnionConstructorName(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct ModuleField* field);
Status kek_codegen_types_WriteTaggedUnionDecl(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl);
Status kek_codegen_types_WriteEnumDecl(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl);
Status kek_codegen_types_WriteAliasDecl(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl);
Status kek_codegen_types_WriteExternDecl(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl);
Status kek_codegen_types_WriteTypeUseDeclaration(struct CompilerContext* program,struct StringBuilder* out,struct TypeUse* use);
bool kek_codegen_types_ShouldWritePlainTypeDecl(struct ModuleDecl* decl);
Status kek_codegen_types_WritePlainTypeDeclaration(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl);
Status kek_codegen_types_WriteTypeKeyDependencies(struct CompilerContext* program,struct StringBuilder* out,struct String key);
Status kek_codegen_types_WriteTypeDeclarationWithDependencies(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl);
Status kek_codegen_types_WriteFieldTypeDependencies(struct CompilerContext* program,struct StringBuilder* out,struct ModuleField* field);
Status kek_codegen_types_WriteTypeRangeDependencies(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize start,usize end);
Status kek_codegen_types_WriteTypeUseWithDependencies(struct CompilerContext* program,struct StringBuilder* out,struct TypeUse* use);
Status kek_codegen_types_WriteTypeKeyDependencies(struct CompilerContext* program,struct StringBuilder* out,struct String key);
Status kek_codegen_types_WriteTypeDeclarationWithDependencies(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl);
Status kek_codegen_types_WriteTypeDeclarations(struct CompilerContext* program,struct StringBuilder* out);
struct CodeWriter kek_codegen_emit_CodeWriterNew(struct StringBuilder* out);
void CodeWriter_CString(struct CodeWriter* this,str text);
void CodeWriter_String(struct CodeWriter* this,struct String text);
void CodeWriter_Byte(struct CodeWriter* this,byte value);
void CodeWriter_Indent(struct CodeWriter* this,usize indent);
void CodeWriter_Token(struct CodeWriter* this,struct CompilerContext* program,usize fileIndex,usize index);
void CodeWriter_TokenRangeRaw(struct CodeWriter* this,struct CompilerContext* program,usize fileIndex,usize start,usize end);
Status kek_codegen_emit_WritePrelude(struct StringBuilder* out);
Status kek_codegen_emit_WriteIndent(struct StringBuilder* out,usize indent);
Status kek_codegen_emit_WriteManualLine(struct StringBuilder* out,str text);
Status kek_codegen_state_EnvInit(struct CompilerContext* program,struct CodegenEnv* env);
Status kek_codegen_state_EnvDestroy(struct CodegenEnv* env);
Status kek_codegen_state_ReserveLocals(struct CodegenEnv* env,usize additional);
Status kek_codegen_state_EnvRestore(struct CodegenEnv* env,usize localCount);
struct Local* kek_codegen_state_EnvFind(struct CodegenEnv* env,struct String name);
Status kek_codegen_state_EnvAdd(struct CompilerContext* program,struct CodegenEnv* env,struct String name,struct String typeKey,struct String cType,bool isArray,struct String arrayLen,bool isPointer);
bool kek_sema_IsTypeDeclKind(DeclKind kind);
int kek_sema_DeclNameIndexCompare(struct DeclNameIndexEntry* left,struct DeclNameIndexEntry* right);
Status kek_sema_SortDeclNameIndex(struct Array__DeclNameIndexEntry* items);
Status kek_sema_ClearDeclNameIndex(struct Array__DeclNameIndexEntry* items);
int kek_sema_MethodDeclIndexCompare(struct MethodDeclIndexEntry* left,struct MethodDeclIndexEntry* right);
Status kek_sema_SortMethodDeclIndex(struct Array__MethodDeclIndexEntry* items);
Status kek_sema_ClearMethodDeclIndex(struct Array__MethodDeclIndexEntry* items);
Status kek_sema_AddDeclNameIndexEntry(struct CompilerContext* program,struct Array__DeclNameIndexEntry* items,struct String name,struct String scope,usize declIndex);
Status kek_sema_AddMethodDeclIndexEntry(struct CompilerContext* program,struct Array__MethodDeclIndexEntry* items,struct ModuleDecl* decl,usize declIndex);
struct String kek_sema_ModuleDeclScopeName(struct ModuleDecl* decl);
Status kek_sema_BuildDeclIndexes(struct CompilerContext* program);
struct ModuleDecl* kek_sema_FindDeclByIndexedName(struct CompilerContext* program,struct Array__DeclNameIndexEntry* items,struct String name,struct String scope);
struct ModuleDecl* kek_sema_FindIndexedFunctionDeclByName(struct CompilerContext* program,struct String name,struct String scopeName);
struct ModuleDecl* kek_sema_FindIndexedMethodDecl(struct CompilerContext* program,struct String receiverType,struct String name,bool isOperator,u8 operatorCode,usize argCount);
bool kek_sema_MarkDeclReachable(struct ModuleDecl* decl);
bool kek_sema_MarkReachableRoots(struct CompilerContext* program);
bool kek_sema_MarkNamedDeclsReachable(struct CompilerContext* program,struct String name);
bool kek_sema_MarkOperatorDeclsReachable(struct CompilerContext* program,u8 operatorCode);
bool kek_sema_MarkReferencesInRange(struct CompilerContext* program,usize fileIndex,usize start,usize end);
Status kek_sema_MarkReachableDecls(struct CompilerContext* program);
Status kek_sema_WriteTokenRangeRaw(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize start,usize end);
bool kek_sema_IsBuiltinTypeName(struct String name);
Status kek_sema_WriteSanitized(struct StringBuilder* out,struct String text);
Status kek_sema_WriteOperatorName(struct StringBuilder* out,u8 operatorCode);
bool kek_sema_ModuleDeclPackageIsRoot(struct ModuleDecl* decl);
Status kek_sema_WriteDeclCName(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl);
Status kek_sema_WriteTypeSuffixFromRange(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize start,usize end);
Status kek_sema_TypeInfoDestroy(struct TypeInfo* info);
Status kek_sema_TypeInfoInitEmpty(struct CompilerContext* program,struct TypeInfo* info);
struct String kek_sema_OwnedStringArrayGet(struct Array__OwnedString* args,usize index);
Status kek_sema_OwnedStringArrayPushClone(struct CompilerContext* program,struct Array__OwnedString* args,struct String value);
Status kek_sema_CloneOwnedStringArray(struct CompilerContext* program,struct Array__OwnedString* source,struct Array__OwnedString* target);
Status kek_sema_ReplaceOwned(struct OwnedString* target,struct OwnedString value);
Status kek_sema_BuildTypeKey(struct CompilerContext* program,usize fileIndex,usize start,usize end,struct OwnedString* out);
Status kek_sema_WriteCTypeFromKey(struct CompilerContext* program,struct StringBuilder* out,struct String key);
Status kek_sema_MakeCTypeFromKey(struct CompilerContext* program,struct String key,struct OwnedString* out);
Status kek_sema_TypeInfoFromKey(struct CompilerContext* program,struct String key,struct TypeInfo* info);
Status kek_sema_RenderTypeInfo(struct CompilerContext* program,usize fileIndex,usize start,usize end,struct TypeInfo* info);
bool kek_sema_TypeUseExists(struct CompilerContext* program,struct String key);
Status kek_sema_AddTypeUse(struct CompilerContext* program,struct TypeInfo* info);
Status kek_sema_AddTypeUseFromBaseArg(struct CompilerContext* program,str baseName,struct String arg0);
bool kek_sema_ConcreteTypeKey(struct CompilerContext* program,struct String key);
bool kek_sema_TypeInfoConcrete(struct CompilerContext* program,struct TypeInfo* info);
Status kek_sema_CollectTypeUsesInRange(struct CompilerContext* program,usize fileIndex,usize start,usize end);
bool kek_sema_ModuleDeclScopeMatches(struct ModuleDecl* decl,struct String scopeName);
struct ModuleDecl* kek_sema_FindFunctionDeclByName(struct CompilerContext* program,struct String name,struct String scopeName);
bool kek_sema_FuncUseExists(struct CompilerContext* program,usize declIndex,struct String key);
usize kek_sema_ModuleDeclIndex(struct CompilerContext* program,struct ModuleDecl* decl);
Status kek_sema_BuildGenericFuncCName(struct CompilerContext* program,struct ModuleDecl* decl,struct Array__TypeInfo* args,struct OwnedString* out);
Status kek_sema_AddFuncUse(struct CompilerContext* program,struct ModuleDecl* decl,struct Array__TypeInfo* args);
Status kek_sema_CollectGenericFunctionUsesInRange(struct CompilerContext* program,usize fileIndex,usize start,usize end);
struct ModuleDecl* kek_sema_FindGenericMethodDecl(struct CompilerContext* program,struct String receiverBase,str methodName);
Status kek_sema_AddGenericMethodUseByName(struct CompilerContext* program,struct TypeUse* typeUse,str methodName);
bool kek_sema_MethodCallNameUsed(struct CompilerContext* program,str methodName);
Status kek_sema_CollectGenericCollectionMethods(struct CompilerContext* program);
Status kek_sema_CollectGenericFunctionUses(struct CompilerContext* program);
Status kek_sema_CollectTypeUses(struct CompilerContext* program);
Status kek_sema_CheckModule(struct CompilerContext* program);
usize kek_module_FindMatching(struct CompilerContext* program,usize fileIndex,usize openIndex);
usize kek_module_SkipDelimited(struct CompilerContext* program,usize fileIndex,usize index);
usize kek_module_SkipAttributes(struct CompilerContext* program,usize fileIndex,usize index);
usize kek_module_FindTopLevelColon(struct CompilerContext* program,usize fileIndex,usize start,usize end);
usize kek_module_FindTokenAtDepthZero(struct CompilerContext* program,usize fileIndex,usize start,usize end,PunctuationKind punctuation);
usize kek_module_FindOperatorScope(struct CompilerContext* program,usize fileIndex,usize start,usize end);
usize kek_module_FindNextGroup(struct CompilerContext* program,usize fileIndex,usize start,usize end);
usize kek_module_FindNextBlock(struct CompilerContext* program,usize fileIndex,usize start,usize end);
u8 kek_module_OperatorCode(struct CompilerContext* program,usize fileIndex,usize index);
Status kek_module_AddDecl(struct CompilerContext* program,struct ModuleDecl* decl);
Status kek_module_AddParam(struct CompilerContext* program,struct ModuleParam* param);
Status kek_module_AddField(struct CompilerContext* program,struct ModuleField* field);
Status kek_module_ModuleError(struct CompilerContext* program,str message);
void kek_module_KeepFirstStatus(Status* first,Status status);
void kek_module_KeepError(struct CompilerContext* program,Status* first,str message);
bool kek_module_DeclHasFieldName(struct CompilerContext* program,struct ModuleDecl* decl,usize fileIndex,usize nameIndex);
void kek_module_InitNestedStructDecl(struct ModuleDecl* nested,struct ModuleField* field,usize firstField,struct ModuleDecl* parent);
bool kek_module_AttributeListHasTagged(struct CompilerContext* program,usize fileIndex,usize start,usize end);
bool kek_module_DeclHasTaggedAttribute(struct CompilerContext* program,usize fileIndex,usize start);
Status kek_module_ParseParams(struct CompilerContext* program,struct ModuleDecl* decl,usize fileIndex,usize start,usize end);
Status kek_module_ParseFields(struct CompilerContext* program,struct ModuleDecl* decl,usize fileIndex,usize start,usize end);
bool kek_module_ModuleDeclNameMatches(struct CompilerContext* program,struct ModuleDecl* decl,struct String name);
struct ModuleDecl* kek_module_FindTypeDecl(struct CompilerContext* program,struct String name);
Status kek_module_ParseAliasDecl(struct CompilerContext* program,usize fileIndex,usize start,usize* outEnd);
Status kek_module_ParseTypeDecl(struct CompilerContext* program,usize fileIndex,usize start,DeclKind kind,bool isTagged,usize* outEnd);
Status kek_module_ParseExternDecl(struct CompilerContext* program,usize fileIndex,usize start,usize* outEnd);
Status kek_module_ParseFunctionDecl(struct CompilerContext* program,usize fileIndex,usize start,usize* outEnd);
Status kek_module_ParseDeclarations(struct CompilerContext* program);
Status kek_module_BuildModule(struct CompilerContext* program);
struct String kek_syntax_TokenText(struct CompilerContext* program,usize fileIndex,usize tokenIndex);
bool kek_syntax_TokenEquals(struct CompilerContext* program,usize fileIndex,usize tokenIndex,str text);
bool kek_syntax_IsTokenKind(struct CompilerContext* program,usize fileIndex,usize tokenIndex,TokenKind kind);
bool kek_syntax_IsPunctuation(struct CompilerContext* program,usize fileIndex,usize tokenIndex,PunctuationKind kind);
bool kek_syntax_IsOperator(struct CompilerContext* program,usize fileIndex,usize tokenIndex,OperatorKind kind);
bool kek_syntax_IsKeyword(struct CompilerContext* program,usize fileIndex,usize tokenIndex,KeywordKind kind);
bool kek_syntax_IsIdentifierText(struct CompilerContext* program,usize fileIndex,usize tokenIndex,str text);
bool kek_syntax_IsEof(struct CompilerContext* program,usize fileIndex,usize tokenIndex);
usize kek_syntax_FileTokenCount(struct CompilerContext* program,usize fileIndex);
Status kek_syntax_Write(struct StringBuilder* out,str text);
Status kek_syntax_WriteString(struct StringBuilder* out,struct String text);
Status kek_syntax_WriteToken(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize tokenIndex);
Status kek_syntax_CloneContextCString(struct CompilerContext* program,str text,struct OwnedString* out);
Status kek_syntax_CloneContextString(struct CompilerContext* program,struct String text,struct OwnedString* out);
Status kek_syntax_DetachBuilder(struct StringBuilder* builder,struct OwnedString* out);
Status kek_syntax_MakeOwnedEmpty(struct CompilerContext* program,struct OwnedString* out);
usize kek_syntax_StringLastSlash(struct String path);
Status kek_syntax_PackageNameFromPath(struct String path,struct Allocator allocator,struct OwnedString* out);
Status kek_syntax_ModuleNameFromPath(struct String path,struct Allocator allocator,struct OwnedString* out);
Status kek_syntax_InitTokenFileSlot(struct Allocator allocator,struct SyntaxFile* tokenFile,usize fileIndex);
Status kek_syntax_SetTokenFileNames(struct Allocator allocator,struct SyntaxFile* tokenFile,bool isRoot);
Status kek_syntax_TokenizeSourceFile(struct Allocator allocator,struct SourceFile* sourceFile,struct SyntaxFile* tokenFile,usize fileIndex,bool isRoot,struct DiagnosticBag* diagnostics);
void kek_syntax_TokenizeJobRun(struct TokenizeJob* job);
ptr kek_syntax_TokenizeFileThreadEntry(ptr arg);
Status kek_syntax_MergeDiagnostics(struct DiagnosticBag* target,struct DiagnosticBag* source);
Status kek_syntax_ProgramInit(struct CompilerContext* program,struct Allocator allocator);
Status kek_syntax_ProgramDestroy(struct CompilerContext* program);
Status kek_syntax_AddDiagnostic(struct CompilerContext* program,str message);
Status kek_syntax_ReserveTokenFiles(struct CompilerContext* program,usize additional);
Status kek_syntax_ReserveDecls(struct CompilerContext* program,usize additional);
Status kek_syntax_ReserveParams(struct CompilerContext* program,usize additional);
Status kek_syntax_ReserveFields(struct CompilerContext* program,usize additional);
Status kek_syntax_ReserveTypeUses(struct CompilerContext* program,usize additional);
Status kek_syntax_ReserveFuncUses(struct CompilerContext* program,usize additional);
Status kek_syntax_ReserveTypeDeclIndex(struct CompilerContext* program,usize additional);
Status kek_syntax_ReserveFunctionDeclIndex(struct CompilerContext* program,usize additional);
Status kek_syntax_ReserveMethodDeclIndex(struct CompilerContext* program,usize additional);
Status kek_syntax_LoadAndTokenize(struct CompilerContext* program,str entryPath);
Status kek_syntax_LoadSyntaxPackage(str entryPath,struct CompilerContext* program);
Status std_thread_ThreadHandleStart(usize* handle,ptr entry,ptr arg);
Status std_thread_ThreadHandleJoin(usize* handle);
void std_mem_Free__byte(struct Allocator allocator,byte* data,usize count);
byte* std_mem_Alloc__byte(struct Allocator allocator,usize count);
byte* std_mem_Resize__byte(struct Allocator allocator,byte* oldData,usize oldCount,usize newCount);
struct Slice__byte std_core_FixedSlice__byte(byte* data,usize len);
struct Span__byte std_core_FixedSpan__byte(byte* data,usize len);
struct Result__String std_core_Ok__String(struct String value);
struct Result__String std_core_Err__String(Status status);
struct Array__Diagnostic std_array_ArrayNew__Diagnostic(struct Allocator allocator);
struct Array__SourceFile std_array_ArrayNew__SourceFile(struct Allocator allocator);
struct Array__Token std_array_ArrayNew__Token(struct Allocator allocator);
struct Array__AstNode std_array_ArrayNew__AstNode(struct Allocator allocator);
struct Array__usize std_array_ArrayNew__usize(struct Allocator allocator);
struct Array__DeferRange std_array_ArrayNew__DeferRange(struct Allocator allocator);
struct Array__Local std_array_ArrayNew__Local(struct Allocator allocator);
struct Array__OwnedString std_array_ArrayNew__OwnedString(struct Allocator allocator);
struct Array__TypeInfo std_array_ArrayNew__TypeInfo(struct Allocator allocator);
struct Array__SyntaxFile std_array_ArrayNew__SyntaxFile(struct Allocator allocator);
struct Array__ModuleDecl std_array_ArrayNew__ModuleDecl(struct Allocator allocator);
struct Array__ModuleParam std_array_ArrayNew__ModuleParam(struct Allocator allocator);
struct Array__ModuleField std_array_ArrayNew__ModuleField(struct Allocator allocator);
struct Array__TypeUse std_array_ArrayNew__TypeUse(struct Allocator allocator);
struct Array__FuncUse std_array_ArrayNew__FuncUse(struct Allocator allocator);
struct Array__DeclNameIndexEntry std_array_ArrayNew__DeclNameIndexEntry(struct Allocator allocator);
struct Array__MethodDeclIndexEntry std_array_ArrayNew__MethodDeclIndexEntry(struct Allocator allocator);
struct Array__TokenizeJob std_array_ArrayNew__TokenizeJob(struct Allocator allocator);
Status Array__Diagnostic_Destroy(struct Array__Diagnostic* this);
Status Array__Diagnostic_Reserve(struct Array__Diagnostic* this,usize additional);
Status Array__Diagnostic_Push(struct Array__Diagnostic* this,struct Diagnostic value);
struct Result__Diagnostic Array__Diagnostic_Pop(struct Array__Diagnostic* this);
struct Slice__Diagnostic Array__Diagnostic_Slice(struct Array__Diagnostic* this);
Status Array__SourceFile_Destroy(struct Array__SourceFile* this);
Status Array__SourceFile_Reserve(struct Array__SourceFile* this,usize additional);
Status Array__SourceFile_Push(struct Array__SourceFile* this,struct SourceFile value);
struct Result__SourceFile Array__SourceFile_Pop(struct Array__SourceFile* this);
struct Slice__SourceFile Array__SourceFile_Slice(struct Array__SourceFile* this);
Status Array__Token_Destroy(struct Array__Token* this);
Status Array__Token_Reserve(struct Array__Token* this,usize additional);
Status Array__Token_Push(struct Array__Token* this,struct Token value);
struct Result__Token Array__Token_Pop(struct Array__Token* this);
struct Slice__Token Array__Token_Slice(struct Array__Token* this);
Status Array__AstNode_Destroy(struct Array__AstNode* this);
Status Array__AstNode_Reserve(struct Array__AstNode* this,usize additional);
Status Array__AstNode_Push(struct Array__AstNode* this,struct AstNode value);
struct Result__AstNode Array__AstNode_Pop(struct Array__AstNode* this);
struct Slice__AstNode Array__AstNode_Slice(struct Array__AstNode* this);
Status Array__usize_Destroy(struct Array__usize* this);
Status Array__usize_Reserve(struct Array__usize* this,usize additional);
Status Array__usize_Push(struct Array__usize* this,usize value);
struct Result__usize Array__usize_Pop(struct Array__usize* this);
struct Slice__usize Array__usize_Slice(struct Array__usize* this);
Status Array__DeferRange_Destroy(struct Array__DeferRange* this);
Status Array__DeferRange_Reserve(struct Array__DeferRange* this,usize additional);
Status Array__DeferRange_Push(struct Array__DeferRange* this,struct DeferRange value);
struct Result__DeferRange Array__DeferRange_Pop(struct Array__DeferRange* this);
struct Slice__DeferRange Array__DeferRange_Slice(struct Array__DeferRange* this);
Status Array__Local_Destroy(struct Array__Local* this);
Status Array__Local_Reserve(struct Array__Local* this,usize additional);
Status Array__Local_Push(struct Array__Local* this,struct Local value);
struct Result__Local Array__Local_Pop(struct Array__Local* this);
struct Slice__Local Array__Local_Slice(struct Array__Local* this);
Status Array__DeclNameIndexEntry_Destroy(struct Array__DeclNameIndexEntry* this);
Status Array__DeclNameIndexEntry_Reserve(struct Array__DeclNameIndexEntry* this,usize additional);
Status Array__DeclNameIndexEntry_Push(struct Array__DeclNameIndexEntry* this,struct DeclNameIndexEntry value);
struct Result__DeclNameIndexEntry Array__DeclNameIndexEntry_Pop(struct Array__DeclNameIndexEntry* this);
struct Slice__DeclNameIndexEntry Array__DeclNameIndexEntry_Slice(struct Array__DeclNameIndexEntry* this);
Status Array__MethodDeclIndexEntry_Destroy(struct Array__MethodDeclIndexEntry* this);
Status Array__MethodDeclIndexEntry_Reserve(struct Array__MethodDeclIndexEntry* this,usize additional);
Status Array__MethodDeclIndexEntry_Push(struct Array__MethodDeclIndexEntry* this,struct MethodDeclIndexEntry value);
struct Result__MethodDeclIndexEntry Array__MethodDeclIndexEntry_Pop(struct Array__MethodDeclIndexEntry* this);
struct Slice__MethodDeclIndexEntry Array__MethodDeclIndexEntry_Slice(struct Array__MethodDeclIndexEntry* this);
Status Array__OwnedString_Destroy(struct Array__OwnedString* this);
Status Array__OwnedString_Reserve(struct Array__OwnedString* this,usize additional);
Status Array__OwnedString_Push(struct Array__OwnedString* this,struct OwnedString value);
struct Result__OwnedString Array__OwnedString_Pop(struct Array__OwnedString* this);
struct Slice__OwnedString Array__OwnedString_Slice(struct Array__OwnedString* this);
Status Array__TypeInfo_Destroy(struct Array__TypeInfo* this);
Status Array__TypeInfo_Reserve(struct Array__TypeInfo* this,usize additional);
Status Array__TypeInfo_Push(struct Array__TypeInfo* this,struct TypeInfo value);
struct Result__TypeInfo Array__TypeInfo_Pop(struct Array__TypeInfo* this);
struct Slice__TypeInfo Array__TypeInfo_Slice(struct Array__TypeInfo* this);
Status Array__SyntaxFile_Destroy(struct Array__SyntaxFile* this);
Status Array__SyntaxFile_Reserve(struct Array__SyntaxFile* this,usize additional);
Status Array__SyntaxFile_Push(struct Array__SyntaxFile* this,struct SyntaxFile value);
struct Result__SyntaxFile Array__SyntaxFile_Pop(struct Array__SyntaxFile* this);
struct Slice__SyntaxFile Array__SyntaxFile_Slice(struct Array__SyntaxFile* this);
Status Array__ModuleDecl_Destroy(struct Array__ModuleDecl* this);
Status Array__ModuleDecl_Reserve(struct Array__ModuleDecl* this,usize additional);
Status Array__ModuleDecl_Push(struct Array__ModuleDecl* this,struct ModuleDecl value);
struct Result__ModuleDecl Array__ModuleDecl_Pop(struct Array__ModuleDecl* this);
struct Slice__ModuleDecl Array__ModuleDecl_Slice(struct Array__ModuleDecl* this);
Status Array__ModuleParam_Destroy(struct Array__ModuleParam* this);
Status Array__ModuleParam_Reserve(struct Array__ModuleParam* this,usize additional);
Status Array__ModuleParam_Push(struct Array__ModuleParam* this,struct ModuleParam value);
struct Result__ModuleParam Array__ModuleParam_Pop(struct Array__ModuleParam* this);
struct Slice__ModuleParam Array__ModuleParam_Slice(struct Array__ModuleParam* this);
Status Array__ModuleField_Destroy(struct Array__ModuleField* this);
Status Array__ModuleField_Reserve(struct Array__ModuleField* this,usize additional);
Status Array__ModuleField_Push(struct Array__ModuleField* this,struct ModuleField value);
struct Result__ModuleField Array__ModuleField_Pop(struct Array__ModuleField* this);
struct Slice__ModuleField Array__ModuleField_Slice(struct Array__ModuleField* this);
Status Array__TypeUse_Destroy(struct Array__TypeUse* this);
Status Array__TypeUse_Reserve(struct Array__TypeUse* this,usize additional);
Status Array__TypeUse_Push(struct Array__TypeUse* this,struct TypeUse value);
struct Result__TypeUse Array__TypeUse_Pop(struct Array__TypeUse* this);
struct Slice__TypeUse Array__TypeUse_Slice(struct Array__TypeUse* this);
Status Array__FuncUse_Destroy(struct Array__FuncUse* this);
Status Array__FuncUse_Reserve(struct Array__FuncUse* this,usize additional);
Status Array__FuncUse_Push(struct Array__FuncUse* this,struct FuncUse value);
struct Result__FuncUse Array__FuncUse_Pop(struct Array__FuncUse* this);
struct Slice__FuncUse Array__FuncUse_Slice(struct Array__FuncUse* this);
Status Array__TokenizeJob_Destroy(struct Array__TokenizeJob* this);
Status Array__TokenizeJob_Reserve(struct Array__TokenizeJob* this,usize additional);
Status Array__TokenizeJob_Push(struct Array__TokenizeJob* this,struct TokenizeJob value);
struct Result__TokenizeJob Array__TokenizeJob_Pop(struct Array__TokenizeJob* this);
struct Slice__TokenizeJob Array__TokenizeJob_Slice(struct Array__TokenizeJob* this);

void std_mem_Free__byte(struct Allocator allocator,byte* data,usize count) {
    if (allocator.context!=0) {
    }
    if (count==0) {
    }
    kek_std_free(data);
}
byte* std_mem_Alloc__byte(struct Allocator allocator,usize count) {
    if (allocator.context!=0) {
    }
    return (kek_std_alloc(count*sizeof(byte)));
}
byte* std_mem_Resize__byte(struct Allocator allocator,byte* oldData,usize oldCount,usize newCount) {
    if (allocator.context!=0) {
    }
    if (oldCount==0) {
    }
    return (kek_std_resize(oldData,newCount*sizeof(byte)));
}
struct Slice__byte std_core_FixedSlice__byte(byte* data,usize len) {
    struct Slice__byte out={0};
    out.data=data;
    out.len=len;
    return (out);
}
struct Span__byte std_core_FixedSpan__byte(byte* data,usize len) {
    struct Span__byte out={0};
    out.data=data;
    out.len=len;
    return (out);
}
struct Result__String std_core_Ok__String(struct String value) {
    struct Result__String out={0};
    out.status=Status_Ok;
    out.value=value;
    return (out);
}
struct Result__String std_core_Err__String(Status status) {
    struct Result__String out={0};
    out.status=status;
    return (out);
}
struct Array__Diagnostic std_array_ArrayNew__Diagnostic(struct Allocator allocator) {
    struct Array__Diagnostic array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__SourceFile std_array_ArrayNew__SourceFile(struct Allocator allocator) {
    struct Array__SourceFile array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__Token std_array_ArrayNew__Token(struct Allocator allocator) {
    struct Array__Token array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__AstNode std_array_ArrayNew__AstNode(struct Allocator allocator) {
    struct Array__AstNode array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__usize std_array_ArrayNew__usize(struct Allocator allocator) {
    struct Array__usize array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__DeferRange std_array_ArrayNew__DeferRange(struct Allocator allocator) {
    struct Array__DeferRange array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__Local std_array_ArrayNew__Local(struct Allocator allocator) {
    struct Array__Local array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__OwnedString std_array_ArrayNew__OwnedString(struct Allocator allocator) {
    struct Array__OwnedString array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__TypeInfo std_array_ArrayNew__TypeInfo(struct Allocator allocator) {
    struct Array__TypeInfo array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__SyntaxFile std_array_ArrayNew__SyntaxFile(struct Allocator allocator) {
    struct Array__SyntaxFile array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__ModuleDecl std_array_ArrayNew__ModuleDecl(struct Allocator allocator) {
    struct Array__ModuleDecl array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__ModuleParam std_array_ArrayNew__ModuleParam(struct Allocator allocator) {
    struct Array__ModuleParam array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__ModuleField std_array_ArrayNew__ModuleField(struct Allocator allocator) {
    struct Array__ModuleField array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__TypeUse std_array_ArrayNew__TypeUse(struct Allocator allocator) {
    struct Array__TypeUse array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__FuncUse std_array_ArrayNew__FuncUse(struct Allocator allocator) {
    struct Array__FuncUse array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__DeclNameIndexEntry std_array_ArrayNew__DeclNameIndexEntry(struct Allocator allocator) {
    struct Array__DeclNameIndexEntry array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__MethodDeclIndexEntry std_array_ArrayNew__MethodDeclIndexEntry(struct Allocator allocator) {
    struct Array__MethodDeclIndexEntry array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
struct Array__TokenizeJob std_array_ArrayNew__TokenizeJob(struct Allocator allocator) {
    struct Array__TokenizeJob array={0};
    array.data=0;
    array.len=0;
    array.cap=0;
    array.allocator=allocator;
    return (array);
}
Status Array__Diagnostic_Destroy(struct Array__Diagnostic* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__Diagnostic_Reserve(struct Array__Diagnostic* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct Diagnostic* newData=(struct Diagnostic*)kek_std_resize(
        this->data,newCap*sizeof(struct Diagnostic));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__Diagnostic_Push(struct Array__Diagnostic* this,struct Diagnostic value) {
    Status status=Array__Diagnostic_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__Diagnostic Array__Diagnostic_Pop(struct Array__Diagnostic* this) {
    struct Result__Diagnostic result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__Diagnostic Array__Diagnostic_Slice(struct Array__Diagnostic* this) {
    struct Slice__Diagnostic out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__SourceFile_Destroy(struct Array__SourceFile* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__SourceFile_Reserve(struct Array__SourceFile* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct SourceFile* newData=(struct SourceFile*)kek_std_resize(
        this->data,newCap*sizeof(struct SourceFile));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__SourceFile_Push(struct Array__SourceFile* this,struct SourceFile value) {
    Status status=Array__SourceFile_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__SourceFile Array__SourceFile_Pop(struct Array__SourceFile* this) {
    struct Result__SourceFile result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__SourceFile Array__SourceFile_Slice(struct Array__SourceFile* this) {
    struct Slice__SourceFile out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__Token_Destroy(struct Array__Token* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__Token_Reserve(struct Array__Token* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct Token* newData=(struct Token*)kek_std_resize(
        this->data,newCap*sizeof(struct Token));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__Token_Push(struct Array__Token* this,struct Token value) {
    Status status=Array__Token_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__Token Array__Token_Pop(struct Array__Token* this) {
    struct Result__Token result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__Token Array__Token_Slice(struct Array__Token* this) {
    struct Slice__Token out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__AstNode_Destroy(struct Array__AstNode* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__AstNode_Reserve(struct Array__AstNode* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct AstNode* newData=(struct AstNode*)kek_std_resize(
        this->data,newCap*sizeof(struct AstNode));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__AstNode_Push(struct Array__AstNode* this,struct AstNode value) {
    Status status=Array__AstNode_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__AstNode Array__AstNode_Pop(struct Array__AstNode* this) {
    struct Result__AstNode result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__AstNode Array__AstNode_Slice(struct Array__AstNode* this) {
    struct Slice__AstNode out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__usize_Destroy(struct Array__usize* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__usize_Reserve(struct Array__usize* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    usize* newData=(usize*)kek_std_resize(
        this->data,newCap*sizeof(usize));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__usize_Push(struct Array__usize* this,usize value) {
    Status status=Array__usize_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__usize Array__usize_Pop(struct Array__usize* this) {
    struct Result__usize result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__usize Array__usize_Slice(struct Array__usize* this) {
    struct Slice__usize out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__DeferRange_Destroy(struct Array__DeferRange* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__DeferRange_Reserve(struct Array__DeferRange* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct DeferRange* newData=(struct DeferRange*)kek_std_resize(
        this->data,newCap*sizeof(struct DeferRange));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__DeferRange_Push(struct Array__DeferRange* this,struct DeferRange value) {
    Status status=Array__DeferRange_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__DeferRange Array__DeferRange_Pop(struct Array__DeferRange* this) {
    struct Result__DeferRange result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__DeferRange Array__DeferRange_Slice(struct Array__DeferRange* this) {
    struct Slice__DeferRange out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__Local_Destroy(struct Array__Local* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__Local_Reserve(struct Array__Local* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct Local* newData=(struct Local*)kek_std_resize(
        this->data,newCap*sizeof(struct Local));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__Local_Push(struct Array__Local* this,struct Local value) {
    Status status=Array__Local_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__Local Array__Local_Pop(struct Array__Local* this) {
    struct Result__Local result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__Local Array__Local_Slice(struct Array__Local* this) {
    struct Slice__Local out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__DeclNameIndexEntry_Destroy(struct Array__DeclNameIndexEntry* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__DeclNameIndexEntry_Reserve(struct Array__DeclNameIndexEntry* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct DeclNameIndexEntry* newData=(struct DeclNameIndexEntry*)kek_std_resize(
        this->data,newCap*sizeof(struct DeclNameIndexEntry));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__DeclNameIndexEntry_Push(struct Array__DeclNameIndexEntry* this,struct DeclNameIndexEntry value) {
    Status status=Array__DeclNameIndexEntry_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__DeclNameIndexEntry Array__DeclNameIndexEntry_Pop(struct Array__DeclNameIndexEntry* this) {
    struct Result__DeclNameIndexEntry result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__DeclNameIndexEntry Array__DeclNameIndexEntry_Slice(struct Array__DeclNameIndexEntry* this) {
    struct Slice__DeclNameIndexEntry out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__MethodDeclIndexEntry_Destroy(struct Array__MethodDeclIndexEntry* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__MethodDeclIndexEntry_Reserve(struct Array__MethodDeclIndexEntry* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct MethodDeclIndexEntry* newData=(struct MethodDeclIndexEntry*)kek_std_resize(
        this->data,newCap*sizeof(struct MethodDeclIndexEntry));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__MethodDeclIndexEntry_Push(struct Array__MethodDeclIndexEntry* this,struct MethodDeclIndexEntry value) {
    Status status=Array__MethodDeclIndexEntry_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__MethodDeclIndexEntry Array__MethodDeclIndexEntry_Pop(struct Array__MethodDeclIndexEntry* this) {
    struct Result__MethodDeclIndexEntry result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__MethodDeclIndexEntry Array__MethodDeclIndexEntry_Slice(struct Array__MethodDeclIndexEntry* this) {
    struct Slice__MethodDeclIndexEntry out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__OwnedString_Destroy(struct Array__OwnedString* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__OwnedString_Reserve(struct Array__OwnedString* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct OwnedString* newData=(struct OwnedString*)kek_std_resize(
        this->data,newCap*sizeof(struct OwnedString));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__OwnedString_Push(struct Array__OwnedString* this,struct OwnedString value) {
    Status status=Array__OwnedString_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__OwnedString Array__OwnedString_Pop(struct Array__OwnedString* this) {
    struct Result__OwnedString result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__OwnedString Array__OwnedString_Slice(struct Array__OwnedString* this) {
    struct Slice__OwnedString out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__TypeInfo_Destroy(struct Array__TypeInfo* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__TypeInfo_Reserve(struct Array__TypeInfo* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct TypeInfo* newData=(struct TypeInfo*)kek_std_resize(
        this->data,newCap*sizeof(struct TypeInfo));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__TypeInfo_Push(struct Array__TypeInfo* this,struct TypeInfo value) {
    Status status=Array__TypeInfo_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__TypeInfo Array__TypeInfo_Pop(struct Array__TypeInfo* this) {
    struct Result__TypeInfo result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__TypeInfo Array__TypeInfo_Slice(struct Array__TypeInfo* this) {
    struct Slice__TypeInfo out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__SyntaxFile_Destroy(struct Array__SyntaxFile* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__SyntaxFile_Reserve(struct Array__SyntaxFile* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct SyntaxFile* newData=(struct SyntaxFile*)kek_std_resize(
        this->data,newCap*sizeof(struct SyntaxFile));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__SyntaxFile_Push(struct Array__SyntaxFile* this,struct SyntaxFile value) {
    Status status=Array__SyntaxFile_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__SyntaxFile Array__SyntaxFile_Pop(struct Array__SyntaxFile* this) {
    struct Result__SyntaxFile result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__SyntaxFile Array__SyntaxFile_Slice(struct Array__SyntaxFile* this) {
    struct Slice__SyntaxFile out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__ModuleDecl_Destroy(struct Array__ModuleDecl* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__ModuleDecl_Reserve(struct Array__ModuleDecl* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct ModuleDecl* newData=(struct ModuleDecl*)kek_std_resize(
        this->data,newCap*sizeof(struct ModuleDecl));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__ModuleDecl_Push(struct Array__ModuleDecl* this,struct ModuleDecl value) {
    Status status=Array__ModuleDecl_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__ModuleDecl Array__ModuleDecl_Pop(struct Array__ModuleDecl* this) {
    struct Result__ModuleDecl result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__ModuleDecl Array__ModuleDecl_Slice(struct Array__ModuleDecl* this) {
    struct Slice__ModuleDecl out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__ModuleParam_Destroy(struct Array__ModuleParam* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__ModuleParam_Reserve(struct Array__ModuleParam* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct ModuleParam* newData=(struct ModuleParam*)kek_std_resize(
        this->data,newCap*sizeof(struct ModuleParam));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__ModuleParam_Push(struct Array__ModuleParam* this,struct ModuleParam value) {
    Status status=Array__ModuleParam_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__ModuleParam Array__ModuleParam_Pop(struct Array__ModuleParam* this) {
    struct Result__ModuleParam result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__ModuleParam Array__ModuleParam_Slice(struct Array__ModuleParam* this) {
    struct Slice__ModuleParam out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__ModuleField_Destroy(struct Array__ModuleField* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__ModuleField_Reserve(struct Array__ModuleField* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct ModuleField* newData=(struct ModuleField*)kek_std_resize(
        this->data,newCap*sizeof(struct ModuleField));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__ModuleField_Push(struct Array__ModuleField* this,struct ModuleField value) {
    Status status=Array__ModuleField_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__ModuleField Array__ModuleField_Pop(struct Array__ModuleField* this) {
    struct Result__ModuleField result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__ModuleField Array__ModuleField_Slice(struct Array__ModuleField* this) {
    struct Slice__ModuleField out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__TypeUse_Destroy(struct Array__TypeUse* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__TypeUse_Reserve(struct Array__TypeUse* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct TypeUse* newData=(struct TypeUse*)kek_std_resize(
        this->data,newCap*sizeof(struct TypeUse));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__TypeUse_Push(struct Array__TypeUse* this,struct TypeUse value) {
    Status status=Array__TypeUse_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__TypeUse Array__TypeUse_Pop(struct Array__TypeUse* this) {
    struct Result__TypeUse result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__TypeUse Array__TypeUse_Slice(struct Array__TypeUse* this) {
    struct Slice__TypeUse out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__FuncUse_Destroy(struct Array__FuncUse* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__FuncUse_Reserve(struct Array__FuncUse* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct FuncUse* newData=(struct FuncUse*)kek_std_resize(
        this->data,newCap*sizeof(struct FuncUse));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__FuncUse_Push(struct Array__FuncUse* this,struct FuncUse value) {
    Status status=Array__FuncUse_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__FuncUse Array__FuncUse_Pop(struct Array__FuncUse* this) {
    struct Result__FuncUse result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__FuncUse Array__FuncUse_Slice(struct Array__FuncUse* this) {
    struct Slice__FuncUse out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
Status Array__TokenizeJob_Destroy(struct Array__TokenizeJob* this) {
    if(this->data!=0){
        kek_std_free(this->data);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return Status_Ok;
}
Status Array__TokenizeJob_Reserve(struct Array__TokenizeJob* this,usize additional) {
    usize needed=this->len+additional;
    if(needed<=this->cap){return Status_Ok;}
    usize newCap=this->cap;
    if(newCap==0){newCap=8;}
    while(newCap<needed){newCap=newCap*2;}
    struct TokenizeJob* newData=(struct TokenizeJob*)kek_std_resize(
        this->data,newCap*sizeof(struct TokenizeJob));
    if(newData==0){return Status_NoMemory;}
    this->data=newData;
    this->cap=newCap;
    return Status_Ok;
}
Status Array__TokenizeJob_Push(struct Array__TokenizeJob* this,struct TokenizeJob value) {
    Status status=Array__TokenizeJob_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
struct Result__TokenizeJob Array__TokenizeJob_Pop(struct Array__TokenizeJob* this) {
    struct Result__TokenizeJob result={0};
    if(this->len==0){result.status=Status_End;return result;}
    this->len-=1;
    result.status=Status_Ok;
    result.value=this->data[this->len];
    return result;
}
struct Slice__TokenizeJob Array__TokenizeJob_Slice(struct Array__TokenizeJob* this) {
    struct Slice__TokenizeJob out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
int main(int argc,str* argv) {
    return (kek_compiler_CompilerMain(argc,argv));
}
struct Allocator std_mem_DefaultAllocator(void) {
    return ((struct Allocator){.context=0});
}
struct String std_string_StringFromCString(str text) {
    struct String out={0};
    usize count=0;
    while (text[count]!=0) {
        count+=1;
    }
    out.data=((ptr)(text));
    out.len=count;
    return (out);
}
struct Slice__byte String_Bytes(struct String* this) {
    struct Slice__byte out={0};
    out.data=this->data;
    out.len=this->len;
    return (out);
}
struct String String_Slice(struct String* this,usize start,usize length) {
    struct String out={0};
    if (start>=this->len) {
        out.data=this->data+this->len;
        out.len=0;
        return (out);
    }
    usize available=this->len-start;
    if (length>available) {
        length=available;
    }
    out.data=this->data+start;
    out.len=length;
    return (out);
}
bool String_Equals(struct String* this,struct String other) {
    if (this->len!=other.len) {
        return (0);
    }
    for (usize i=0;i<this->len;i++) {
        if (this->data[i]!=other.data[i]) {
            return (0);
        }
    }
    return (1);
}
bool String_EqualsCString(struct String* this,str text) {
    return (String_Equals(this,std_string_StringFromCString(text)));
}
int String_Compare(struct String* this,struct String other) {
    usize limit=this->len;
    if (other.len<limit) {
        limit=other.len;
    }
    for (usize i=0;i<limit;i++) {
        if (this->data[i]<other.data[i]) {
            return (-1);
        }
        if (this->data[i]>other.data[i]) {
            return (1);
        }
    }
    if (this->len<other.len) {
        return (-1);
    }
    if (this->len>other.len) {
        return (1);
    }
    return (0);
}
struct Result__usize String_FindByte(struct String* this,byte value) {
    struct Result__usize result={0};
    for (usize i=0;i<this->len;i++) {
        if (this->data[i]==value) {
            result.status=Status_Ok;
            result.value=i;
            return (result);
        }
    }
    result.status=Status_NotFound;
    return (result);
}
bool String_ContainsByte(struct String* this,byte value) {
    struct Result__usize found=String_FindByte(this,value);
    return (found.status==Status_Ok);
}
bool String_StartsWith(struct String* this,struct String prefix) {
    if (prefix.len>this->len) {
        return (0);
    }
    for (usize i=0;i<prefix.len;i++) {
        if (this->data[i]!=prefix.data[i]) {
            return (0);
        }
    }
    return (1);
}
bool String_EndsWith(struct String* this,struct String suffix) {
    if (suffix.len>this->len) {
        return (0);
    }
    usize offset=this->len-suffix.len;
    for (usize i=0;i<suffix.len;i++) {
        if (this->data[offset+i]!=suffix.data[i]) {
            return (0);
        }
    }
    return (1);
}
bool std_string_IsAsciiSpace(byte c) {
    return (c==32||c==9||c==10||c==13);
}
bool std_string_IsAsciiAlpha(byte c) {
    return ((c>=65&&c<=90)||(c>=97&&c<=122)||c==95);
}
bool std_string_IsAsciiDigit(byte c) {
    return (c>=48&&c<=57);
}
bool std_string_IsAsciiWord(byte c) {
    return (std_string_IsAsciiAlpha(c)||std_string_IsAsciiDigit(c));
}
struct String OwnedString_View(struct OwnedString* this) {
    struct String out={0};
    out.data=this->data;
    out.len=this->len;
    return (out);
}
struct String std_string_OwnedStringView(struct OwnedString* owned) {
    struct String out={0};
    out.data=owned->data;
    out.len=owned->len;
    return (out);
}
Status std_string_DestroyOwnedString(struct OwnedString* owned) {
    if (owned->data!=0) {
        std_mem_Free__byte(owned->allocator,owned->data,owned->cap);
    }
    owned->data=0;
    owned->len=0;
    owned->cap=0;
    return (Status_Ok);
}
Status std_string_CloneString(struct String text,struct Allocator allocator,struct OwnedString* out) {
    byte* data=std_mem_Alloc__byte(allocator,text.len+1);
    if (data==0) {
        return (Status_NoMemory);
    }
    for (usize i=0;i<text.len;i++) {
        data[i]=text.data[i];
    }
    data[text.len]=0;
    out->data=data;
    out->len=text.len;
    out->cap=text.len+1;
    out->allocator=allocator;
    return (Status_Ok);
}
Status std_string_CloneCString(str text,struct Allocator allocator,struct OwnedString* out) {
    return (std_string_CloneString(std_string_StringFromCString(text),allocator,out));
}
Status OwnedString_Destroy(struct OwnedString* this) {
    if (this->data!=0) {
        std_mem_Free__byte(this->allocator,this->data,this->cap);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return (Status_Ok);
}
struct StringBuilder std_string_StringBuilderNew(struct Allocator allocator) {
    struct StringBuilder builder={0};
    builder.data=0;
    builder.len=0;
    builder.cap=0;
    builder.allocator=allocator;
    return (builder);
}
Status StringBuilder_Destroy(struct StringBuilder* this) {
    if (this->data!=0) {
        std_mem_Free__byte(this->allocator,this->data,this->cap);
    }
    this->data=0;
    this->len=0;
    this->cap=0;
    return (Status_Ok);
}
Status StringBuilder_Reserve(struct StringBuilder* this,usize additional) {
    usize needed=this->len+additional;
    if (needed<=this->cap) {
        return (Status_Ok);
    }
    usize newCap=this->cap;
    if (newCap==0) {
        newCap=16;
    }
    while (newCap<needed) {
        newCap=newCap*2;
    }
    byte* newData=std_mem_Resize__byte(this->allocator,this->data,this->cap,newCap);
    if (newData==0) {
        return (Status_NoMemory);
    }
    this->data=newData;
    this->cap=newCap;
    return (Status_Ok);
}
struct Result__usize StringBuilder_Write(struct StringBuilder* this,struct Slice__byte data) {
    struct Result__usize result={0};
    Status status=StringBuilder_Reserve(this,data.len);
    if (status!=Status_Ok) {
        result.status=status;
        return (result);
    }
    for (usize i=0;i<data.len;i++) {
        this->data[this->len+i]=data.data[i];
    }
    this->len+=data.len;
    result.status=Status_Ok;
    result.value=data.len;
    return (result);
}
Status StringBuilder_WriteByte(struct StringBuilder* this,byte value) {
    byte buffer[1]={value};
    struct Result__usize result=StringBuilder_Write(this,std_core_FixedSlice__byte(buffer,1));
    return (result.status);
}
Status StringBuilder_WriteString(struct StringBuilder* this,struct String text) {
    struct Result__usize result=StringBuilder_Write(this,String_Bytes(&text));
    return (result.status);
}
Status StringBuilder_WriteCString(struct StringBuilder* this,str text) {
    return (StringBuilder_WriteString(this,std_string_StringFromCString(text)));
}
Status StringBuilder_WriteRepeatByte(struct StringBuilder* this,byte value,usize count) {
    for (usize i=0;i<count;i++) {
        Status status=StringBuilder_WriteByte(this,value);
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
Status StringBuilder_WriteIndent(struct StringBuilder* this,usize count) {
    return (StringBuilder_WriteRepeatByte(this,' ',count));
}
struct String StringBuilder_View(struct StringBuilder* this) {
    struct String out={0};
    out.data=this->data;
    out.len=this->len;
    return (out);
}
struct Result__OwnedString StringBuilder_Detach(struct StringBuilder* this) {
    struct OwnedString out={0};
    out.data=this->data;
    out.len=this->len;
    out.cap=this->cap;
    out.allocator=this->allocator;
    this->data=0;
    this->len=0;
    this->cap=0;
    struct Result__OwnedString result={0};
    result.status=Status_Ok;
    result.value=out;
    return (result);
}
struct ByteCursor std_scan_ByteCursorNew(struct String input) {
    struct ByteCursor cursor={0};
    cursor.input=input;
    cursor.pos=0;
    cursor.line=1;
    cursor.column=1;
    return (cursor);
}
bool ByteCursor_AtEnd(struct ByteCursor* this) {
    return (this->pos>=this->input.len);
}
byte ByteCursor_Peek(struct ByteCursor* this) {
    if (ByteCursor_AtEnd(this)) {
        return (0);
    }
    return (this->input.data[this->pos]);
}
byte ByteCursor_PeekAt(struct ByteCursor* this,usize offset) {
    usize target=this->pos+offset;
    if (target>=this->input.len) {
        return (0);
    }
    return (this->input.data[target]);
}
byte ByteCursor_Advance(struct ByteCursor* this) {
    byte value=ByteCursor_Peek(this);
    if (ByteCursor_AtEnd(this)) {
        return (0);
    }
    this->pos+=1;
    if (value=='\n') {
        this->line+=1;
        this->column=1;
    } else {
        this->column+=1;
    }
    return (value);
}
Status ByteCursor_SkipAsciiWhitespace(struct ByteCursor* this) {
    while (!ByteCursor_AtEnd(this)&&std_string_IsAsciiSpace(ByteCursor_Peek(this))) {
        ByteCursor_Advance(this);
    }
    return (Status_Ok);
}
struct Result__usize MemoryReader_Read(struct MemoryReader* this,struct Span__byte out) {
    struct Result__usize result={0};
    if (this->pos>=this->len) {
        result.status=Status_End;
        return (result);
    }
    usize remaining=this->len-this->pos;
    usize amount=out.len;
    if (amount>remaining) {
        amount=remaining;
    }
    for (usize i=0;i<amount;i++) {
        out.data[i]=this->data[this->pos+i];
    }
    this->pos+=amount;
    result.status=Status_Ok;
    result.value=amount;
    return (result);
}
struct Result__usize MemoryWriter_Write(struct MemoryWriter* this,struct Slice__byte data) {
    struct Result__usize result={0};
    usize remaining=this->len-this->pos;
    if (data.len>remaining) {
        result.status=Status_NoMemory;
        result.value=this->pos;
        return (result);
    }
    for (usize i=0;i<data.len;i++) {
        this->data[this->pos+i]=data.data[i];
    }
    this->pos+=data.len;
    result.status=Status_Ok;
    result.value=data.len;
    return (result);
}
struct Result__File std_file_FileOpen(str path,FileMode mode) {
    struct Result__File result={0};
    str modeText="rb";
    if (mode==FileMode_Write) {
        modeText="wb";
    } else {
        if (mode==FileMode_Append) {
            modeText="ab";
        } else {
            if (mode==FileMode_ReadWrite) {
                modeText="r+b";
            }
        }
    }
    RawHandle handle=kek_std_file_open(path,modeText);
    if (handle==0) {
        result.status=Status_NotFound;
        return (result);
    }
    struct File file={0};
    file.handle=handle;
    file.owned=1;
    result.status=Status_Ok;
    result.value=file;
    return (result);
}
struct Result__usize File_Read(struct File* this,struct Span__byte out) {
    struct Result__usize result={0};
    if (this->handle==0) {
        result.status=Status_Invalid;
        return (result);
    }
    usize read=kek_std_file_read(out.data,1,out.len,this->handle);
    if (read==0) {
        result.status=Status_End;
        return (result);
    }
    result.status=Status_Ok;
    result.value=read;
    return (result);
}
struct Result__usize File_Write(struct File* this,struct Slice__byte data) {
    struct Result__usize result={0};
    if (this->handle==0) {
        result.status=Status_Invalid;
        return (result);
    }
    usize written=kek_std_file_write(data.data,1,data.len,this->handle);
    if (written!=data.len) {
        result.status=Status_IoError;
        result.value=written;
        return (result);
    }
    result.status=Status_Ok;
    result.value=written;
    return (result);
}
Status std_file_WriteFile(str path,struct String text) {
    struct Result__File opened=std_file_FileOpen(path,FileMode_Write);
    if (opened.status!=Status_Ok) {
        return (opened.status);
    }
    struct File file=opened.value;
    struct Result__usize write=File_Write(&file,String_Bytes(&text));
    Status closeStatus=File_Close(&file);
    if (write.status!=Status_Ok) {
        return (write.status);
    }
    return (closeStatus);
}
Status File_Close(struct File* this) {
    if (this->handle==0) {
        return (Status_Invalid);
    }
    if (!this->owned) {
        return (Status_Unsupported);
    }
    if (kek_std_file_close(this->handle)!=0) {
        return (Status_IoError);
    }
    this->handle=0;
    this->owned=0;
    return (Status_Ok);
}
struct File std_file_Stdout(void) {
    struct File file={0};
    file.handle=kek_std_stdout();
    file.owned=0;
    return (file);
}
struct File std_file_Stderr(void) {
    struct File file={0};
    file.handle=kek_std_stderr();
    file.owned=0;
    return (file);
}
Status std_file_ReadAllToOwnedString(struct File file,struct Allocator allocator,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(allocator);
    byte buffer[4096]={0};
    while (1) {
        struct Result__usize read=File_Read(&file,std_core_FixedSpan__byte(buffer,((void)(buffer),4096)));
        if (read.status==Status_End) {
            break;
        }
        if (read.status!=Status_Ok) {
            StringBuilder_Destroy(&builder);
            return (read.status);
        }
        struct Result__usize write=StringBuilder_Write(&builder,std_core_FixedSlice__byte(buffer,read.value));
        if (write.status!=Status_Ok) {
            StringBuilder_Destroy(&builder);
            return (write.status);
        }
    }
    out->data=builder.data;
    out->len=builder.len;
    out->cap=builder.cap;
    out->allocator=builder.allocator;
    builder.data=0;
    builder.len=0;
    builder.cap=0;
    return (Status_Ok);
}
Status Directory_Close(struct Directory* this) {
    if (this->handle==0) {
        return (Status_Invalid);
    }
    if (kek_std_dir_close(this->handle)!=0) {
        return (Status_IoError);
    }
    this->handle=0;
    return (Status_Ok);
}
Status std_format_WriteByteToBuilder(struct StringBuilder* writer,byte value) {
    byte buffer[1]={value};
    struct Result__usize result=StringBuilder_Write(writer,std_core_FixedSlice__byte(buffer,1));
    return (result.status);
}
Status std_format_WriteStringToFile(struct File* writer,struct String text) {
    struct Result__usize result=File_Write(writer,String_Bytes(&text));
    return (result.status);
}
Status std_format_FormatU64ToBuilder(struct StringBuilder* writer,u64 value,u8 base) {
    byte digits[16]={48,49,50,51,52,53,54,55,56,57,65,66,67,68,69,70,};
    byte buffer[64]={0};
    usize len=0;
    u64 radix=base;
    if (radix<2) {
        return (Status_Invalid);
    }
    if (radix>16) {
        return (Status_Invalid);
    }
    if (value==0) {
        return (std_format_WriteByteToBuilder(writer,48));
    }
    while (value>0) {
        u64 digit=value%radix;
        buffer[len]=digits[digit];
        len+=1;
        value=value/radix;
    }
    while (len>0) {
        len-=1;
        Status status=std_format_WriteByteToBuilder(writer,buffer[len]);
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
Status std_format_FormatI64ToBuilder(struct StringBuilder* writer,i64 value) {
    if (value<0) {
        Status status=std_format_WriteByteToBuilder(writer,45);
        if (status!=Status_Ok) {
            return (status);
        }
        return (std_format_FormatU64ToBuilder(writer,((u64)(0-value)),10));
    }
    return (std_format_FormatU64ToBuilder(writer,((u64)(value)),10));
}
struct TemplateContext std_template_TemplateContextNew(struct TemplateArg* named,usize namedLen,struct String* positional,usize positionalLen) {
    struct TemplateContext ctx={0};
    ctx.named=named;
    ctx.namedLen=namedLen;
    ctx.positional=positional;
    ctx.positionalLen=positionalLen;
    return (ctx);
}
struct TemplateArg std_template_TemplateArgNew(struct String name,struct String value) {
    struct TemplateArg arg={0};
    arg.name=name;
    arg.value=value;
    return (arg);
}
struct Result__String std_template_TemplateFindNamed(struct TemplateContext ctx,struct String name) {
    for (usize i=0;i<ctx.namedLen;i++) {
        if (String_Equals(&ctx.named[i].name,name)) {
            return (std_core_Ok__String(ctx.named[i].value));
        }
    }
    return (std_core_Err__String(Status_NotFound));
}
struct Result__usize std_template_TemplateParseIndex(struct String text) {
    struct Result__usize result={0};
    result.status=Status_Invalid;
    result.value=0;
    if (text.len==0) {
        return (result);
    }
    usize value=0;
    for (usize i=0;i<text.len;i++) {
        byte c=text.data[i];
        if (!std_string_IsAsciiDigit(c)) {
            return (result);
        }
        value=value*10+((usize)(c-'0'));
    }
    result.status=Status_Ok;
    result.value=value;
    return (result);
}
Status std_template_TemplateWritePlaceholder(struct StringBuilder* out,struct String name,struct TemplateContext ctx) {
    struct Result__usize index=std_template_TemplateParseIndex(name);
    if (index.status==Status_Ok) {
        if (index.value>=ctx.positionalLen) {
            return (Status_Invalid);
        }
        return (StringBuilder_WriteString(out,ctx.positional[index.value]));
    }
    struct Result__String named=std_template_TemplateFindNamed(ctx,name);
    if (named.status!=Status_Ok) {
        return (Status_Invalid);
    }
    return (StringBuilder_WriteString(out,named.value));
}
Status std_template_RenderTemplateToBuilder(struct StringBuilder* out,struct String templateText,struct TemplateContext ctx) {
    usize i=0;
    while (i<templateText.len) {
        byte c=templateText.data[i];
        if (c=='{') {
            if (i+1<templateText.len&&templateText.data[i+1]=='{') {
                Status status=StringBuilder_WriteByte(out,'{');
                if (status!=Status_Ok) {
                    return (status);
                }
                i+=2;
                continue;
            }
            usize start=i+1;
            usize end=start;
            while (end<templateText.len&&templateText.data[end]!='}') {
                end+=1;
            }
            if (end>=templateText.len) {
                return (Status_Invalid);
            }
            Status status=std_template_TemplateWritePlaceholder(out,String_Slice(&templateText,start,end-start),ctx);
            if (status!=Status_Ok) {
                return (status);
            }
            i=end+1;
            continue;
        }
        if (c=='}') {
            if (i+1<templateText.len&&templateText.data[i+1]=='}') {
                Status status=StringBuilder_WriteByte(out,'}');
                if (status!=Status_Ok) {
                    return (status);
                }
                i+=2;
                continue;
            }
            return (Status_Invalid);
        }
        Status status=StringBuilder_WriteByte(out,c);
        if (status!=Status_Ok) {
            return (status);
        }
        i+=1;
    }
    return (Status_Ok);
}
struct SourceLocation kek_diagnostics_SourceLocationNew(usize line,usize column,usize offset,usize length) {
    struct SourceLocation location={0};
    location.line=line;
    location.column=column;
    location.offset=offset;
    location.length=length;
    return (location);
}
void kek_diagnostics_DiagnosticBagInit(struct DiagnosticBag* bag,struct Allocator allocator) {
    bag->items=std_array_ArrayNew__Diagnostic(allocator);
    bag->errorCount=0;
}
Status kek_diagnostics_DiagnosticBagDestroy(struct DiagnosticBag* bag) {
    for (usize i=0;i<bag->items.len;i++) {
        std_string_DestroyOwnedString(&bag->items.data[i].message);
    }
    struct Array__Diagnostic items=bag->items;
    Status status=Array__Diagnostic_Destroy(&items);
    bag->items=items;
    bag->errorCount=0;
    return (status);
}
Status kek_diagnostics_DiagnosticBagReserve(struct DiagnosticBag* bag,usize additional) {
    struct Array__Diagnostic items=bag->items;
    Status status=Array__Diagnostic_Reserve(&items,additional);
    bag->items=items;
    return (status);
}
Status kek_diagnostics_DiagnosticAdd(struct DiagnosticBag* bag,DiagnosticSeverity severity,DiagnosticPhase phase,i64 fileIndex,struct SourceLocation location,struct String message) {
    Status status=kek_diagnostics_DiagnosticBagReserve(bag,1);
    if (status!=Status_Ok) {
        return (status);
    }
    struct Diagnostic* diagnostic=&bag->items.data[bag->items.len];
    diagnostic->severity=severity;
    diagnostic->phase=phase;
    diagnostic->fileIndex=fileIndex;
    diagnostic->location=location;
    status=std_string_CloneString(message,bag->items.allocator,&diagnostic->message);
    if (status!=Status_Ok) {
        return (status);
    }
    bag->items.len+=1;
    if (severity==DiagnosticSeverity_Error) {
        bag->errorCount+=1;
    }
    return (Status_Ok);
}
Status kek_diagnostics_DiagnosticAddCString(struct DiagnosticBag* bag,DiagnosticSeverity severity,DiagnosticPhase phase,i64 fileIndex,struct SourceLocation location,str message) {
    return (kek_diagnostics_DiagnosticAdd(bag,severity,phase,fileIndex,location,std_string_StringFromCString(message)));
}
Status kek_diagnostics_DiagnosticAddPathMessage(struct DiagnosticBag* bag,DiagnosticSeverity severity,DiagnosticPhase phase,i64 fileIndex,struct SourceLocation location,str prefix,struct String path) {
    struct StringBuilder builder=std_string_StringBuilderNew(bag->items.allocator);
    Status status=StringBuilder_WriteCString(&builder,prefix);
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(&builder,path);
    }
    if (status!=Status_Ok) {
        StringBuilder_Destroy(&builder);
        return (status);
    }
    struct String message=StringBuilder_View(&builder);
    status=kek_diagnostics_DiagnosticAdd(bag,severity,phase,fileIndex,location,message);
    StringBuilder_Destroy(&builder);
    return (status);
}
Status kek_diagnostics_WriteU64Field(struct StringBuilder* out,u64 value,bool separator) {
    Status status=std_format_FormatU64ToBuilder(out,value,10);
    if (status!=Status_Ok) {
        return (status);
    }
    if (separator) {
        return (StringBuilder_WriteByte(out,'|'));
    }
    return (Status_Ok);
}
Status kek_diagnostics_WriteI64Field(struct StringBuilder* out,i64 value,bool separator) {
    Status status=std_format_FormatI64ToBuilder(out,value);
    if (status!=Status_Ok) {
        return (status);
    }
    if (separator) {
        return (StringBuilder_WriteByte(out,'|'));
    }
    return (Status_Ok);
}
Status kek_diagnostics_WriteDiagnosticDump(struct DiagnosticBag* bag,struct StringBuilder* out) {
    for (usize i=0;i<bag->items.len;i++) {
        struct Diagnostic* diagnostic=&bag->items.data[i];
        Status status=StringBuilder_WriteCString(out,"diag|");
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_WriteU64Field(out,((u64)(diagnostic->severity)),1);
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_WriteU64Field(out,((u64)(diagnostic->phase)),1);
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_WriteI64Field(out,diagnostic->fileIndex,1);
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_WriteU64Field(out,((u64)(diagnostic->location.line)),1);
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_WriteU64Field(out,((u64)(diagnostic->location.column)),1);
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_WriteU64Field(out,((u64)(diagnostic->location.offset)),1);
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_WriteU64Field(out,((u64)(diagnostic->location.length)),1);
        if (status!=Status_Ok) {
            return (status);
        }
        struct String message=std_string_OwnedStringView(&diagnostic->message);
        status=StringBuilder_WriteString(out,message);
        if (status!=Status_Ok) {
            return (status);
        }
        status=StringBuilder_WriteByte(out,'\n');
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
void kek_source_FileTableInit(struct FileTable* table,struct Allocator allocator) {
    table->files=std_array_ArrayNew__SourceFile(allocator);
    table->hasImportRoot=0;
}
Status kek_source_FileTableDestroy(struct FileTable* table) {
    for (usize i=0;i<table->files.len;i++) {
        std_string_DestroyOwnedString(&table->files.data[i].path);
        std_string_DestroyOwnedString(&table->files.data[i].content);
    }
    if (table->hasImportRoot) {
        std_string_DestroyOwnedString(&table->importRoot);
        table->hasImportRoot=0;
    }
    struct Array__SourceFile files=table->files;
    Status status=Array__SourceFile_Destroy(&files);
    table->files=files;
    return (status);
}
bool kek_source_StringEndsWithCString(struct String text,str suffixText) {
    struct String suffix=std_string_StringFromCString(suffixText);
    return (String_EndsWith(&text,suffix));
}
bool kek_source_PathIsAbsolute(struct String path) {
    return (path.len>0&&path.data[0]=='/');
}
usize kek_source_LastPathSeparator(struct String path,usize end) {
    usize i=end;
    while (i>0) {
        byte c=path.data[i-1];
        if (c=='/'||c=='\\') {
            return (i-1);
        }
        i-=1;
    }
    return (path.len);
}
Status kek_source_DirectoryPrefix(struct String path,struct Allocator allocator,struct OwnedString* out) {
    usize separator=kek_source_LastPathSeparator(path,path.len);
    if (separator>=path.len) {
        return (std_string_CloneCString("",allocator,out));
    }
    return (std_string_CloneString(String_Slice(&path,0,separator+1),allocator,out));
}
Status kek_source_JoinPath(struct String root,struct String path,struct Allocator allocator,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(allocator);
    Status status=StringBuilder_WriteString(&builder,root);
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(&builder,path);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(&builder,0);
    }
    if (status!=Status_Ok) {
        StringBuilder_Destroy(&builder);
        return (status);
    }
    struct Result__OwnedString detached=StringBuilder_Detach(&builder);
    if (detached.status!=Status_Ok) {
        StringBuilder_Destroy(&builder);
        return (detached.status);
    }
    out->data=detached.value.data;
    out->len=detached.value.len-1;
    out->cap=detached.value.cap;
    out->allocator=detached.value.allocator;
    return (Status_Ok);
}
Status kek_source_NormalizePath(struct String path,struct Allocator allocator,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(allocator);
    bool previousSlash=0;
    usize i=0;
    while (i<path.len) {
        byte c=path.data[i];
        if (c=='\\') {
            c='/';
        }
        if (c=='/'&&previousSlash) {
            i+=1;
            continue;
        }
        if (c=='.'&&i+1<path.len&&path.data[i+1]=='/') {
            i+=2;
            continue;
        }
        Status status=StringBuilder_WriteByte(&builder,c);
        if (status!=Status_Ok) {
            StringBuilder_Destroy(&builder);
            return (status);
        }
        previousSlash=c=='/';
        i+=1;
    }
    Status termStatus=StringBuilder_WriteByte(&builder,0);
    if (termStatus!=Status_Ok) {
        StringBuilder_Destroy(&builder);
        return (termStatus);
    }
    struct Result__OwnedString detached=StringBuilder_Detach(&builder);
    if (detached.status!=Status_Ok) {
        StringBuilder_Destroy(&builder);
        return (detached.status);
    }
    out->data=detached.value.data;
    out->len=detached.value.len-1;
    out->cap=detached.value.cap;
    out->allocator=detached.value.allocator;
    return (Status_Ok);
}
bool kek_source_FileReadable(struct String path,struct Allocator allocator) {
    struct OwnedString normalized={0};
    Status status=kek_source_NormalizePath(path,allocator,&normalized);
    if (status!=Status_Ok) {
        return (0);
    }
    struct Result__File opened=std_file_FileOpen(((str)(normalized.data)),FileMode_Read);
    if (opened.status!=Status_Ok) {
        std_string_DestroyOwnedString(&normalized);
        return (0);
    }
    struct File file=opened.value;
    File_Close(&file);
    std_string_DestroyOwnedString(&normalized);
    return (1);
}
bool kek_source_FileAlreadyLoaded(struct FileTable* table,struct String path) {
    for (usize i=0;i<table->files.len;i++) {
        struct String filePath=std_string_OwnedStringView(&table->files.data[i].path);
        if (String_Equals(&filePath,path)) {
            return (1);
        }
    }
    return (0);
}
Status kek_source_FileTableReserve(struct FileTable* table,usize additional) {
    struct Array__SourceFile files=table->files;
    Status status=Array__SourceFile_Reserve(&files,additional);
    table->files=files;
    return (status);
}
Status kek_source_ReadFile(struct String path,struct FileTable* table,struct DiagnosticBag* diagnostics) {
    struct OwnedString normalized={0};
    Status status=kek_source_NormalizePath(path,table->files.allocator,&normalized);
    if (status!=Status_Ok) {
        return (status);
    }
    struct String normalizedPath=std_string_OwnedStringView(&normalized);
    if (kek_source_FileAlreadyLoaded(table,normalizedPath)) {
        std_string_DestroyOwnedString(&normalized);
        return (Status_Ok);
    }
    status=kek_source_FileTableReserve(table,1);
    if (status!=Status_Ok) {
        std_string_DestroyOwnedString(&normalized);
        return (status);
    }
    struct Result__File opened=std_file_FileOpen(((str)(normalized.data)),FileMode_Read);
    if (opened.status!=Status_Ok) {
        status=kek_diagnostics_DiagnosticAddPathMessage(diagnostics,DiagnosticSeverity_Error,DiagnosticPhase_Source,-1,kek_diagnostics_SourceLocationNew(0,0,0,0),"could not open file ",normalizedPath);
        std_string_DestroyOwnedString(&normalized);
        return (status);
    }
    struct OwnedString content={0};
    struct File file=opened.value;
    status=std_file_ReadAllToOwnedString(file,table->files.allocator,&content);
    Status closeStatus=File_Close(&file);
    if (status!=Status_Ok) {
        kek_diagnostics_DiagnosticAddPathMessage(diagnostics,DiagnosticSeverity_Error,DiagnosticPhase_Source,-1,kek_diagnostics_SourceLocationNew(0,0,0,0),"could not read file ",normalizedPath);
        std_string_DestroyOwnedString(&normalized);
        return (status);
    }
    if (closeStatus!=Status_Ok) {
        std_string_DestroyOwnedString(&content);
        std_string_DestroyOwnedString(&normalized);
        return (closeStatus);
    }
    struct SourceFile* source=&table->files.data[table->files.len];
    source->path=normalized;
    source->content=content;
    source->fileIndex=table->files.len;
    table->files.len+=1;
    return (Status_Ok);
}
Status kek_source_SetImportRoot(struct FileTable* table,struct String entryPath) {
    struct OwnedString normalized={0};
    Status status=kek_source_NormalizePath(entryPath,table->files.allocator,&normalized);
    if (status!=Status_Ok) {
        return (status);
    }
    struct String normalizedPath=std_string_OwnedStringView(&normalized);
    struct OwnedString root={0};
    status=kek_source_DirectoryPrefix(normalizedPath,table->files.allocator,&root);
    std_string_DestroyOwnedString(&normalized);
    if (status!=Status_Ok) {
        return (status);
    }
    table->importRoot=root;
    table->hasImportRoot=1;
    return (Status_Ok);
}
Status kek_source_ReadResolvedImportFile(struct FileTable* table,struct String path,struct DiagnosticBag* diagnostics) {
    if (kek_source_PathIsAbsolute(path)||kek_source_FileReadable(path,table->files.allocator)||!table->hasImportRoot) {
        return (kek_source_ReadFile(path,table,diagnostics));
    }
    struct String root=std_string_OwnedStringView(&table->importRoot);
    while (root.len>0) {
        struct OwnedString candidate={0};
        Status joinStatus=kek_source_JoinPath(root,path,table->files.allocator,&candidate);
        if (joinStatus!=Status_Ok) {
            return (joinStatus);
        }
        struct String candidatePath=std_string_OwnedStringView(&candidate);
        if (kek_source_FileReadable(candidatePath,table->files.allocator)) {
            Status status=kek_source_ReadFile(candidatePath,table,diagnostics);
            std_string_DestroyOwnedString(&candidate);
            return (status);
        }
        std_string_DestroyOwnedString(&candidate);
        if (root.len<=1) {
            break;
        }
        usize separator=kek_source_LastPathSeparator(root,root.len-1);
        if (separator>=root.len) {
            break;
        }
        root=String_Slice(&root,0,separator+1);
    }
    return (kek_source_ReadFile(path,table,diagnostics));
}
bool kek_source_ImportDirectiveAt(struct String source,usize cursor) {
    usize lineStart=cursor;
    while (lineStart>0&&source.data[lineStart-1]!='\n'&&source.data[lineStart-1]!='\r') {
        lineStart-=1;
    }
    for (usize i=lineStart;i<cursor;i++) {
        byte c=source.data[i];
        if (c!=' '&&c!='\t') {
            return (0);
        }
    }
    return (1);
}
Status kek_source_LoadImportFile(struct FileTable* table,struct String path,struct DiagnosticBag* diagnostics,i64 fileIndex,struct SourceLocation location) {
    if (!kek_source_StringEndsWithCString(path,".kek")) {
        return (kek_diagnostics_DiagnosticAddPathMessage(diagnostics,DiagnosticSeverity_Error,DiagnosticPhase_Source,fileIndex,location,"import path must name a .kek file ",path));
    }
    usize before=table->files.len;
    Status status=kek_source_ReadResolvedImportFile(table,path,diagnostics);
    if (status!=Status_Ok) {
        return (status);
    }
    if (table->files.len>before) {
        return (kek_source_LoadImports(table,&table->files.data[before],diagnostics));
    }
    return (Status_Ok);
}
Status kek_source_LoadImports(struct FileTable* table,struct SourceFile* file,struct DiagnosticBag* diagnostics) {
    struct String source=std_string_OwnedStringView(&file->content);
    usize currentFileIndex=file->fileIndex;
    struct String prefix=std_string_StringFromCString("#import");
    usize cursor=0;
    while (cursor+prefix.len+1<=source.len) {
        bool matched=1;
        for (usize i=0;i<prefix.len;i++) {
            if (source.data[cursor+i]!=prefix.data[i]) {
                matched=0;
                break;
            }
        }
        if (!matched) {
            cursor+=1;
            continue;
        }
        if (!kek_source_ImportDirectiveAt(source,cursor)) {
            cursor+=1;
            continue;
        }
        if (source.data[cursor+prefix.len]!='(') {
            cursor+=1;
            continue;
        }
        usize start=cursor+prefix.len+1;
        usize end=start;
        while (end<source.len&&source.data[end]!=')') {
            end+=1;
        }
        struct SourceLocation location=kek_diagnostics_SourceLocationNew(1,1,cursor,prefix.len);
        if (end>=source.len) {
            return (kek_diagnostics_DiagnosticAddCString(diagnostics,DiagnosticSeverity_Error,DiagnosticPhase_Source,((i64)(currentFileIndex)),location,"unterminated import"));
        }
        usize length=end-start;
        if (length==0) {
            return (kek_diagnostics_DiagnosticAddCString(diagnostics,DiagnosticSeverity_Error,DiagnosticPhase_Source,((i64)(currentFileIndex)),location,"invalid import path"));
        }
        struct String importPath=String_Slice(&source,start,length);
        Status status=kek_source_LoadImportFile(table,importPath,diagnostics,((i64)(currentFileIndex)),location);
        if (status!=Status_Ok) {
            return (status);
        }
        cursor=end+1;
    }
    return (Status_Ok);
}
Status kek_source_LoadCompilationSources(str entryPath,struct Allocator allocator,struct FileTable* table,struct DiagnosticBag* diagnostics) {
    kek_source_FileTableInit(table,allocator);
    Status rootStatus=kek_source_SetImportRoot(table,std_string_StringFromCString(entryPath));
    if (rootStatus!=Status_Ok) {
        return (rootStatus);
    }
    Status status=kek_source_ReadFile(std_string_StringFromCString(entryPath),table,diagnostics);
    if (status!=Status_Ok) {
        return (status);
    }
    return (kek_source_LoadImports(table,&table->files.data[0],diagnostics));
}
struct Token kek_tokenizer_TokenNew(TokenKind kind,u64 subkind,usize line,usize column,usize offset,usize length) {
    struct Token token={0};
    token.kind=kind;
    token.subkind=subkind;
    token.payload=TokenPayload_None(subkind);
    if (kind==TokenKind_Operator) {
        token.payload=TokenPayload_Operator(((OperatorKind)(subkind)));
    }
    if (kind==TokenKind_Keyword) {
        token.payload=TokenPayload_Keyword(((KeywordKind)(subkind)));
    }
    if (kind==TokenKind_Punctuation) {
        token.payload=TokenPayload_Punctuation(((PunctuationKind)(subkind)));
    }
    token.line=line;
    token.column=column;
    token.offset=offset;
    token.length=length;
    return (token);
}
struct Tokenizer kek_tokenizer_TokenizerNew(struct String source,bool emitComments) {
    struct Tokenizer tokenizer={0};
    tokenizer.cursor=std_scan_ByteCursorNew(source);
    tokenizer.emitComments=emitComments;
    return (tokenizer);
}
bool Tokenizer_AtEnd(struct Tokenizer* this) {
    return (ByteCursor_AtEnd(&this->cursor));
}
byte Tokenizer_Peek(struct Tokenizer* this) {
    return (ByteCursor_Peek(&this->cursor));
}
byte Tokenizer_PeekAt(struct Tokenizer* this,usize offset) {
    return (ByteCursor_PeekAt(&this->cursor,offset));
}
byte Tokenizer_Advance(struct Tokenizer* this) {
    return (ByteCursor_Advance(&this->cursor));
}
bool kek_tokenizer_TextAtEquals(str text,struct String source,usize start,usize length) {
    if (start+length>source.len) {
        return (0);
    }
    struct String slice=String_Slice(&source,start,length);
    return (String_EqualsCString(&slice,text));
}
bool Tokenizer_StartsWith(struct Tokenizer* this,str text) {
    struct String rest=String_Slice(&this->cursor.input,this->cursor.pos,this->cursor.input.len-this->cursor.pos);
    return (String_StartsWith(&rest,std_string_StringFromCString(text)));
}
void Tokenizer_AdvanceMany(struct Tokenizer* this,usize count) {
    for (usize i=0;i<count;i++) {
        ByteCursor_Advance(&this->cursor);
    }
}
void Tokenizer_SkipWhitespace(struct Tokenizer* this) {
    ByteCursor_SkipAsciiWhitespace(&this->cursor);
}
void Tokenizer_SkipWhitespaceAndComments(struct Tokenizer* this) {
    while (1) {
        Tokenizer_SkipWhitespace(this);
        if (Tokenizer_Peek(this)=='/'&&Tokenizer_PeekAt(this,1)=='/') {
            while (!Tokenizer_AtEnd(this)&&Tokenizer_Peek(this)!='\n') {
                Tokenizer_Advance(this);
            }
            continue;
        }
        if (Tokenizer_Peek(this)=='/'&&Tokenizer_PeekAt(this,1)=='*') {
            Tokenizer_AdvanceMany(this,2);
            while (!Tokenizer_AtEnd(this)) {
                if (Tokenizer_Peek(this)=='*'&&Tokenizer_PeekAt(this,1)=='/') {
                    Tokenizer_AdvanceMany(this,2);
                    break;
                }
                Tokenizer_Advance(this);
            }
            continue;
        }
        break;
    }
}
struct Token Tokenizer_ReadLineComment(struct Tokenizer* this) {
    usize start=this->cursor.pos;
    usize line=this->cursor.line;
    usize column=this->cursor.column;
    TokenKind kind=TokenKind_Comment;
    if (Tokenizer_PeekAt(this,2)=='/') {
        kind=TokenKind_DocComment;
    }
    while (!Tokenizer_AtEnd(this)&&Tokenizer_Peek(this)!='\n') {
        Tokenizer_Advance(this);
    }
    return (kek_tokenizer_TokenNew(kind,0,line,column,start,this->cursor.pos-start));
}
struct Token Tokenizer_ReadBlockComment(struct Tokenizer* this) {
    usize start=this->cursor.pos;
    usize line=this->cursor.line;
    usize column=this->cursor.column;
    Tokenizer_AdvanceMany(this,2);
    while (!Tokenizer_AtEnd(this)) {
        if (Tokenizer_Peek(this)=='*'&&Tokenizer_PeekAt(this,1)=='/') {
            Tokenizer_AdvanceMany(this,2);
            break;
        }
        Tokenizer_Advance(this);
    }
    return (kek_tokenizer_TokenNew(TokenKind_Comment,0,line,column,start,this->cursor.pos-start));
}
u64 kek_tokenizer_KeywordSubkind(struct String source,usize start,usize length) {
    if (kek_tokenizer_TextAtEquals("if",source,start,length)) {
        return (((u64)(KeywordKind_If)));
    }
    if (kek_tokenizer_TextAtEquals("else",source,start,length)) {
        return (((u64)(KeywordKind_Else)));
    }
    if (kek_tokenizer_TextAtEquals("while",source,start,length)) {
        return (((u64)(KeywordKind_While)));
    }
    if (kek_tokenizer_TextAtEquals("for",source,start,length)) {
        return (((u64)(KeywordKind_For)));
    }
    if (kek_tokenizer_TextAtEquals("return",source,start,length)) {
        return (((u64)(KeywordKind_Return)));
    }
    if (kek_tokenizer_TextAtEquals("do",source,start,length)) {
        return (((u64)(KeywordKind_Do)));
    }
    if (kek_tokenizer_TextAtEquals("break",source,start,length)) {
        return (((u64)(KeywordKind_Break)));
    }
    if (kek_tokenizer_TextAtEquals("continue",source,start,length)) {
        return (((u64)(KeywordKind_Continue)));
    }
    if (kek_tokenizer_TextAtEquals("using",source,start,length)) {
        return (((u64)(KeywordKind_Using)));
    }
    if (kek_tokenizer_TextAtEquals("alias",source,start,length)) {
        return (((u64)(KeywordKind_Alias)));
    }
    if (kek_tokenizer_TextAtEquals("export",source,start,length)) {
        return (((u64)(KeywordKind_Export)));
    }
    if (kek_tokenizer_TextAtEquals("extern",source,start,length)) {
        return (((u64)(KeywordKind_Extern)));
    }
    if (kek_tokenizer_TextAtEquals("enum",source,start,length)) {
        return (((u64)(KeywordKind_Enum)));
    }
    if (kek_tokenizer_TextAtEquals("struct",source,start,length)) {
        return (((u64)(KeywordKind_Struct)));
    }
    if (kek_tokenizer_TextAtEquals("union",source,start,length)) {
        return (((u64)(KeywordKind_Union)));
    }
    if (kek_tokenizer_TextAtEquals("switch",source,start,length)) {
        return (((u64)(KeywordKind_Switch)));
    }
    if (kek_tokenizer_TextAtEquals("select",source,start,length)) {
        return (((u64)(KeywordKind_Select)));
    }
    if (kek_tokenizer_TextAtEquals("case",source,start,length)) {
        return (((u64)(KeywordKind_Case)));
    }
    if (kek_tokenizer_TextAtEquals("default",source,start,length)) {
        return (((u64)(KeywordKind_Default)));
    }
    if (kek_tokenizer_TextAtEquals("each",source,start,length)) {
        return (((u64)(KeywordKind_Each)));
    }
    if (kek_tokenizer_TextAtEquals("packed",source,start,length)) {
        return (((u64)(KeywordKind_Packed)));
    }
    if (kek_tokenizer_TextAtEquals("aligned",source,start,length)) {
        return (((u64)(KeywordKind_Aligned)));
    }
    if (kek_tokenizer_TextAtEquals("comptime",source,start,length)) {
        return (((u64)(KeywordKind_Comptime)));
    }
    if (kek_tokenizer_TextAtEquals("defer",source,start,length)) {
        return (((u64)(KeywordKind_Defer)));
    }
    if (kek_tokenizer_TextAtEquals("tagged",source,start,length)) {
        return (((u64)(KeywordKind_Tagged)));
    }
    if (kek_tokenizer_TextAtEquals("true",source,start,length)) {
        return (((u64)(KeywordKind_True)));
    }
    if (kek_tokenizer_TextAtEquals("false",source,start,length)) {
        return (((u64)(KeywordKind_False)));
    }
    if (kek_tokenizer_TextAtEquals("unreachable",source,start,length)) {
        return (((u64)(KeywordKind_Unreachable)));
    }
    if (kek_tokenizer_TextAtEquals("panic",source,start,length)) {
        return (((u64)(KeywordKind_Panic)));
    }
    return (1000);
}
struct Token Tokenizer_ReadIdentifierOrKeyword(struct Tokenizer* this) {
    usize start=this->cursor.pos;
    usize line=this->cursor.line;
    usize column=this->cursor.column;
    while (std_string_IsAsciiWord(Tokenizer_Peek(this))) {
        Tokenizer_Advance(this);
    }
    usize length=this->cursor.pos-start;
    u64 keyword=kek_tokenizer_KeywordSubkind(this->cursor.input,start,length);
    if (keyword!=1000) {
        return (kek_tokenizer_TokenNew(TokenKind_Keyword,keyword,line,column,start,length));
    }
    return (kek_tokenizer_TokenNew(TokenKind_Identifier,0,line,column,start,length));
}
struct Token Tokenizer_ReadNumber(struct Tokenizer* this) {
    usize start=this->cursor.pos;
    usize line=this->cursor.line;
    usize column=this->cursor.column;
    while (std_string_IsAsciiWord(Tokenizer_Peek(this))||Tokenizer_Peek(this)=='.'||Tokenizer_Peek(this)=='_') {
        Tokenizer_Advance(this);
    }
    return (kek_tokenizer_TokenNew(TokenKind_Number,0,line,column,start,this->cursor.pos-start));
}
struct Token Tokenizer_ReadDelimited(struct Tokenizer* this,TokenKind kind,byte delimiter) {
    usize start=this->cursor.pos;
    usize line=this->cursor.line;
    usize column=this->cursor.column;
    Tokenizer_Advance(this);
    while (!Tokenizer_AtEnd(this)) {
        if (Tokenizer_Peek(this)=='\\') {
            Tokenizer_AdvanceMany(this,2);
            continue;
        }
        if (Tokenizer_Peek(this)==delimiter) {
            Tokenizer_Advance(this);
            break;
        }
        Tokenizer_Advance(this);
    }
    return (kek_tokenizer_TokenNew(kind,0,line,column,start,this->cursor.pos-start));
}
struct Token Tokenizer_ReadTripleQuotedString(struct Tokenizer* this) {
    usize start=this->cursor.pos;
    usize line=this->cursor.line;
    usize column=this->cursor.column;
    Tokenizer_AdvanceMany(this,3);
    while (!Tokenizer_AtEnd(this)) {
        if (Tokenizer_StartsWith(this,"\"\"\"")) {
            Tokenizer_AdvanceMany(this,3);
            break;
        }
        Tokenizer_Advance(this);
    }
    return (kek_tokenizer_TokenNew(TokenKind_String,0,line,column,start,this->cursor.pos-start));
}
bool Tokenizer_ReadOperator(struct Tokenizer* this,struct Token* out,usize start,usize line,usize column) {
    if (Tokenizer_StartsWith(this,"::")) {
        Tokenizer_AdvanceMany(this,2);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_Scope)),line,column,start,2);
        return (1);
    }
    if (Tokenizer_StartsWith(this,"==")) {
        Tokenizer_AdvanceMany(this,2);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_Equal)),line,column,start,2);
        return (1);
    }
    if (Tokenizer_StartsWith(this,"!=")) {
        Tokenizer_AdvanceMany(this,2);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_NotEqual)),line,column,start,2);
        return (1);
    }
    if (Tokenizer_StartsWith(this,"<=")) {
        Tokenizer_AdvanceMany(this,2);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_LessEqual)),line,column,start,2);
        return (1);
    }
    if (Tokenizer_StartsWith(this,">=")) {
        Tokenizer_AdvanceMany(this,2);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_GreaterEqual)),line,column,start,2);
        return (1);
    }
    if (Tokenizer_StartsWith(this,"&&")) {
        Tokenizer_AdvanceMany(this,2);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_LogicalAnd)),line,column,start,2);
        return (1);
    }
    if (Tokenizer_StartsWith(this,"||")) {
        Tokenizer_AdvanceMany(this,2);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_LogicalOr)),line,column,start,2);
        return (1);
    }
    if (Tokenizer_StartsWith(this,"+=")) {
        Tokenizer_AdvanceMany(this,2);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_PlusAssign)),line,column,start,2);
        return (1);
    }
    if (Tokenizer_StartsWith(this,"-=")) {
        Tokenizer_AdvanceMany(this,2);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_MinusAssign)),line,column,start,2);
        return (1);
    }
    if (Tokenizer_StartsWith(this,"->")) {
        Tokenizer_AdvanceMany(this,2);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_Arrow)),line,column,start,2);
        return (1);
    }
    byte c=Tokenizer_Peek(this);
    if (c=='+') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_Plus)),line,column,start,1);
        return (1);
    }
    if (c=='-') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_Minus)),line,column,start,1);
        return (1);
    }
    if (c=='*') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_Multiply)),line,column,start,1);
        return (1);
    }
    if (c=='/') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_Divide)),line,column,start,1);
        return (1);
    }
    if (c=='%') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_Modulo)),line,column,start,1);
        return (1);
    }
    if (c=='=') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_Assign)),line,column,start,1);
        return (1);
    }
    if (c=='<') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_Less)),line,column,start,1);
        return (1);
    }
    if (c=='>') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_Greater)),line,column,start,1);
        return (1);
    }
    if (c=='!') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_LogicalNot)),line,column,start,1);
        return (1);
    }
    if (c=='&') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_BitwiseAnd)),line,column,start,1);
        return (1);
    }
    if (c=='|') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_BitwiseOr)),line,column,start,1);
        return (1);
    }
    if (c=='~') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Operator,((u64)(OperatorKind_BitwiseNot)),line,column,start,1);
        return (1);
    }
    return (0);
}
bool Tokenizer_ReadPunctuation(struct Tokenizer* this,struct Token* out,usize start,usize line,usize column) {
    byte c=Tokenizer_Peek(this);
    if (c=='(') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Punctuation,((u64)(PunctuationKind_LeftParen)),line,column,start,1);
        return (1);
    }
    if (c==')') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Punctuation,((u64)(PunctuationKind_RightParen)),line,column,start,1);
        return (1);
    }
    if (c=='{') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Punctuation,((u64)(PunctuationKind_LeftBrace)),line,column,start,1);
        return (1);
    }
    if (c=='}') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Punctuation,((u64)(PunctuationKind_RightBrace)),line,column,start,1);
        return (1);
    }
    if (c=='[') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Punctuation,((u64)(PunctuationKind_LeftBracket)),line,column,start,1);
        return (1);
    }
    if (c==']') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Punctuation,((u64)(PunctuationKind_RightBracket)),line,column,start,1);
        return (1);
    }
    if (c==';') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Punctuation,((u64)(PunctuationKind_Semicolon)),line,column,start,1);
        return (1);
    }
    if (c==',') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Punctuation,((u64)(PunctuationKind_Comma)),line,column,start,1);
        return (1);
    }
    if (c==':') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Punctuation,((u64)(PunctuationKind_Colon)),line,column,start,1);
        return (1);
    }
    if (c=='.') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Punctuation,((u64)(PunctuationKind_Dot)),line,column,start,1);
        return (1);
    }
    if (c=='#') {
        Tokenizer_Advance(this);
        out[0]=kek_tokenizer_TokenNew(TokenKind_Punctuation,((u64)(PunctuationKind_Hash)),line,column,start,1);
        return (1);
    }
    return (0);
}
struct Token Tokenizer_Next(struct Tokenizer* this) {
    if (this->emitComments) {
        Tokenizer_SkipWhitespace(this);
        if (Tokenizer_Peek(this)=='/'&&Tokenizer_PeekAt(this,1)=='/') {
            return (Tokenizer_ReadLineComment(this));
        }
        if (Tokenizer_Peek(this)=='/'&&Tokenizer_PeekAt(this,1)=='*') {
            return (Tokenizer_ReadBlockComment(this));
        }
    } else {
        Tokenizer_SkipWhitespaceAndComments(this);
    }
    if (Tokenizer_AtEnd(this)) {
        return (kek_tokenizer_TokenNew(TokenKind_Eof,0,this->cursor.line,this->cursor.column,this->cursor.pos,0));
    }
    usize start=this->cursor.pos;
    usize line=this->cursor.line;
    usize column=this->cursor.column;
    byte c=Tokenizer_Peek(this);
    if (std_string_IsAsciiAlpha(c)) {
        return (Tokenizer_ReadIdentifierOrKeyword(this));
    }
    if (std_string_IsAsciiDigit(c)) {
        return (Tokenizer_ReadNumber(this));
    }
    if (Tokenizer_StartsWith(this,"\"\"\"")) {
        return (Tokenizer_ReadTripleQuotedString(this));
    }
    if (c=='"') {
        return (Tokenizer_ReadDelimited(this,TokenKind_String,'"'));
    }
    if (c=='\'') {
        return (Tokenizer_ReadDelimited(this,TokenKind_Char,'\''));
    }
    struct Token token={0};
    if (Tokenizer_ReadOperator(this,&token,start,line,column)) {
        return (token);
    }
    if (Tokenizer_ReadPunctuation(this,&token,start,line,column)) {
        return (token);
    }
    Tokenizer_Advance(this);
    return (kek_tokenizer_TokenNew(TokenKind_Identifier,0,line,column,start,1));
}
Status kek_tokenizer_TokenizeToArray(struct String source,bool emitComments,struct Allocator allocator,struct Array__Token* out) {
    out[0]=std_array_ArrayNew__Token(allocator);
    struct Tokenizer tokenizer=kek_tokenizer_TokenizerNew(source,emitComments);
    while (1) {
        struct Token token=Tokenizer_Next(&tokenizer);
        Status status=Array__Token_Push(out,token);
        if (status!=Status_Ok) {
            return (status);
        }
        if (token.kind==TokenKind_Eof) {
            break;
        }
    }
    return (Status_Ok);
}
struct SourceLocation kek_ast_TokenLocation(struct Token token) {
    return (kek_diagnostics_SourceLocationNew(token.line,token.column,token.offset,token.length));
}
Status kek_ast_AstTreeReserve(struct AstTree* tree,usize additional) {
    struct Array__AstNode nodes=tree->nodes;
    Status status=Array__AstNode_Reserve(&nodes,additional);
    tree->nodes=nodes;
    return (status);
}
Status kek_ast_AstTreeInit(struct AstTree* tree,struct Allocator allocator) {
    tree->nodes=std_array_ArrayNew__AstNode(allocator);
    tree->root=0;
    Status status=kek_ast_AstTreeReserve(tree,1);
    if (status!=Status_Ok) {
        return (status);
    }
    struct AstNode sentinel={0};
    tree->nodes.data[0]=sentinel;
    tree->nodes.len=1;
    return (Status_Ok);
}
Status kek_ast_AstTreeDestroy(struct AstTree* tree) {
    struct Array__AstNode nodes=tree->nodes;
    Status status=Array__AstNode_Destroy(&nodes);
    tree->nodes=nodes;
    tree->root=0;
    return (status);
}
bool kek_ast_IsPunctuationToken(struct Token* token,PunctuationKind punctuation) {
    return (token->payload.tag==TokenPayloadTag_Punctuation&&token->payload.data.Punctuation==punctuation);
}
bool kek_ast_IsOperatorToken(struct Token* token,OperatorKind operatorKind) {
    return (token->payload.tag==TokenPayloadTag_Operator&&token->payload.data.Operator==operatorKind);
}
bool kek_ast_IsTriviaToken(struct Token* token) {
    return (token->kind==TokenKind_Comment||token->kind==TokenKind_DocComment);
}
bool kek_ast_IsClosingPunctuation(struct Token* token) {
    return (kek_ast_IsPunctuationToken(token,PunctuationKind_RightParen)||kek_ast_IsPunctuationToken(token,PunctuationKind_RightBrace)||kek_ast_IsPunctuationToken(token,PunctuationKind_RightBracket));
}
bool kek_ast_IsAstTerminator(struct Token* token,u64 closePunctuation) {
    if (token->kind==TokenKind_Eof) {
        return (1);
    }
    if (closePunctuation<11&&token->kind==TokenKind_Punctuation&&token->subkind==closePunctuation) {
        return (1);
    }
    if (kek_ast_IsClosingPunctuation(token)) {
        return (1);
    }
    return (0);
}
bool kek_ast_IsGenericTerminator(struct Token* token) {
    return (token->kind==TokenKind_Eof||kek_ast_IsOperatorToken(token,OperatorKind_Greater));
}
bool kek_ast_TokenTextEquals(struct Parser* parser,struct Token* token,str text) {
    usize length=0;
    while (text[length]!=0) {
        length+=1;
    }
    if (!(token->kind==TokenKind_Identifier||token->kind==TokenKind_Number||token->kind==TokenKind_String)) {
        return (0);
    }
    if (token->length!=length) {
        return (0);
    }
    if (token->offset+length>parser->source.len) {
        return (0);
    }
    for (usize i=0;i<length;i++) {
        if (parser->source.data[token->offset+i]!=text[i]) {
            return (0);
        }
    }
    return (1);
}
usize kek_ast_CreateAstNode(struct Parser* parser,AstKind kind,struct SourceLocation location) {
    Status status=kek_ast_AstTreeReserve(&parser->tree,1);
    if (status!=Status_Ok) {
        kek_diagnostics_DiagnosticAddCString(parser->diagnostics,DiagnosticSeverity_Error,DiagnosticPhase_Parse,parser->fileIndex,location,"AST node storage capacity exceeded");
        parser->errorCount+=1;
        if (parser->tree.nodes.len>0) {
            return (parser->tree.nodes.len-1);
        }
        return (0);
    }
    usize index=parser->tree.nodes.len;
    struct AstNode node={0};
    node.kind=kind;
    node.location=location;
    parser->tree.nodes.data[index]=node;
    parser->tree.nodes.len+=1;
    return (index);
}
void kek_ast_AddAstChild(struct AstTree* tree,usize parentIndex,usize childIndex) {
    if (parentIndex==0||childIndex==0) {
        return;
    }
    tree->nodes.data[childIndex].nextSibling=0;
    if (tree->nodes.data[parentIndex].lastChild!=0) {
        usize last=tree->nodes.data[parentIndex].lastChild;
        tree->nodes.data[last].nextSibling=childIndex;
    } else {
        tree->nodes.data[parentIndex].firstChild=childIndex;
    }
    tree->nodes.data[parentIndex].lastChild=childIndex;
    tree->nodes.data[parentIndex].childCount+=1;
}
void kek_ast_FinishLocationFromChildren(struct AstTree* tree,usize nodeIndex) {
    if (nodeIndex==0||tree->nodes.data[nodeIndex].childCount==0) {
        return;
    }
    usize firstIndex=tree->nodes.data[nodeIndex].firstChild;
    usize lastIndex=tree->nodes.data[nodeIndex].lastChild;
    struct SourceLocation first=tree->nodes.data[firstIndex].location;
    struct SourceLocation last=tree->nodes.data[lastIndex].location;
    tree->nodes.data[nodeIndex].location=first;
    if (last.offset+last.length>=first.offset) {
        tree->nodes.data[nodeIndex].location.length=(last.offset+last.length)-first.offset;
    }
}
usize kek_ast_ParseTokenNode(struct Parser* parser) {
    struct Token token=parser->tokens[parser->position];
    parser->position+=1;
    usize nodeIndex=kek_ast_CreateAstNode(parser,AstKind_Token,kek_ast_TokenLocation(token));
    parser->tree.nodes.data[nodeIndex].token=token;
    return (nodeIndex);
}
void kek_ast_ReportParseError(struct Parser* parser,struct Token* token,str message) {
    kek_diagnostics_DiagnosticAddCString(parser->diagnostics,DiagnosticSeverity_Error,DiagnosticPhase_Parse,parser->fileIndex,kek_ast_TokenLocation(token[0]),message);
    parser->errorCount+=1;
}
str kek_ast_PunctuationName(u64 punctuation) {
    if (punctuation==((u64)(PunctuationKind_RightParen))) {
        return (")");
    }
    if (punctuation==((u64)(PunctuationKind_RightBrace))) {
        return ("}");
    }
    if (punctuation==((u64)(PunctuationKind_RightBracket))) {
        return ("]");
    }
    return ("<end of file>");
}
void kek_ast_ReportExpected(struct Parser* parser,struct Token* token,u64 punctuation) {
    struct StringBuilder builder=std_string_StringBuilderNew(parser->tree.nodes.allocator);
    StringBuilder_WriteCString(&builder,"expected '");
    StringBuilder_WriteCString(&builder,kek_ast_PunctuationName(punctuation));
    StringBuilder_WriteByte(&builder,'\'');
    struct String message=StringBuilder_View(&builder);
    kek_diagnostics_DiagnosticAdd(parser->diagnostics,DiagnosticSeverity_Error,DiagnosticPhase_Parse,parser->fileIndex,kek_ast_TokenLocation(token[0]),message);
    StringBuilder_Destroy(&builder);
    parser->errorCount+=1;
}
bool kek_ast_ShouldParseGenericList(struct Parser* parser,usize previousChildIndex) {
    if (previousChildIndex==0||parser->tree.nodes.data[previousChildIndex].kind!=AstKind_Token) {
        return (0);
    }
    struct Token* previousToken=&parser->tree.nodes.data[previousChildIndex].token;
    if (previousToken->kind!=TokenKind_Identifier&&previousToken->kind!=TokenKind_Keyword) {
        return (0);
    }
    if (kek_ast_TokenTextEquals(parser,previousToken,"cast")) {
        return (0);
    }
    if (parser->position>=parser->count||!kek_ast_IsOperatorToken(&parser->tokens[parser->position],OperatorKind_Less)) {
        return (0);
    }
    int depth=0;
    for (usize i=parser->position;i<parser->count;i++) {
        struct Token* token=&parser->tokens[i];
        if (kek_ast_IsOperatorToken(token,OperatorKind_Less)) {
            depth+=1;
            continue;
        }
        if (kek_ast_IsOperatorToken(token,OperatorKind_Greater)) {
            depth-=1;
            if (depth==0) {
                return (1);
            }
            continue;
        }
        if (depth==1&&token->kind==TokenKind_Punctuation&&token->subkind==((u64)(PunctuationKind_Dot))) {
            return (0);
        }
        if (token->kind==TokenKind_Eof||kek_ast_IsPunctuationToken(token,PunctuationKind_Semicolon)||kek_ast_IsPunctuationToken(token,PunctuationKind_RightParen)||kek_ast_IsPunctuationToken(token,PunctuationKind_LeftBrace)||kek_ast_IsPunctuationToken(token,PunctuationKind_RightBrace)) {
            return (0);
        }
    }
    return (0);
}
void kek_ast_ParseChildrenInto(struct Parser* parser,usize parentIndex,u64 closePunctuation) {
    while (parser->position<parser->count&&!kek_ast_IsAstTerminator(&parser->tokens[parser->position],closePunctuation)) {
        if (kek_ast_IsTriviaToken(&parser->tokens[parser->position])) {
            parser->position+=1;
            continue;
        }
        usize previousPosition=parser->position;
        usize statement=kek_ast_ParseStatement(parser,closePunctuation);
        if (parser->tree.nodes.data[statement].childCount>0) {
            kek_ast_AddAstChild(&parser->tree,parentIndex,statement);
        } else {
            if (parser->position==previousPosition) {
                break;
            }
        }
    }
}
usize kek_ast_ParseDelimited(struct Parser* parser,AstKind kind,u64 closePunctuation) {
    struct Token open=parser->tokens[parser->position];
    parser->position+=1;
    usize node=kek_ast_CreateAstNode(parser,kind,kek_ast_TokenLocation(open));
    kek_ast_ParseChildrenInto(parser,node,closePunctuation);
    if (parser->position<parser->count&&parser->tokens[parser->position].kind==TokenKind_Punctuation&&parser->tokens[parser->position].subkind==closePunctuation) {
        struct Token close=parser->tokens[parser->position];
        parser->position+=1;
        parser->tree.nodes.data[node].location=kek_ast_TokenLocation(open);
        parser->tree.nodes.data[node].location.length=(close.offset+close.length)-open.offset;
    } else {
        if (parser->position<parser->count) {
            kek_ast_ReportExpected(parser,&parser->tokens[parser->position],closePunctuation);
            if (kek_ast_IsClosingPunctuation(&parser->tokens[parser->position])) {
                parser->position+=1;
            }
        } else {
            kek_ast_ReportParseError(parser,&open,"unterminated delimiter");
        }
    }
    if (parser->tree.nodes.data[node].childCount>0&&parser->tree.nodes.data[node].location.length==open.length) {
        kek_ast_FinishLocationFromChildren(&parser->tree,node);
        parser->tree.nodes.data[node].location.offset=open.offset;
        parser->tree.nodes.data[node].location.line=open.line;
        parser->tree.nodes.data[node].location.column=open.column;
    }
    return (node);
}
usize kek_ast_ParseGenericDelimited(struct Parser* parser) {
    struct Token open=parser->tokens[parser->position];
    parser->position+=1;
    usize node=kek_ast_CreateAstNode(parser,AstKind_Generic,kek_ast_TokenLocation(open));
    while (parser->position<parser->count&&!kek_ast_IsGenericTerminator(&parser->tokens[parser->position])) {
        if (kek_ast_IsTriviaToken(&parser->tokens[parser->position])) {
            parser->position+=1;
            continue;
        }
        usize statement=kek_ast_CreateAstNode(parser,AstKind_Statement,kek_ast_TokenLocation(parser->tokens[parser->position]));
        while (parser->position<parser->count&&!kek_ast_IsGenericTerminator(&parser->tokens[parser->position])) {
            struct Token* token=&parser->tokens[parser->position];
            if (kek_ast_IsTriviaToken(token)) {
                parser->position+=1;
                continue;
            }
            if (kek_ast_IsPunctuationToken(token,PunctuationKind_Comma)) {
                parser->position+=1;
                break;
            }
            if (kek_ast_IsPunctuationToken(token,PunctuationKind_LeftParen)) {
                kek_ast_AddAstChild(&parser->tree,statement,kek_ast_ParseDelimited(parser,AstKind_Group,((u64)(PunctuationKind_RightParen))));
                continue;
            }
            if (kek_ast_IsPunctuationToken(token,PunctuationKind_LeftBracket)) {
                kek_ast_AddAstChild(&parser->tree,statement,kek_ast_ParseDelimited(parser,AstKind_Index,((u64)(PunctuationKind_RightBracket))));
                continue;
            }
            if (kek_ast_ShouldParseGenericList(parser,parser->tree.nodes.data[statement].lastChild)) {
                kek_ast_AddAstChild(&parser->tree,statement,kek_ast_ParseGenericDelimited(parser));
                continue;
            }
            kek_ast_AddAstChild(&parser->tree,statement,kek_ast_ParseTokenNode(parser));
        }
        kek_ast_FinishLocationFromChildren(&parser->tree,statement);
        if (parser->tree.nodes.data[statement].childCount>0) {
            kek_ast_AddAstChild(&parser->tree,node,statement);
        }
    }
    if (parser->position<parser->count&&kek_ast_IsOperatorToken(&parser->tokens[parser->position],OperatorKind_Greater)) {
        struct Token close=parser->tokens[parser->position];
        parser->position+=1;
        parser->tree.nodes.data[node].location=kek_ast_TokenLocation(open);
        parser->tree.nodes.data[node].location.length=(close.offset+close.length)-open.offset;
    } else {
        if (parser->position<parser->count) {
            kek_ast_ReportParseError(parser,&parser->tokens[parser->position],"expected '>'");
        } else {
            kek_ast_ReportParseError(parser,&open,"unterminated generic list");
        }
    }
    if (parser->tree.nodes.data[node].childCount>0&&parser->tree.nodes.data[node].location.length==open.length) {
        kek_ast_FinishLocationFromChildren(&parser->tree,node);
        parser->tree.nodes.data[node].location.offset=open.offset;
        parser->tree.nodes.data[node].location.line=open.line;
        parser->tree.nodes.data[node].location.column=open.column;
    }
    return (node);
}
usize kek_ast_ParseStatement(struct Parser* parser,u64 closePunctuation) {
    usize statement=kek_ast_CreateAstNode(parser,AstKind_Statement,kek_ast_TokenLocation(parser->tokens[parser->position]));
    while (parser->position<parser->count&&!kek_ast_IsAstTerminator(&parser->tokens[parser->position],closePunctuation)) {
        struct Token* token=&parser->tokens[parser->position];
        if (kek_ast_IsTriviaToken(token)) {
            parser->position+=1;
            continue;
        }
        if (kek_ast_IsPunctuationToken(token,PunctuationKind_Semicolon)||kek_ast_IsPunctuationToken(token,PunctuationKind_Comma)) {
            parser->position+=1;
            break;
        }
        if (kek_ast_IsClosingPunctuation(token)) {
            break;
        }
        if (kek_ast_IsPunctuationToken(token,PunctuationKind_LeftBrace)) {
            kek_ast_AddAstChild(&parser->tree,statement,kek_ast_ParseDelimited(parser,AstKind_Block,((u64)(PunctuationKind_RightBrace))));
            break;
        }
        if (kek_ast_IsPunctuationToken(token,PunctuationKind_LeftParen)) {
            kek_ast_AddAstChild(&parser->tree,statement,kek_ast_ParseDelimited(parser,AstKind_Group,((u64)(PunctuationKind_RightParen))));
            continue;
        }
        if (kek_ast_IsPunctuationToken(token,PunctuationKind_LeftBracket)) {
            kek_ast_AddAstChild(&parser->tree,statement,kek_ast_ParseDelimited(parser,AstKind_Index,((u64)(PunctuationKind_RightBracket))));
            continue;
        }
        if (kek_ast_ShouldParseGenericList(parser,parser->tree.nodes.data[statement].lastChild)) {
            kek_ast_AddAstChild(&parser->tree,statement,kek_ast_ParseGenericDelimited(parser));
            continue;
        }
        kek_ast_AddAstChild(&parser->tree,statement,kek_ast_ParseTokenNode(parser));
    }
    kek_ast_FinishLocationFromChildren(&parser->tree,statement);
    return (statement);
}
usize kek_ast_ParseList(struct Parser* parser,AstKind listKind,u64 closePunctuation) {
    usize list=kek_ast_CreateAstNode(parser,listKind,kek_ast_TokenLocation(parser->tokens[parser->position]));
    kek_ast_ParseChildrenInto(parser,list,closePunctuation);
    if (closePunctuation==11&&parser->position<parser->count&&kek_ast_IsClosingPunctuation(&parser->tokens[parser->position])) {
        kek_ast_ReportParseError(parser,&parser->tokens[parser->position],"unexpected closing delimiter");
        parser->position+=1;
    }
    kek_ast_FinishLocationFromChildren(&parser->tree,list);
    return (list);
}
Status kek_ast_ParseTokens(struct Token* tokens,usize count,struct String source,i64 fileIndex,struct Allocator allocator,struct DiagnosticBag* diagnostics,struct AstTree* out) {
    struct Parser parser={0};
    parser.tokens=tokens;
    parser.count=count;
    parser.position=0;
    parser.source=source;
    parser.fileIndex=fileIndex;
    parser.diagnostics=diagnostics;
    parser.errorCount=0;
    Status status=kek_ast_AstTreeInit(&parser.tree,allocator);
    if (status!=Status_Ok) {
        return (status);
    }
    usize root=kek_ast_ParseList(&parser,AstKind_File,11);
    parser.tree.root=root;
    parser.tree.nodes.data[root].location=kek_diagnostics_SourceLocationNew(1,1,0,source.len);
    out->nodes=parser.tree.nodes;
    out->root=parser.tree.root;
    return (Status_Ok);
}
int kek_compiler_CompilerFail(str text) {
    struct File stderr=std_file_Stderr();
    std_format_WriteStringToFile(&stderr,std_string_StringFromCString(text));
    return (1);
}
int kek_compiler_CompilerPrintHelp(void) {
    struct File stdout=std_file_Stdout();
    std_format_WriteStringToFile(&stdout,std_string_StringFromCString("kek build <input.kek> -o <output.c>\n"));
    std_format_WriteStringToFile(&stdout,std_string_StringFromCString("kek --help\n"));
    std_format_WriteStringToFile(&stdout,std_string_StringFromCString("kek --version\n"));
    return (0);
}
Status kek_compiler_CompileToCString(str inputPath,struct Allocator allocator,struct StringBuilder* out,struct DiagnosticBag* diagnostics) {
    struct CompilerContext program={0};
    Status status=kek_syntax_ProgramInit(&program,allocator);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_syntax_LoadSyntaxPackage(inputPath,&program);
    if (status==Status_Ok) {
        status=kek_module_BuildModule(&program);
    }
    if (status==Status_Ok) {
        status=kek_sema_CheckModule(&program);
    }
    if (status==Status_Ok) {
        status=kek_codegen_GenerateC(&program,out);
    }
    if (program.diagnostics.items.len>0) {
        for (usize i=0;i<program.diagnostics.items.len;i++) {
            struct Diagnostic* item=&program.diagnostics.items.data[i];
            kek_diagnostics_DiagnosticAdd(diagnostics,item->severity,item->phase,item->fileIndex,item->location,std_string_OwnedStringView(&item->message));
        }
    }
    kek_syntax_ProgramDestroy(&program);
    return (status);
}
Status kek_compiler_CompileToC(str inputPath,str outputPath,struct Allocator allocator,struct DiagnosticBag* diagnostics) {
    struct StringBuilder generated=std_string_StringBuilderNew(allocator);
    Status status=kek_compiler_CompileToCString(inputPath,allocator,&generated,diagnostics);
    if (status==Status_Ok) {
        struct String view=StringBuilder_View(&generated);
        status=std_file_WriteFile(outputPath,view);
    }
    StringBuilder_Destroy(&generated);
    return (status);
}
Status kek_compiler_CompilerRunBuild(str inputPath,str outputPath) {
    struct Allocator allocator=std_mem_DefaultAllocator();
    struct DiagnosticBag diagnostics={0};
    kek_diagnostics_DiagnosticBagInit(&diagnostics,allocator);
    Status status=kek_compiler_CompileToC(inputPath,outputPath,allocator,&diagnostics);
    if (diagnostics.items.len>0) {
        struct StringBuilder dump=std_string_StringBuilderNew(allocator);
        kek_diagnostics_WriteDiagnosticDump(&diagnostics,&dump);
        struct File stderr=std_file_Stderr();
        struct String view=StringBuilder_View(&dump);
        File_Write(&stderr,String_Bytes(&view));
        StringBuilder_Destroy(&dump);
    }
    kek_diagnostics_DiagnosticBagDestroy(&diagnostics);
    return (status);
}
int kek_compiler_CompilerMain(int argc,str* argv) {
    if (argc<2) {
        return (kek_compiler_CompilerFail("missing command\n"));
    }
    struct String command=std_string_StringFromCString(argv[1]);
    if (String_EqualsCString(&command,"--help")) {
        return (kek_compiler_CompilerPrintHelp());
    }
    if (String_EqualsCString(&command,"--version")) {
        struct File stdout=std_file_Stdout();
        std_format_WriteStringToFile(&stdout,std_string_StringFromCString("0.3.0-self\n"));
        return (0);
    }
    if (!String_EqualsCString(&command,"build")) {
        return (kek_compiler_CompilerFail("unknown command\n"));
    }
    if (argc<3) {
        return (kek_compiler_CompilerFail("missing input\n"));
    }
    str inputPath=argv[2];
    str outputPath="out.kek.c";
    usize i=3;
    while (i<((usize)(argc))) {
        struct String arg=std_string_StringFromCString(argv[i]);
        if (String_EqualsCString(&arg,"-o")) {
            if (i+1>=((usize)(argc))) {
                return (kek_compiler_CompilerFail("missing -o value\n"));
            }
            outputPath=argv[i+1];
            i+=2;
            continue;
        }
        return (kek_compiler_CompilerFail("unknown build option\n"));
    }
    Status status=kek_compiler_CompilerRunBuild(inputPath,outputPath);
    if (status!=Status_Ok) {
        return (kek_compiler_CompilerFail("build failed\n"));
    }
    return (0);
}
Status kek_codegen_WriteProgram(struct CompilerContext* program,struct StringBuilder* out) {
    Status status=kek_codegen_emit_WritePrelude(out);
    if (status==Status_Ok) {
        status=kek_codegen_types_WriteTypeDeclarations(program,out);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'\n');
    }
    if (status==Status_Ok) {
        status=kek_codegen_funcs_WriteFunctionDeclarations(program,out);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'\n');
    }
    if (status==Status_Ok) {
        status=kek_codegen_funcs_WriteFunctionDefinitions(program,out);
    }
    return (status);
}
Status kek_codegen_GenerateC(struct CompilerContext* program,struct StringBuilder* out) {
    return (kek_codegen_WriteProgram(program,out));
}
Status kek_codegen_funcs_WriteSubstTypeForFunc(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct FuncUse* funcUse,usize fileIndex,usize start,usize end) {
    struct TypeUse temp={0};
    kek_codegen_types_FuncUseTempTypeUse(funcUse,&temp);
    struct String firstTypeToken=kek_syntax_TokenText(program,fileIndex,start);
    if (start<end&&String_EqualsCString(&firstTypeToken,"ptr")&&start+1<end&&kek_syntax_IsOperator(program,fileIndex,start+1,OperatorKind_Less)) {
        usize genericEnd=kek_module_FindMatching(program,fileIndex,start+1);
        struct OwnedString innerKey={0};
        Status innerStatus=kek_codegen_types_MakeSubstTypeKey(program,decl,&temp,fileIndex,start+2,genericEnd,&innerKey);
        if (innerStatus==Status_Ok) {
            innerStatus=kek_sema_WriteCTypeFromKey(program,out,std_string_OwnedStringView(&innerKey));
        }
        if (innerStatus==Status_Ok) {
            innerStatus=StringBuilder_WriteByte(out,'*');
        }
        std_string_DestroyOwnedString(&innerKey);
        return (innerStatus);
    }
    struct OwnedString key={0};
    Status status=kek_codegen_types_MakeSubstTypeKey(program,decl,&temp,fileIndex,start,end,&key);
    if (status==Status_Ok) {
        status=kek_sema_WriteCTypeFromKey(program,out,std_string_OwnedStringView(&key));
    }
    if (status==Status_Ok) {
        std_string_DestroyOwnedString(&key);
    }
    return (status);
}
Status kek_codegen_funcs_WriteFunctionSignature(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct String nameOverride,struct FuncUse* funcUse) {
    struct String functionName=kek_syntax_TokenText(program,decl->fileIndex,decl->nameIndex);
    bool isMain=!decl->hasReceiver&&String_EqualsCString(&functionName,"main")&&kek_sema_ModuleDeclPackageIsRoot(decl);
    Status status=Status_Ok;
    if (isMain) {
        status=StringBuilder_WriteCString(out,"int");
    } else {
        if (funcUse!=0) {
            status=kek_codegen_funcs_WriteSubstTypeForFunc(program,out,decl,funcUse,decl->fileIndex,decl->returnStart,decl->returnEnd);
        } else {
            struct TypeInfo returnInfo={0};
            status=kek_sema_RenderTypeInfo(program,decl->fileIndex,decl->returnStart,decl->returnEnd,&returnInfo);
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&returnInfo.cType));
            }
            if (status==Status_Ok) {
                kek_sema_TypeInfoDestroy(&returnInfo);
            }
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,' ');
    }
    if (status==Status_Ok) {
        if (nameOverride.len>0) {
            status=StringBuilder_WriteString(out,nameOverride);
        } else {
            status=kek_sema_WriteDeclCName(program,out,decl);
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'(');
    }
    bool first=1;
    if (decl->hasReceiver) {
        if (funcUse!=0) {
            status=kek_codegen_funcs_WriteSubstTypeForFunc(program,out,decl,funcUse,decl->fileIndex,decl->receiverStart,decl->receiverEnd);
        } else {
            struct TypeInfo receiverInfo={0};
            if (status==Status_Ok) {
                status=kek_sema_RenderTypeInfo(program,decl->fileIndex,decl->receiverStart,decl->receiverEnd,&receiverInfo);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&receiverInfo.cType));
            }
            if (status==Status_Ok) {
                kek_sema_TypeInfoDestroy(&receiverInfo);
            }
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"* this");
        }
        first=0;
    }
    for (usize i=0;i<decl->paramCount;i++) {
        struct ModuleParam* param=&program->params.data[decl->firstParam+i];
        if (!first&&status==Status_Ok) {
            status=StringBuilder_WriteByte(out,',');
        }
        if (funcUse!=0) {
            if (status==Status_Ok) {
                status=kek_codegen_funcs_WriteSubstTypeForFunc(program,out,decl,funcUse,param->fileIndex,param->typeStart,param->typeEnd);
            }
        } else {
            struct TypeInfo paramInfo={0};
            if (status==Status_Ok) {
                status=kek_sema_RenderTypeInfo(program,param->fileIndex,param->typeStart,param->typeEnd,&paramInfo);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&paramInfo.cType));
            }
            if (status==Status_Ok) {
                kek_sema_TypeInfoDestroy(&paramInfo);
            }
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,' ');
        }
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,param->fileIndex,param->nameIndex);
        }
        first=0;
    }
    if (first&&status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"void");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,')');
    }
    return (status);
}
Status kek_codegen_funcs_WriteFunctionPrototype(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl) {
    Status status=kek_codegen_funcs_WriteFunctionSignature(program,out,decl,std_string_StringFromCString(""),0);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    return (status);
}
Status kek_codegen_funcs_WriteGenericFunctionBody(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct FuncUse* use) {
    Status status=kek_codegen_funcs_WriteFunctionSignature(program,out,decl,std_string_OwnedStringView(&use->cName),use);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," {\n");
    }
    struct CodegenEnv env={0};
    if (status==Status_Ok) {
        status=kek_codegen_state_EnvInit(program,&env);
    }
    if (status==Status_Ok) {
        env.genericDecl=decl;
        env.genericFuncUse=use;
    }
    struct OwnedString returnKey={0};
    struct OwnedString returnCType={0};
    bool returnIsPointer=0;
    if (status==Status_Ok) {
        status=kek_codegen_types_MakeSubstTypeInfoForFunc(program,decl,use,decl->fileIndex,decl->returnStart,decl->returnEnd,&returnKey,&returnCType,&returnIsPointer);
    }
    if (status==Status_Ok) {
        kek_sema_ReplaceOwned(&env.returnTypeKey,returnKey);
        kek_sema_ReplaceOwned(&env.returnCType,returnCType);
        returnKey.data=0;
        returnCType.data=0;
    }
    if (decl->hasReceiver&&status==Status_Ok) {
        struct OwnedString receiverKey={0};
        struct OwnedString receiverCType={0};
        bool receiverIsPointer=0;
        status=kek_codegen_types_MakeSubstTypeInfoForFunc(program,decl,use,decl->fileIndex,decl->receiverStart,decl->receiverEnd,&receiverKey,&receiverCType,&receiverIsPointer);
        if (status==Status_Ok) {
            env.hasThis=1;
            kek_sema_ReplaceOwned(&env.thisTypeKey,receiverKey);
            kek_sema_ReplaceOwned(&env.thisCType,receiverCType);
            receiverKey.data=0;
            receiverCType.data=0;
            status=kek_codegen_state_EnvAdd(program,&env,std_string_StringFromCString("this"),std_string_OwnedStringView(&env.thisTypeKey),std_string_OwnedStringView(&env.thisCType),0,std_string_StringFromCString(""),1);
        }
    }
    for (usize i=0;i<decl->paramCount&&status==Status_Ok;i++) {
        struct ModuleParam* param=&program->params.data[decl->firstParam+i];
        struct OwnedString paramKey={0};
        struct OwnedString paramCType={0};
        bool paramIsPointer=0;
        status=kek_codegen_types_MakeSubstTypeInfoForFunc(program,decl,use,param->fileIndex,param->typeStart,param->typeEnd,&paramKey,&paramCType,&paramIsPointer);
        if (status==Status_Ok) {
            status=kek_codegen_state_EnvAdd(program,&env,kek_syntax_TokenText(program,param->fileIndex,param->nameIndex),std_string_OwnedStringView(&paramKey),std_string_OwnedStringView(&paramCType),0,std_string_StringFromCString(""),paramIsPointer);
        }
        if (status==Status_Ok) {
            std_string_DestroyOwnedString(&paramKey);
            std_string_DestroyOwnedString(&paramCType);
        }
    }
    if (status==Status_Ok) {
        status=kek_codegen_stmt_WriteBlock(program,out,&env,decl->fileIndex,decl->bodyStart,decl->bodyEnd,1);
    }
    kek_codegen_state_EnvDestroy(&env);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"}\n");
    }
    return (status);
}
bool kek_codegen_funcs_GenericBodyMentionsGenericParam(struct CompilerContext* program,struct ModuleDecl* decl,struct FuncUse* use) {
    for (usize i=decl->bodyStart;i<decl->bodyEnd;i++) {
        struct Token token=program->tokenFiles.data[decl->fileIndex].tokens[i];
        if (!(token.kind==TokenKind_Identifier||token.kind==TokenKind_Keyword)) {
            continue;
        }
        struct String name=kek_syntax_TokenText(program,decl->fileIndex,i);
        for (usize argIndex=0;argIndex<use->args.len;argIndex++) {
            if (kek_codegen_types_GenericParamEquals(program,decl,argIndex,name)) {
                return (1);
            }
        }
    }
    return (0);
}
bool kek_codegen_funcs_CanLowerGenericFunctionBody(struct CompilerContext* program,struct ModuleDecl* decl,struct FuncUse* use) {
    if (!decl->hasBody) {
        return (0);
    }
    if (decl->hasReceiver) {
        return (0);
    }
    if (kek_codegen_funcs_GenericBodyMentionsGenericParam(program,decl,use)) {
        return (1);
    }
    return (1);
}
Status kek_codegen_funcs_WriteFunctionBody(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl) {
    Status status=kek_codegen_funcs_WriteFunctionSignature(program,out,decl,std_string_StringFromCString(""),0);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," {\n");
    }
    struct CodegenEnv env={0};
    if (status==Status_Ok) {
        status=kek_codegen_state_EnvInit(program,&env);
    }
    struct TypeInfo returnInfo={0};
    if (status==Status_Ok) {
        status=kek_sema_RenderTypeInfo(program,decl->fileIndex,decl->returnStart,decl->returnEnd,&returnInfo);
    }
    if (status==Status_Ok) {
        kek_sema_ReplaceOwned(&env.returnTypeKey,returnInfo.key);
        kek_sema_ReplaceOwned(&env.returnCType,returnInfo.cType);
        returnInfo.key.data=0;
        returnInfo.cType.data=0;
    }
    if (decl->hasReceiver&&status==Status_Ok) {
        struct TypeInfo receiverInfo={0};
        status=kek_sema_RenderTypeInfo(program,decl->fileIndex,decl->receiverStart,decl->receiverEnd,&receiverInfo);
        if (status==Status_Ok) {
            env.hasThis=1;
            kek_sema_ReplaceOwned(&env.thisTypeKey,receiverInfo.key);
            kek_sema_ReplaceOwned(&env.thisCType,receiverInfo.cType);
            receiverInfo.key.data=0;
            receiverInfo.cType.data=0;
            status=kek_codegen_state_EnvAdd(program,&env,std_string_StringFromCString("this"),std_string_OwnedStringView(&env.thisTypeKey),std_string_OwnedStringView(&env.thisCType),0,std_string_StringFromCString(""),1);
        }
        kek_sema_TypeInfoDestroy(&receiverInfo);
    }
    for (usize i=0;i<decl->paramCount&&status==Status_Ok;i++) {
        struct ModuleParam* param=&program->params.data[decl->firstParam+i];
        struct TypeInfo paramInfo={0};
        status=kek_sema_RenderTypeInfo(program,param->fileIndex,param->typeStart,param->typeEnd,&paramInfo);
        if (status==Status_Ok) {
            status=kek_codegen_state_EnvAdd(program,&env,kek_syntax_TokenText(program,param->fileIndex,param->nameIndex),std_string_OwnedStringView(&paramInfo.key),std_string_OwnedStringView(&paramInfo.cType),0,std_string_StringFromCString(""),paramInfo.isPointer);
        }
        kek_sema_TypeInfoDestroy(&paramInfo);
    }
    kek_sema_TypeInfoDestroy(&returnInfo);
    if (status==Status_Ok) {
        status=kek_codegen_stmt_WriteBlock(program,out,&env,decl->fileIndex,decl->bodyStart,decl->bodyEnd,1);
    }
    kek_codegen_state_EnvDestroy(&env);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"}\n");
    }
    return (status);
}
Status kek_codegen_funcs_WriteArrayMethodRef(struct StringBuilder* out,struct FuncUse* use,str name) {
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    CodeWriter_CString(&w,"Array__");
    CodeWriter_String(&w,kek_sema_OwnedStringArrayGet(&use->args,0));
    CodeWriter_Byte(&w,'_');
    CodeWriter_CString(&w,name);
    return (w.status);
}
Status kek_codegen_funcs_WriteLinkedListMethodRef(struct StringBuilder* out,struct FuncUse* use,str name) {
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    CodeWriter_CString(&w,"LinkedList__");
    CodeWriter_String(&w,kek_sema_OwnedStringArrayGet(&use->args,0));
    CodeWriter_Byte(&w,'_');
    CodeWriter_CString(&w,name);
    return (w.status);
}
Status kek_codegen_funcs_WriteResultTypeRef(struct StringBuilder* out,struct FuncUse* use) {
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    CodeWriter_CString(&w,"struct Result__");
    CodeWriter_String(&w,kek_sema_OwnedStringArrayGet(&use->args,0));
    return (w.status);
}
Status kek_codegen_funcs_WriteListNodeTypeRef(struct StringBuilder* out,struct FuncUse* use) {
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    CodeWriter_CString(&w,"struct ListNode__");
    CodeWriter_String(&w,kek_sema_OwnedStringArrayGet(&use->args,0));
    return (w.status);
}
Status kek_codegen_funcs_WriteManualArrayGenericBody(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct FuncUse* use,struct String argC) {
    struct String name=kek_syntax_TokenText(program,decl->fileIndex,decl->nameIndex);
    Status status=Status_Ok;
    if (String_EqualsCString(&name,"Destroy")) {
        status=kek_codegen_emit_WriteManualLine(out,"    if(this->data!=0){");
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteManualLine(out,"        kek_std_free(this->data);");
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteManualLine(out,"    }");
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteManualLine(out,"    this->data=0;");
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteManualLine(out,"    this->len=0;");
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteManualLine(out,"    this->cap=0;");
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteManualLine(out,"    return Status_Ok;");
        }
    } else {
        if (String_EqualsCString(&name,"Clear")) {
            status=kek_codegen_emit_WriteManualLine(out,"    this->len=0;");
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    return Status_Ok;");
            }
        } else {
            if (String_EqualsCString(&name,"Reserve")) {
                status=kek_codegen_emit_WriteManualLine(out,"    usize needed=this->len+additional;");
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    if(needed<=this->cap){return Status_Ok;}");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    usize newCap=this->cap;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    if(newCap==0){newCap=8;}");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    while(newCap<needed){newCap=newCap*2;}");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"    ");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteString(out,argC);
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"* newData=(");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteString(out,argC);
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"*)kek_std_resize(");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteByte(out,'\n');
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"        this->data,newCap*sizeof(");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteString(out,argC);
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"));");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    if(newData==0){return Status_NoMemory;}");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    this->data=newData;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    this->cap=newCap;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    return Status_Ok;");
                }
            } else {
                if (String_EqualsCString(&name,"AppendSlice")) {
                    status=StringBuilder_WriteCString(out,"    Status status=");
                    if (status==Status_Ok) {
                        status=kek_codegen_funcs_WriteArrayMethodRef(out,use,"Reserve");
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_emit_WriteManualLine(out,"(this,items.len);");
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_emit_WriteManualLine(out,"    if(status!=Status_Ok){return status;}");
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_emit_WriteManualLine(out,"    for(usize i=0;i<items.len;i++){");
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_emit_WriteManualLine(out,"        this->data[this->len+i]=items.data[i];");
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_emit_WriteManualLine(out,"    }");
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_emit_WriteManualLine(out,"    this->len+=items.len;");
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_emit_WriteManualLine(out,"    return Status_Ok;");
                    }
                } else {
                    if (String_EqualsCString(&name,"Push")) {
                        status=StringBuilder_WriteCString(out,"    Status status=");
                        if (status==Status_Ok) {
                            status=kek_codegen_funcs_WriteArrayMethodRef(out,use,"Reserve");
                        }
                        if (status==Status_Ok) {
                            status=kek_codegen_emit_WriteManualLine(out,"(this,1);");
                        }
                        if (status==Status_Ok) {
                            status=kek_codegen_emit_WriteManualLine(out,"    if(status!=Status_Ok){return status;}");
                        }
                        if (status==Status_Ok) {
                            status=kek_codegen_emit_WriteManualLine(out,"    this->data[this->len]=value;");
                        }
                        if (status==Status_Ok) {
                            status=kek_codegen_emit_WriteManualLine(out,"    this->len+=1;");
                        }
                        if (status==Status_Ok) {
                            status=kek_codegen_emit_WriteManualLine(out,"    return Status_Ok;");
                        }
                    } else {
                        if (String_EqualsCString(&name,"PushZeroed")) {
                            status=StringBuilder_WriteCString(out,"    Status status=");
                            if (status==Status_Ok) {
                                status=kek_codegen_funcs_WriteArrayMethodRef(out,use,"Reserve");
                            }
                            if (status==Status_Ok) {
                                status=kek_codegen_emit_WriteManualLine(out,"(this,1);");
                            }
                            if (status==Status_Ok) {
                                status=kek_codegen_emit_WriteManualLine(out,"    if(status!=Status_Ok){return status;}");
                            }
                            if (status==Status_Ok) {
                                status=StringBuilder_WriteCString(out,"    kek_std_mem_set((void*)(&this->data[this->len]),0,sizeof(");
                            }
                            if (status==Status_Ok) {
                                status=StringBuilder_WriteString(out,argC);
                            }
                            if (status==Status_Ok) {
                                status=kek_codegen_emit_WriteManualLine(out,"));");
                            }
                            if (status==Status_Ok) {
                                status=kek_codegen_emit_WriteManualLine(out,"    this->len+=1;");
                            }
                            if (status==Status_Ok) {
                                status=kek_codegen_emit_WriteManualLine(out,"    return Status_Ok;");
                            }
                        } else {
                            if (String_EqualsCString(&name,"Pop")) {
                                status=StringBuilder_WriteCString(out,"    ");
                                if (status==Status_Ok) {
                                    status=kek_codegen_funcs_WriteResultTypeRef(out,use);
                                }
                                if (status==Status_Ok) {
                                    status=kek_codegen_emit_WriteManualLine(out," result={0};");
                                }
                                if (status==Status_Ok) {
                                    status=kek_codegen_emit_WriteManualLine(out,"    if(this->len==0){result.status=Status_End;return result;}");
                                }
                                if (status==Status_Ok) {
                                    status=kek_codegen_emit_WriteManualLine(out,"    this->len-=1;");
                                }
                                if (status==Status_Ok) {
                                    status=kek_codegen_emit_WriteManualLine(out,"    result.status=Status_Ok;");
                                }
                                if (status==Status_Ok) {
                                    status=kek_codegen_emit_WriteManualLine(out,"    result.value=this->data[this->len];");
                                }
                                if (status==Status_Ok) {
                                    status=kek_codegen_emit_WriteManualLine(out,"    return result;");
                                }
                            } else {
                                if (String_EqualsCString(&name,"Get")) {
                                    status=StringBuilder_WriteCString(out,"    ");
                                    if (status==Status_Ok) {
                                        status=kek_codegen_funcs_WriteResultTypeRef(out,use);
                                    }
                                    if (status==Status_Ok) {
                                        status=kek_codegen_emit_WriteManualLine(out," result={0};");
                                    }
                                    if (status==Status_Ok) {
                                        status=kek_codegen_emit_WriteManualLine(out,"    if(index>=this->len){result.status=Status_Invalid;return result;}");
                                    }
                                    if (status==Status_Ok) {
                                        status=kek_codegen_emit_WriteManualLine(out,"    result.status=Status_Ok;");
                                    }
                                    if (status==Status_Ok) {
                                        status=kek_codegen_emit_WriteManualLine(out,"    result.value=this->data[index];");
                                    }
                                    if (status==Status_Ok) {
                                        status=kek_codegen_emit_WriteManualLine(out,"    return result;");
                                    }
                                } else {
                                    if (String_EqualsCString(&name,"Set")) {
                                        status=kek_codegen_emit_WriteManualLine(out,"    if(index>=this->len){return Status_Invalid;}");
                                        if (status==Status_Ok) {
                                            status=kek_codegen_emit_WriteManualLine(out,"    this->data[index]=value;");
                                        }
                                        if (status==Status_Ok) {
                                            status=kek_codegen_emit_WriteManualLine(out,"    return Status_Ok;");
                                        }
                                    } else {
                                        if (String_EqualsCString(&name,"GetPtr")) {
                                            status=kek_codegen_emit_WriteManualLine(out,"    if(index>=this->len){return 0;}");
                                            if (status==Status_Ok) {
                                                status=kek_codegen_emit_WriteManualLine(out,"    return &this->data[index];");
                                            }
                                        } else {
                                            if (String_EqualsCString(&name,"LastPtr")) {
                                                status=kek_codegen_emit_WriteManualLine(out,"    if(this->len==0){return 0;}");
                                                if (status==Status_Ok) {
                                                    status=kek_codegen_emit_WriteManualLine(out,"    return &this->data[this->len-1];");
                                                }
                                            } else {
                                                if (String_EqualsCString(&name,"Span")) {
                                                    status=StringBuilder_WriteCString(out,"    struct Span__");
                                                    if (status==Status_Ok) {
                                                        status=StringBuilder_WriteString(out,kek_sema_OwnedStringArrayGet(&use->args,0));
                                                    }
                                                    if (status==Status_Ok) {
                                                        status=kek_codegen_emit_WriteManualLine(out," out={0};");
                                                    }
                                                    if (status==Status_Ok) {
                                                        status=kek_codegen_emit_WriteManualLine(out,"    out.data=this->data;");
                                                    }
                                                    if (status==Status_Ok) {
                                                        status=kek_codegen_emit_WriteManualLine(out,"    out.len=this->len;");
                                                    }
                                                    if (status==Status_Ok) {
                                                        status=kek_codegen_emit_WriteManualLine(out,"    return out;");
                                                    }
                                                } else {
                                                    if (String_EqualsCString(&name,"Slice")) {
                                                        status=StringBuilder_WriteCString(out,"    struct Slice__");
                                                        if (status==Status_Ok) {
                                                            status=StringBuilder_WriteString(out,kek_sema_OwnedStringArrayGet(&use->args,0));
                                                        }
                                                        if (status==Status_Ok) {
                                                            status=kek_codegen_emit_WriteManualLine(out," out={0};");
                                                        }
                                                        if (status==Status_Ok) {
                                                            status=kek_codegen_emit_WriteManualLine(out,"    out.data=this->data;");
                                                        }
                                                        if (status==Status_Ok) {
                                                            status=kek_codegen_emit_WriteManualLine(out,"    out.len=this->len;");
                                                        }
                                                        if (status==Status_Ok) {
                                                            status=kek_codegen_emit_WriteManualLine(out,"    return out;");
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return (status);
}
Status kek_codegen_funcs_WriteManualLinkedListGenericBody(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct FuncUse* use) {
    struct String name=kek_syntax_TokenText(program,decl->fileIndex,decl->nameIndex);
    Status status=Status_Ok;
    if (String_EqualsCString(&name,"PushBack")||String_EqualsCString(&name,"PushFront")) {
        status=StringBuilder_WriteCString(out,"    ");
        if (status==Status_Ok) {
            status=kek_codegen_funcs_WriteListNodeTypeRef(out,use);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"* node=(");
        }
        if (status==Status_Ok) {
            status=kek_codegen_funcs_WriteListNodeTypeRef(out,use);
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteManualLine(out,"*)kek_std_alloc(");
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"        sizeof(");
        }
        if (status==Status_Ok) {
            status=kek_codegen_funcs_WriteListNodeTypeRef(out,use);
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteManualLine(out,"));");
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteManualLine(out,"    if(node==0){return Status_NoMemory;}");
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteManualLine(out,"    node->value=value;");
        }
        if (String_EqualsCString(&name,"PushBack")) {
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    node->next=0;");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    node->prev=this->last;");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    if(this->last!=0){");
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"        ");
            }
            if (status==Status_Ok) {
                status=kek_codegen_funcs_WriteListNodeTypeRef(out,use);
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"* last=this->last;");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"        last->next=node;");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    }else{");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"        this->first=node;");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    }");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    this->last=node;");
            }
        } else {
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    node->prev=0;");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    node->next=this->first;");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    if(this->first!=0){");
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"        ");
            }
            if (status==Status_Ok) {
                status=kek_codegen_funcs_WriteListNodeTypeRef(out,use);
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"* first=this->first;");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"        first->prev=node;");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    }else{");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"        this->last=node;");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    }");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    this->first=node;");
            }
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteManualLine(out,"    this->len+=1;");
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteManualLine(out,"    return Status_Ok;");
        }
    } else {
        if (String_EqualsCString(&name,"PopBack")||String_EqualsCString(&name,"PopFront")) {
            status=StringBuilder_WriteCString(out,"    ");
            if (status==Status_Ok) {
                status=kek_codegen_funcs_WriteResultTypeRef(out,use);
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out," result={0};");
            }
            if (String_EqualsCString(&name,"PopBack")) {
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    if(this->last==0){result.status=Status_End;return result;}");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"    ");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_funcs_WriteListNodeTypeRef(out,use);
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"* node=this->last;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    result.value=node->value;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    this->last=node->prev;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    if(this->last!=0){");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"        ");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_funcs_WriteListNodeTypeRef(out,use);
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"* last=this->last;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"        last->next=0;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    }else{");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"        this->first=0;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    }");
                }
            } else {
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    if(this->first==0){result.status=Status_End;return result;}");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"    ");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_funcs_WriteListNodeTypeRef(out,use);
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"* node=this->first;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    result.value=node->value;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    this->first=node->next;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    if(this->first!=0){");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"        ");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_funcs_WriteListNodeTypeRef(out,use);
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"* first=this->first;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"        first->prev=0;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    }else{");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"        this->last=0;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    }");
                }
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    this->len-=1;");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    kek_std_free(node);");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    result.status=Status_Ok;");
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteManualLine(out,"    return result;");
            }
        } else {
            if (String_EqualsCString(&name,"Destroy")) {
                status=kek_codegen_emit_WriteManualLine(out,"    while(this->len>0){");
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"        ");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_funcs_WriteResultTypeRef(out,use);
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out," ignored={0};");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"        ignored=");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_funcs_WriteLinkedListMethodRef(out,use,"PopBack");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"(this);");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"        if(ignored.status!=Status_Ok){return ignored.status;}");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    }");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    return Status_Ok;");
                }
            }
        }
    }
    return (status);
}
Status kek_codegen_funcs_WriteManualGenericBody(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct FuncUse* use) {
    struct String name=kek_syntax_TokenText(program,decl->fileIndex,decl->nameIndex);
    struct OwnedString argC={0};
    Status status=kek_sema_MakeCTypeFromKey(program,kek_sema_OwnedStringArrayGet(&use->args,0),&argC);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_codegen_funcs_WriteFunctionSignature(program,out,decl,std_string_OwnedStringView(&use->cName),use);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," {\n");
    }
    bool handled=0;
    if (decl->hasReceiver&&status==Status_Ok) {
        struct String receiverBase=kek_syntax_TokenText(program,decl->fileIndex,decl->receiverStart);
        if (String_EqualsCString(&receiverBase,"Array")) {
            status=kek_codegen_funcs_WriteManualArrayGenericBody(program,out,decl,use,std_string_OwnedStringView(&argC));
            handled=1;
        } else {
            if (String_EqualsCString(&receiverBase,"LinkedList")) {
                status=kek_codegen_funcs_WriteManualLinkedListGenericBody(program,out,decl,use);
                handled=1;
            }
        }
    }
    if (!handled&&String_EqualsCString(&name,"FixedSlice")) {
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"    struct Slice__");
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,kek_sema_OwnedStringArrayGet(&use->args,0));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out," out={0};\n    out.data=data;\n    out.len=len;\n    return out;\n");
        }
    } else {
        if (String_EqualsCString(&name,"FixedSpan")) {
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"    struct Span__");
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,kek_sema_OwnedStringArrayGet(&use->args,0));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out," out={0};\n    out.data=data;\n    out.len=len;\n    return out;\n");
            }
        } else {
            if (String_EqualsCString(&name,"ArrayNew")) {
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"    struct Array__");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteString(out,kek_sema_OwnedStringArrayGet(&use->args,0));
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out," array={0};");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    array.allocator=allocator;");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteManualLine(out,"    return array;");
                }
            } else {
                if (String_EqualsCString(&name,"LinkedListNew")) {
                    if (status==Status_Ok) {
                        status=StringBuilder_WriteCString(out,"    struct LinkedList__");
                    }
                    if (status==Status_Ok) {
                        status=StringBuilder_WriteString(out,kek_sema_OwnedStringArrayGet(&use->args,0));
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_emit_WriteManualLine(out," list={0};");
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_emit_WriteManualLine(out,"    list.allocator=allocator;");
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_emit_WriteManualLine(out,"    return list;");
                    }
                } else {
                    if (String_EqualsCString(&name,"Alloc")) {
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteCString(out,"    (void)allocator;\n");
                        }
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteCString(out,"    return (");
                        }
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&argC));
                        }
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteCString(out,"*)kek_std_alloc(count*sizeof(");
                        }
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&argC));
                        }
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteCString(out,"));\n");
                        }
                    } else {
                        if (String_EqualsCString(&name,"Resize")) {
                            if (status==Status_Ok) {
                                status=StringBuilder_WriteCString(out,"    (void)allocator;\n    (void)oldCount;\n");
                            }
                            if (status==Status_Ok) {
                                status=StringBuilder_WriteCString(out,"    return (");
                            }
                            if (status==Status_Ok) {
                                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&argC));
                            }
                            if (status==Status_Ok) {
                                status=StringBuilder_WriteCString(out,"*)kek_std_resize(oldData,newCount*sizeof(");
                            }
                            if (status==Status_Ok) {
                                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&argC));
                            }
                            if (status==Status_Ok) {
                                status=StringBuilder_WriteCString(out,"));\n");
                            }
                        } else {
                            if (String_EqualsCString(&name,"Free")) {
                                if (status==Status_Ok) {
                                    status=StringBuilder_WriteCString(out,"    (void)allocator;\n    (void)count;\n    kek_std_free(data);\n");
                                }
                            } else {
                                if (String_EqualsCString(&name,"Copy")) {
                                    if (status==Status_Ok) {
                                        status=StringBuilder_WriteCString(out,"    kek_std_mem_copy(dest,src,count*sizeof(");
                                    }
                                    if (status==Status_Ok) {
                                        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&argC));
                                    }
                                    if (status==Status_Ok) {
                                        status=StringBuilder_WriteCString(out,"));\n");
                                    }
                                } else {
                                    if (String_EqualsCString(&name,"GenericAdd")) {
                                        if (status==Status_Ok) {
                                            status=StringBuilder_WriteCString(out,"    return left+right;\n");
                                        }
                                    } else {
                                        if (String_EqualsCString(&name,"PkgGenericEcho")) {
                                            if (status==Status_Ok) {
                                                status=StringBuilder_WriteCString(out,"    return value;\n");
                                            }
                                        } else {
                                            if (String_EqualsCString(&name,"Ok")) {
                                                if (status==Status_Ok) {
                                                    status=StringBuilder_WriteCString(out,"    struct Result__");
                                                }
                                                if (status==Status_Ok) {
                                                    status=StringBuilder_WriteString(out,kek_sema_OwnedStringArrayGet(&use->args,0));
                                                }
                                                if (status==Status_Ok) {
                                                    status=StringBuilder_WriteCString(out," out={0};\n    out.status=Status_Ok;\n    out.value=value;\n    return out;\n");
                                                }
                                            } else {
                                                if (String_EqualsCString(&name,"Err")) {
                                                    if (status==Status_Ok) {
                                                        status=StringBuilder_WriteCString(out,"    struct Result__");
                                                    }
                                                    if (status==Status_Ok) {
                                                        status=StringBuilder_WriteString(out,kek_sema_OwnedStringArrayGet(&use->args,0));
                                                    }
                                                    if (status==Status_Ok) {
                                                        status=StringBuilder_WriteCString(out," out={0};\n    out.status=status;\n    return out;\n");
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"}\n");
    }
    std_string_DestroyOwnedString(&argC);
    return (status);
}
Status kek_codegen_funcs_WriteFunctionDeclarations(struct CompilerContext* program,struct StringBuilder* out) {
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if (decl->kind==DeclKind_Function&&!decl->isGeneric&&decl->reachable) {
            Status status=kek_codegen_funcs_WriteFunctionPrototype(program,out,decl);
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    for (usize i=0;i<program->funcUses.len;i++) {
        struct FuncUse* use=&program->funcUses.data[i];
        if (use->declIndex<program->decls.len) {
            Status status=kek_codegen_funcs_WriteFunctionSignature(program,out,&program->decls.data[use->declIndex],std_string_OwnedStringView(&use->cName),use);
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,";\n");
            }
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    return (Status_Ok);
}
Status kek_codegen_funcs_WriteFunctionDefinitions(struct CompilerContext* program,struct StringBuilder* out) {
    for (usize i=0;i<program->funcUses.len;i++) {
        struct FuncUse* use=&program->funcUses.data[i];
        if (use->declIndex<program->decls.len) {
            struct ModuleDecl* decl=&program->decls.data[use->declIndex];
            Status status=Status_Ok;
            if (kek_codegen_funcs_CanLowerGenericFunctionBody(program,decl,use)) {
                status=kek_codegen_funcs_WriteGenericFunctionBody(program,out,decl,use);
            } else {
                status=kek_codegen_funcs_WriteManualGenericBody(program,out,decl,use);
            }
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if (decl->kind==DeclKind_Function&&!decl->isGeneric&&decl->hasBody&&decl->reachable) {
            Status status=kek_codegen_funcs_WriteFunctionBody(program,out,decl);
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    return (Status_Ok);
}
usize kek_codegen_stmt_FindStatementSemicolon(struct CompilerContext* program,usize fileIndex,usize start,usize end) {
    usize i=start;
    while (i<end) {
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftParen)||kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftBrace)||kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftBracket)) {
            i=kek_module_SkipDelimited(program,fileIndex,i);
            continue;
        }
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_Semicolon)) {
            return (i);
        }
        i+=1;
    }
    return (end);
}
Status kek_codegen_stmt_ArrayLenString(struct CompilerContext* program,usize fileIndex,usize start,usize end,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
    Status status=kek_sema_WriteTokenRangeRaw(program,&builder,fileIndex,start,end);
    if (status==Status_Ok) {
        status=kek_syntax_DetachBuilder(&builder,out);
    }
    StringBuilder_Destroy(&builder);
    return (status);
}
bool kek_codegen_stmt_ModuleFieldDefaultIsZero(struct CompilerContext* program,struct ModuleField* field) {
    if (!field->hasDefault) {
        return (1);
    }
    if (field->defaultEnd!=field->defaultStart+1) {
        return (0);
    }
    if (kek_syntax_IsTokenKind(program,field->fileIndex,field->defaultStart,TokenKind_Number)&&kek_syntax_TokenEquals(program,field->fileIndex,field->defaultStart,"0")) {
        return (1);
    }
    if (kek_syntax_IsIdentifierText(program,field->fileIndex,field->defaultStart,"false")) {
        return (1);
    }
    return (0);
}
Status kek_codegen_stmt_WriteDefaultInitializer(struct CompilerContext* program,struct StringBuilder* out,struct String typeKey) {
    struct ModuleDecl* decl=kek_codegen_expr_FindFieldDecl(program,typeKey);
    if (decl!=0&&decl->kind==DeclKind_Union) {
        return (StringBuilder_WriteCString(out,"{0}"));
    }
    if (decl==0||decl->kind!=DeclKind_Struct) {
        return (StringBuilder_WriteCString(out,"0"));
    }
    bool hasNonZeroDefault=0;
    for (usize i=0;i<decl->fieldCount;i++) {
        struct ModuleField* field=&program->fields.data[decl->firstField+i];
        if (field->hasDefault&&!kek_codegen_stmt_ModuleFieldDefaultIsZero(program,field)) {
            hasNonZeroDefault=1;
            break;
        }
    }
    if (!hasNonZeroDefault) {
        return (StringBuilder_WriteCString(out,"{0}"));
    }
    Status status=StringBuilder_WriteByte(out,'{');
    bool first=1;
    for (usize i=0;i<decl->fieldCount;i++) {
        struct ModuleField* field=&program->fields.data[decl->firstField+i];
        if (!field->hasDefault||kek_codegen_stmt_ModuleFieldDefaultIsZero(program,field)) {
            continue;
        }
        if (!first&&status==Status_Ok) {
            status=StringBuilder_WriteByte(out,',');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,'.');
        }
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,field->fileIndex,field->nameIndex);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,'=');
        }
        struct Expr defaultExpr={0};
        struct CodegenEnv emptyEnv={0};
        kek_codegen_state_EnvInit(program,&emptyEnv);
        if (status==Status_Ok) {
            status=kek_codegen_expr_CompileExpressionRange(program,&emptyEnv,field->fileIndex,field->defaultStart,field->defaultEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&defaultExpr);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&defaultExpr.text));
        }
        if (status==Status_Ok) {
            kek_codegen_expr_ExprDestroy(&defaultExpr);
        }
        kek_codegen_state_EnvDestroy(&emptyEnv);
        first=0;
        if (status!=Status_Ok) {
            return (status);
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'}');
    }
    return (status);
}
Status kek_codegen_stmt_WriteVarDecl(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize semicolon,usize indent) {
    usize colon=kek_module_FindTopLevelColon(program,fileIndex,start,semicolon);
    if (colon>=semicolon) {
        return (Status_Invalid);
    }
    usize nameIndex=colon+1;
    usize afterName=nameIndex+1;
    bool isArray=0;
    usize arrayStart=semicolon;
    usize arrayEnd=semicolon;
    if (afterName<semicolon&&kek_syntax_IsPunctuation(program,fileIndex,afterName,PunctuationKind_LeftBracket)) {
        isArray=1;
        arrayStart=afterName+1;
        arrayEnd=kek_module_FindMatching(program,fileIndex,afterName);
        afterName=arrayEnd+1;
        while (afterName<semicolon&&kek_syntax_IsPunctuation(program,fileIndex,afterName,PunctuationKind_LeftBracket)) {
            afterName=kek_module_FindMatching(program,fileIndex,afterName)+1;
        }
    }
    usize arraySuffixStart=nameIndex+1;
    usize arraySuffixEnd=afterName;
    usize initStart=semicolon;
    bool hasInit=0;
    for (usize i=afterName;i<semicolon;i++) {
        if (kek_syntax_IsOperator(program,fileIndex,i,OperatorKind_Assign)) {
            initStart=i+1;
            hasInit=1;
            break;
        }
    }
    struct TypeInfo typeInfo={0};
    Status status=kek_codegen_expr_RenderEnvTypeInfo(program,env,fileIndex,start,colon,&typeInfo);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_codegen_emit_WriteIndent(out,indent);
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&typeInfo.cType));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,' ');
    }
    if (status==Status_Ok) {
        status=kek_syntax_WriteToken(program,out,fileIndex,nameIndex);
    }
    struct OwnedString arrayLen={0};
    if (isArray) {
        if (status==Status_Ok) {
            status=kek_sema_WriteTokenRangeRaw(program,out,fileIndex,arraySuffixStart,arraySuffixEnd);
        }
        if (status==Status_Ok) {
            status=kek_codegen_stmt_ArrayLenString(program,fileIndex,arrayStart,arrayEnd,&arrayLen);
        }
    } else {
        if (status==Status_Ok) {
            status=kek_syntax_MakeOwnedEmpty(program,&arrayLen);
        }
    }
    if (hasInit) {
        struct Expr init={0};
        if (status==Status_Ok) {
            status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,initStart,semicolon,std_string_OwnedStringView(&typeInfo.key),std_string_OwnedStringView(&typeInfo.cType),isArray,&init);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,'=');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&init.text));
        }
        if (status==Status_Ok) {
            kek_codegen_expr_ExprDestroy(&init);
        }
    } else {
        if (!isArray) {
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(out,'=');
            }
            if (status==Status_Ok) {
                status=kek_codegen_stmt_WriteDefaultInitializer(program,out,std_string_OwnedStringView(&typeInfo.key));
            }
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    if (status==Status_Ok) {
        status=kek_codegen_state_EnvAdd(program,env,kek_syntax_TokenText(program,fileIndex,nameIndex),std_string_OwnedStringView(&typeInfo.key),std_string_OwnedStringView(&typeInfo.cType),isArray,std_string_OwnedStringView(&arrayLen),typeInfo.isPointer);
    }
    std_string_DestroyOwnedString(&arrayLen);
    kek_sema_TypeInfoDestroy(&typeInfo);
    return (status);
}
Status kek_codegen_stmt_WriteExprStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize semicolon,usize indent) {
    if (start>=semicolon) {
        return (Status_Ok);
    }
    if (start+2==semicolon&&kek_syntax_IsOperator(program,fileIndex,start+1,OperatorKind_Plus)&&kek_syntax_IsOperator(program,fileIndex,start+2,OperatorKind_Plus)) {
    }
    struct Expr expr={0};
    Status status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,start,semicolon,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&expr);
    if (status!=Status_Ok) {
        kek_codegen_expr_ExprDestroy(&expr);
        return (status);
    }
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    CodeWriter_Indent(&w,indent);
    CodeWriter_String(&w,std_string_OwnedStringView(&expr.text));
    CodeWriter_CString(&w,";\n");
    status=w.status;
    kek_codegen_expr_ExprDestroy(&expr);
    return (status);
}
Status kek_codegen_stmt_WriteIfStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    usize groupStart=kek_module_FindNextGroup(program,fileIndex,start,kek_syntax_FileTokenCount(program,fileIndex));
    usize groupEnd=kek_module_FindMatching(program,fileIndex,groupStart);
    usize blockStart=groupEnd+1;
    usize blockEnd=blockStart;
    struct Expr cond={0};
    Status status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&cond);
    if (status==Status_Ok) {
        status=kek_codegen_emit_WriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"if (");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&cond.text));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,") ");
    }
    kek_codegen_expr_ExprDestroy(&cond);
    if (blockStart<kek_syntax_FileTokenCount(program,fileIndex)&&kek_syntax_IsPunctuation(program,fileIndex,blockStart,PunctuationKind_LeftBrace)) {
        blockEnd=kek_module_FindMatching(program,fileIndex,blockStart);
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"{\n");
        }
        if (status==Status_Ok) {
            status=kek_codegen_stmt_WriteBlock(program,out,env,fileIndex,blockStart+1,blockEnd,indent+1);
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"}");
        }
        outNext[0]=blockEnd+1;
    } else {
        usize semicolon=kek_codegen_stmt_FindStatementSemicolon(program,fileIndex,blockStart,kek_syntax_FileTokenCount(program,fileIndex));
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"{\n");
        }
        if (status==Status_Ok) {
            status=kek_codegen_stmt_WriteExprStatement(program,out,env,fileIndex,blockStart,semicolon,indent+1);
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"}");
        }
        outNext[0]=semicolon+1;
    }
    if (outNext[0]<kek_syntax_FileTokenCount(program,fileIndex)&&kek_syntax_IsKeyword(program,fileIndex,outNext[0],KeywordKind_Else)) {
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out," else ");
        }
        usize elseStart=outNext[0]+1;
        if (elseStart<kek_syntax_FileTokenCount(program,fileIndex)&&kek_syntax_IsPunctuation(program,fileIndex,elseStart,PunctuationKind_LeftBrace)) {
            usize elseEnd=kek_module_FindMatching(program,fileIndex,elseStart);
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"{\n");
            }
            if (status==Status_Ok) {
                status=kek_codegen_stmt_WriteBlock(program,out,env,fileIndex,elseStart+1,elseEnd,indent+1);
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteIndent(out,indent);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"}");
            }
            outNext[0]=elseEnd+1;
        } else {
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"{\n");
            }
            usize semicolon=kek_codegen_stmt_FindStatementSemicolon(program,fileIndex,elseStart,kek_syntax_FileTokenCount(program,fileIndex));
            if (status==Status_Ok) {
                status=kek_codegen_stmt_WriteExprStatement(program,out,env,fileIndex,elseStart,semicolon,indent+1);
            }
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteIndent(out,indent);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"}");
            }
            outNext[0]=semicolon+1;
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'\n');
    }
    return (status);
}
Status kek_codegen_stmt_WriteWhileStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    usize groupStart=kek_module_FindNextGroup(program,fileIndex,start,kek_syntax_FileTokenCount(program,fileIndex));
    usize groupEnd=kek_module_FindMatching(program,fileIndex,groupStart);
    usize blockStart=groupEnd+1;
    usize blockEnd=kek_module_FindMatching(program,fileIndex,blockStart);
    struct Expr cond={0};
    Status status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&cond);
    if (status!=Status_Ok) {
        kek_codegen_expr_ExprDestroy(&cond);
        return (status);
    }
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    CodeWriter_Indent(&w,indent);
    CodeWriter_CString(&w,"while (");
    CodeWriter_String(&w,std_string_OwnedStringView(&cond.text));
    CodeWriter_CString(&w,") {\n");
    status=w.status;
    kek_codegen_expr_ExprDestroy(&cond);
    if (status==Status_Ok) {
        status=kek_codegen_stmt_WriteBlock(program,out,env,fileIndex,blockStart+1,blockEnd,indent+1);
    }
    w.status=status;
    CodeWriter_Indent(&w,indent);
    CodeWriter_CString(&w,"}\n");
    status=w.status;
    outNext[0]=blockEnd+1;
    return (status);
}
Status kek_codegen_stmt_WriteDoStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    usize blockStart=start+1;
    usize blockEnd=kek_module_FindMatching(program,fileIndex,blockStart);
    usize whileIndex=blockEnd+1;
    usize groupStart=kek_module_FindNextGroup(program,fileIndex,whileIndex,kek_syntax_FileTokenCount(program,fileIndex));
    usize groupEnd=kek_module_FindMatching(program,fileIndex,groupStart);
    struct Expr cond={0};
    Status status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&cond);
    if (status!=Status_Ok) {
        kek_codegen_expr_ExprDestroy(&cond);
        return (status);
    }
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    CodeWriter_Indent(&w,indent);
    CodeWriter_CString(&w,"do {\n");
    status=w.status;
    if (status==Status_Ok) {
        status=kek_codegen_stmt_WriteBlock(program,out,env,fileIndex,blockStart+1,blockEnd,indent+1);
    }
    w.status=status;
    CodeWriter_Indent(&w,indent);
    CodeWriter_CString(&w,"} while (");
    CodeWriter_String(&w,std_string_OwnedStringView(&cond.text));
    CodeWriter_CString(&w,");\n");
    status=w.status;
    kek_codegen_expr_ExprDestroy(&cond);
    outNext[0]=groupEnd+2;
    return (status);
}
Status kek_codegen_stmt_WriteForStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    usize groupStart=kek_module_FindNextGroup(program,fileIndex,start,kek_syntax_FileTokenCount(program,fileIndex));
    usize groupEnd=kek_module_FindMatching(program,fileIndex,groupStart);
    usize firstSemi=kek_module_FindTokenAtDepthZero(program,fileIndex,groupStart+1,groupEnd,PunctuationKind_Semicolon);
    usize secondSemi=kek_module_FindTokenAtDepthZero(program,fileIndex,firstSemi+1,groupEnd,PunctuationKind_Semicolon);
    usize blockStart=groupEnd+1;
    usize blockEnd=kek_module_FindMatching(program,fileIndex,blockStart);
    Status status=kek_codegen_emit_WriteIndent(out,indent);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"for (");
    }
    usize colon=kek_module_FindTopLevelColon(program,fileIndex,groupStart+1,firstSemi);
    if (colon<firstSemi) {
        struct TypeInfo typeInfo={0};
        if (status==Status_Ok) {
            status=kek_codegen_expr_RenderEnvTypeInfo(program,env,fileIndex,groupStart+1,colon,&typeInfo);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&typeInfo.cType));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,' ');
        }
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,fileIndex,colon+1);
        }
        if (status==Status_Ok) {
            status=kek_codegen_state_EnvAdd(program,env,kek_syntax_TokenText(program,fileIndex,colon+1),std_string_OwnedStringView(&typeInfo.key),std_string_OwnedStringView(&typeInfo.cType),0,std_string_StringFromCString(""),typeInfo.isPointer);
        }
        usize eq=colon+2;
        while (eq<firstSemi&&!kek_syntax_IsOperator(program,fileIndex,eq,OperatorKind_Assign)) {
            eq+=1;
        }
        if (eq<firstSemi) {
            struct Expr init={0};
            if (status==Status_Ok) {
                status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,eq+1,firstSemi,std_string_OwnedStringView(&typeInfo.key),std_string_OwnedStringView(&typeInfo.cType),0,&init);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(out,'=');
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&init.text));
            }
            if (status==Status_Ok) {
                kek_codegen_expr_ExprDestroy(&init);
            }
        }
        kek_sema_TypeInfoDestroy(&typeInfo);
    } else {
        struct Expr initExpr={0};
        if (status==Status_Ok) {
            status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,groupStart+1,firstSemi,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&initExpr);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&initExpr.text));
        }
        if (status==Status_Ok) {
            kek_codegen_expr_ExprDestroy(&initExpr);
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,';');
    }
    struct Expr cond={0};
    if (status==Status_Ok) {
        status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,firstSemi+1,secondSemi,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&cond);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&cond.text));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,';');
    }
    if (status==Status_Ok) {
        status=kek_sema_WriteTokenRangeRaw(program,out,fileIndex,secondSemi+1,groupEnd);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,") {\n");
    }
    if (status==Status_Ok) {
        status=kek_codegen_stmt_WriteBlock(program,out,env,fileIndex,blockStart+1,blockEnd,indent+1);
    }
    if (status==Status_Ok) {
        status=kek_codegen_emit_WriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"}\n");
    }
    kek_codegen_expr_ExprDestroy(&cond);
    outNext[0]=blockEnd+1;
    return (status);
}
Status kek_codegen_stmt_WriteEachStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    usize genericStart=start+1;
    usize genericEnd=kek_module_FindMatching(program,fileIndex,genericStart);
    usize groupStart=genericEnd+1;
    usize groupEnd=kek_module_FindMatching(program,fileIndex,groupStart);
    usize blockStart=groupEnd+1;
    usize blockEnd=kek_module_FindMatching(program,fileIndex,blockStart);
    usize comma=kek_module_FindTokenAtDepthZero(program,fileIndex,genericStart+1,genericEnd,PunctuationKind_Comma);
    usize valueTypeStart=genericStart+1;
    usize valueTypeEnd=genericEnd;
    usize valueNameIndex=genericStart+1;
    bool hasIndex=0;
    usize indexTypeStart=genericStart+1;
    usize indexTypeEnd=genericStart+1;
    usize indexNameIndex=genericStart+1;
    if (comma<genericEnd) {
        hasIndex=1;
        usize indexColon=kek_module_FindTopLevelColon(program,fileIndex,genericStart+1,comma);
        indexTypeStart=genericStart+1;
        indexTypeEnd=indexColon;
        indexNameIndex=indexColon+1;
        usize valueColon=kek_module_FindTopLevelColon(program,fileIndex,comma+1,genericEnd);
        valueTypeStart=comma+1;
        valueTypeEnd=valueColon;
        valueNameIndex=valueColon+1;
    } else {
        usize valueColon=kek_module_FindTopLevelColon(program,fileIndex,genericStart+1,genericEnd);
        valueTypeStart=genericStart+1;
        valueTypeEnd=valueColon;
        valueNameIndex=valueColon+1;
    }
    struct TypeInfo valueType={0};
    Status status=kek_codegen_expr_RenderEnvTypeInfo(program,env,fileIndex,valueTypeStart,valueTypeEnd,&valueType);
    if (status!=Status_Ok) {
        return (status);
    }
    if (groupStart+2<groupEnd&&kek_syntax_IsIdentifierText(program,fileIndex,groupStart+1,"range")) {
        usize rangeGroup=groupStart+4;
        while (rangeGroup<groupEnd&&!kek_syntax_IsPunctuation(program,fileIndex,rangeGroup,PunctuationKind_LeftParen)) {
            rangeGroup+=1;
        }
        usize rangeEnd=kek_module_FindMatching(program,fileIndex,rangeGroup);
        usize firstComma=kek_module_FindTokenAtDepthZero(program,fileIndex,rangeGroup+1,rangeEnd,PunctuationKind_Comma);
        usize secondComma=kek_module_FindTokenAtDepthZero(program,fileIndex,firstComma+1,rangeEnd,PunctuationKind_Comma);
        struct Expr beginExpr={0};
        struct Expr endExpr={0};
        struct Expr stepExpr={0};
        status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,rangeGroup+1,firstComma,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&beginExpr);
        usize rangeValueEnd=rangeEnd;
        if (secondComma<rangeEnd) {
            rangeValueEnd=secondComma;
        }
        if (status==Status_Ok) {
            status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,firstComma+1,rangeValueEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&endExpr);
        }
        if (secondComma<rangeEnd) {
            if (status==Status_Ok) {
                status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,secondComma+1,rangeEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&stepExpr);
            }
        } else {
            if (status==Status_Ok) {
                kek_codegen_expr_ExprInitEmpty(program,&stepExpr);
                status=kek_codegen_expr_ExprSetCString(program,&stepExpr,"1");
            }
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"for (");
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&valueType.cType));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,' ');
        }
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,fileIndex,valueNameIndex);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,'=');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&beginExpr.text));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,';');
        }
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,fileIndex,valueNameIndex);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,'<');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&endExpr.text));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,';');
        }
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,fileIndex,valueNameIndex);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"+=");
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&stepExpr.text));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,") {\n");
        }
        if (status==Status_Ok) {
            status=kek_codegen_state_EnvAdd(program,env,kek_syntax_TokenText(program,fileIndex,valueNameIndex),std_string_OwnedStringView(&valueType.key),std_string_OwnedStringView(&valueType.cType),0,std_string_StringFromCString(""),valueType.isPointer);
        }
        if (status==Status_Ok) {
            status=kek_codegen_stmt_WriteBlock(program,out,env,fileIndex,blockStart+1,blockEnd,indent+1);
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"}\n");
        }
        kek_codegen_expr_ExprDestroy(&beginExpr);
        kek_codegen_expr_ExprDestroy(&endExpr);
        kek_codegen_expr_ExprDestroy(&stepExpr);
    } else {
        struct Expr arrayExpr={0};
        status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&arrayExpr);
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"for (");
        }
        if (hasIndex) {
            struct TypeInfo indexType={0};
            if (status==Status_Ok) {
                status=kek_codegen_expr_RenderEnvTypeInfo(program,env,fileIndex,indexTypeStart,indexTypeEnd,&indexType);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&indexType.cType));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(out,' ');
            }
            if (status==Status_Ok) {
                status=kek_syntax_WriteToken(program,out,fileIndex,indexNameIndex);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"=0;");
            }
            if (status==Status_Ok) {
                status=kek_syntax_WriteToken(program,out,fileIndex,indexNameIndex);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(out,'<');
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&arrayExpr.arrayLen));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(out,';');
            }
            if (status==Status_Ok) {
                status=kek_syntax_WriteToken(program,out,fileIndex,indexNameIndex);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"++) {\n");
            }
            if (status==Status_Ok) {
                status=kek_codegen_state_EnvAdd(program,env,kek_syntax_TokenText(program,fileIndex,indexNameIndex),std_string_OwnedStringView(&indexType.key),std_string_OwnedStringView(&indexType.cType),0,std_string_StringFromCString(""),indexType.isPointer);
            }
            kek_sema_TypeInfoDestroy(&indexType);
        } else {
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"usize __kek_each_idx=0;__kek_each_idx<");
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&arrayExpr.arrayLen));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,";__kek_each_idx++) {\n");
            }
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteIndent(out,indent+1);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&valueType.cType));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,' ');
        }
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,fileIndex,valueNameIndex);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,'=');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&arrayExpr.text));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,'[');
        }
        if (hasIndex) {
            if (status==Status_Ok) {
                status=kek_syntax_WriteToken(program,out,fileIndex,indexNameIndex);
            }
        } else {
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"__kek_each_idx");
            }
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"];\n");
        }
        if (status==Status_Ok) {
            status=kek_codegen_state_EnvAdd(program,env,kek_syntax_TokenText(program,fileIndex,valueNameIndex),std_string_OwnedStringView(&valueType.key),std_string_OwnedStringView(&valueType.cType),0,std_string_StringFromCString(""),valueType.isPointer);
        }
        if (status==Status_Ok) {
            status=kek_codegen_stmt_WriteBlock(program,out,env,fileIndex,blockStart+1,blockEnd,indent+1);
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"}\n");
        }
        kek_codegen_expr_ExprDestroy(&arrayExpr);
    }
    kek_sema_TypeInfoDestroy(&valueType);
    outNext[0]=blockEnd+1;
    return (status);
}
Status kek_codegen_stmt_WriteSwitchStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    usize groupStart=kek_module_FindNextGroup(program,fileIndex,start,kek_syntax_FileTokenCount(program,fileIndex));
    usize groupEnd=kek_module_FindMatching(program,fileIndex,groupStart);
    usize blockStart=groupEnd+1;
    usize blockEnd=kek_module_FindMatching(program,fileIndex,blockStart);
    struct Expr value={0};
    Status status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&value);
    if (status==Status_Ok) {
        status=kek_codegen_emit_WriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"switch (");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&value.text));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,") {\n");
    }
    kek_codegen_expr_ExprDestroy(&value);
    usize i=blockStart+1;
    while (i<blockEnd&&status==Status_Ok) {
        if (kek_syntax_IsKeyword(program,fileIndex,i,KeywordKind_Case)) {
            usize caseGroup=i+1;
            usize caseGroupEnd=kek_module_FindMatching(program,fileIndex,caseGroup);
            struct Expr caseExpr={0};
            status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,caseGroup+1,caseGroupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&caseExpr);
            if (status==Status_Ok) {
                status=kek_codegen_emit_WriteIndent(out,indent+1);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"case ");
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&caseExpr.text));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,": ");
            }
            kek_codegen_expr_ExprDestroy(&caseExpr);
            usize caseBlock=caseGroupEnd+1;
            if (caseBlock<blockEnd&&kek_syntax_IsPunctuation(program,fileIndex,caseBlock,PunctuationKind_Colon)) {
                caseBlock+=1;
            }
            if (caseBlock<blockEnd&&kek_syntax_IsPunctuation(program,fileIndex,caseBlock,PunctuationKind_LeftBrace)) {
                usize caseBlockEnd=kek_module_FindMatching(program,fileIndex,caseBlock);
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"{\n");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_stmt_WriteBlock(program,out,env,fileIndex,caseBlock+1,caseBlockEnd,indent+2);
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteIndent(out,indent+1);
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"}\n");
                }
                i=caseBlockEnd+1;
            } else {
                usize semicolon=kek_codegen_stmt_FindStatementSemicolon(program,fileIndex,caseBlock,blockEnd);
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"{\n");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_stmt_WriteExprStatement(program,out,env,fileIndex,caseBlock,semicolon,indent+2);
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteIndent(out,indent+1);
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"}\n");
                }
                i=semicolon+1;
            }
            continue;
        }
        if (kek_syntax_IsKeyword(program,fileIndex,i,KeywordKind_Default)) {
            status=kek_codegen_emit_WriteIndent(out,indent+1);
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"default: ");
            }
            usize bodyStart=i+1;
            if (bodyStart<blockEnd&&kek_syntax_IsPunctuation(program,fileIndex,bodyStart,PunctuationKind_Colon)) {
                bodyStart+=1;
            }
            if (bodyStart<blockEnd&&kek_syntax_IsPunctuation(program,fileIndex,bodyStart,PunctuationKind_LeftBrace)) {
                usize bodyEnd=kek_module_FindMatching(program,fileIndex,bodyStart);
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"{\n");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_stmt_WriteBlock(program,out,env,fileIndex,bodyStart+1,bodyEnd,indent+2);
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteIndent(out,indent+1);
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"}\n");
                }
                i=bodyEnd+1;
            } else {
                usize semicolon=kek_codegen_stmt_FindStatementSemicolon(program,fileIndex,bodyStart,blockEnd);
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"{\n");
                }
                if (status==Status_Ok) {
                    status=kek_codegen_stmt_WriteExprStatement(program,out,env,fileIndex,bodyStart,semicolon,indent+2);
                }
                if (status==Status_Ok) {
                    status=kek_codegen_emit_WriteIndent(out,indent+1);
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"}\n");
                }
                i=semicolon+1;
            }
            continue;
        }
        i+=1;
    }
    if (status==Status_Ok) {
        status=kek_codegen_emit_WriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"}\n");
    }
    outNext[0]=blockEnd+1;
    return (status);
}
struct ModuleField* kek_codegen_stmt_FindTaggedUnionVariant(struct CompilerContext* program,struct ModuleDecl* decl,struct String name,usize* outIndex) {
    for (usize i=0;i<decl->fieldCount;i++) {
        struct ModuleField* field=&program->fields.data[decl->firstField+i];
        struct String fieldName=kek_syntax_TokenText(program,field->fileIndex,field->nameIndex);
        if (String_Equals(&fieldName,name)) {
            outIndex[0]=i;
            return (field);
        }
    }
    return (0);
}
bool kek_codegen_stmt_SelectMatchedVariant(struct Array__usize* matched,usize index) {
    for (usize i=0;i<matched->len;i++) {
        if (matched->data[i]==index) {
            return (1);
        }
    }
    return (0);
}
Status kek_codegen_stmt_WriteSelectTempName(struct StringBuilder* out,usize index) {
    Status status=StringBuilder_WriteCString(out,"__kek_select_");
    if (status==Status_Ok) {
        status=std_format_FormatU64ToBuilder(out,((u64)(index)),10);
    }
    return (status);
}
Status kek_codegen_stmt_WriteSelectStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    usize groupStart=kek_module_FindNextGroup(program,fileIndex,start,kek_syntax_FileTokenCount(program,fileIndex));
    usize groupEnd=kek_module_FindMatching(program,fileIndex,groupStart);
    usize blockStart=groupEnd+1;
    usize blockEnd=kek_module_FindMatching(program,fileIndex,blockStart);
    if (groupStart>=kek_syntax_FileTokenCount(program,fileIndex)||groupEnd>=kek_syntax_FileTokenCount(program,fileIndex)||blockStart>=kek_syntax_FileTokenCount(program,fileIndex)||!kek_syntax_IsPunctuation(program,fileIndex,blockStart,PunctuationKind_LeftBrace)) {
        return (Status_Invalid);
    }
    struct Expr value={0};
    Status status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&value);
    struct ModuleDecl* decl=0;
    if (status==Status_Ok) {
        decl=kek_codegen_expr_FindFieldDecl(program,std_string_OwnedStringView(&value.typeKey));
        if (decl==0||decl->kind!=DeclKind_Union||!decl->isTagged) {
            status=Status_Invalid;
        }
    }
    usize selectIndex=env->selectCounter;
    env->selectCounter+=1;
    if (status==Status_Ok) {
        status=kek_codegen_emit_WriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&value.cType));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,' ');
    }
    if (status==Status_Ok) {
        status=kek_codegen_stmt_WriteSelectTempName(out,selectIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," = ");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&value.text));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    if (status==Status_Ok) {
        status=kek_codegen_emit_WriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"switch (");
    }
    if (status==Status_Ok) {
        status=kek_codegen_stmt_WriteSelectTempName(out,selectIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,".tag) {\n");
    }
    struct Array__usize matched=std_array_ArrayNew__usize(program->allocator);
    usize i=blockStart+1;
    while (i<blockEnd&&status==Status_Ok) {
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_Semicolon)||kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_Comma)) {
            i+=1;
            continue;
        }
        if (!(kek_syntax_IsTokenKind(program,fileIndex,i,TokenKind_Identifier)||kek_syntax_IsTokenKind(program,fileIndex,i,TokenKind_Keyword))) {
            status=Status_Invalid;
            break;
        }
        struct String armTypeName=kek_syntax_TokenText(program,fileIndex,i);
        struct String declName=kek_syntax_TokenText(program,decl->fileIndex,decl->nameIndex);
        if (!String_Equals(&armTypeName,declName)) {
            status=Status_Invalid;
            break;
        }
        if (i+4>=blockEnd||!kek_syntax_IsOperator(program,fileIndex,i+1,OperatorKind_Scope)) {
            status=Status_Invalid;
            break;
        }
        usize variantIndex=0;
        struct ModuleField* field=kek_codegen_stmt_FindTaggedUnionVariant(program,decl,kek_syntax_TokenText(program,fileIndex,i+2),&variantIndex);
        if (field==0||kek_codegen_stmt_SelectMatchedVariant(&matched,variantIndex)) {
            status=Status_Invalid;
            break;
        }
        usize bindGroup=i+3;
        if (!kek_syntax_IsPunctuation(program,fileIndex,bindGroup,PunctuationKind_LeftParen)) {
            status=Status_Invalid;
            break;
        }
        usize bindGroupEnd=kek_module_FindMatching(program,fileIndex,bindGroup);
        if (bindGroupEnd!=bindGroup+2||!kek_syntax_IsTokenKind(program,fileIndex,bindGroup+1,TokenKind_Identifier)) {
            status=Status_Invalid;
            break;
        }
        if (bindGroupEnd+2>=blockEnd||!kek_syntax_IsPunctuation(program,fileIndex,bindGroupEnd+1,PunctuationKind_Colon)||!kek_syntax_IsPunctuation(program,fileIndex,bindGroupEnd+2,PunctuationKind_LeftBrace)) {
            status=Status_Invalid;
            break;
        }
        usize armBlock=bindGroupEnd+2;
        usize armBlockEnd=kek_module_FindMatching(program,fileIndex,armBlock);
        if (armBlockEnd>blockEnd) {
            status=Status_Invalid;
            break;
        }
        status=Array__usize_Push(&matched,variantIndex);
        struct TypeInfo fieldType={0};
        bool fieldTypeReady=0;
        if (status==Status_Ok) {
            status=kek_codegen_expr_RenderEnvTypeInfo(program,env,field->fileIndex,field->typeStart,field->typeEnd,&fieldType);
            fieldTypeReady=status==Status_Ok;
        }
        usize localCount=env->locals.len;
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteIndent(out,indent+1);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"case ");
        }
        if (status==Status_Ok) {
            status=kek_codegen_types_WriteTaggedUnionVariantName(program,out,decl,field);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,": {\n");
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteIndent(out,indent+2);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&fieldType.cType));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,' ');
        }
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,fileIndex,bindGroup+1);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out," = ");
        }
        if (status==Status_Ok) {
            status=kek_codegen_stmt_WriteSelectTempName(out,selectIndex);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,".data.");
        }
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,field->fileIndex,field->nameIndex);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,";\n");
        }
        if (status==Status_Ok) {
            status=kek_codegen_state_EnvAdd(program,env,kek_syntax_TokenText(program,fileIndex,bindGroup+1),std_string_OwnedStringView(&fieldType.key),std_string_OwnedStringView(&fieldType.cType),0,std_string_StringFromCString(""),fieldType.isPointer);
        }
        if (status==Status_Ok) {
            status=kek_codegen_stmt_WriteBlock(program,out,env,fileIndex,armBlock+1,armBlockEnd,indent+2);
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteIndent(out,indent+2);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"break;\n");
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteIndent(out,indent+1);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"}\n");
        }
        kek_codegen_state_EnvRestore(env,localCount);
        if (fieldTypeReady) {
            kek_sema_TypeInfoDestroy(&fieldType);
        }
        i=armBlockEnd+1;
    }
    if (status==Status_Ok&&matched.len!=decl->fieldCount) {
        status=Status_Invalid;
    }
    if (status==Status_Ok) {
        status=kek_codegen_emit_WriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"}\n");
    }
    struct Array__usize matchedValues=matched;
    Status destroyStatus=Array__usize_Destroy(&matchedValues);
    matched=matchedValues;
    kek_codegen_expr_ExprDestroy(&value);
    if (status==Status_Ok&&destroyStatus!=Status_Ok) {
        status=destroyStatus;
    }
    outNext[0]=blockEnd+1;
    return (status);
}
Status kek_codegen_stmt_WriteReturnStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize semicolon,usize indent) {
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    CodeWriter_Indent(&w,indent);
    CodeWriter_CString(&w,"return");
    if (start+1<semicolon) {
        struct Expr value={0};
        if (w.status==Status_Ok) {
            w.status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,start+1,semicolon,std_string_OwnedStringView(&env->returnTypeKey),std_string_OwnedStringView(&env->returnCType),0,&value);
            if (w.status==Status_Ok) {
                CodeWriter_Byte(&w,' ');
                CodeWriter_String(&w,std_string_OwnedStringView(&value.text));
                kek_codegen_expr_ExprDestroy(&value);
            }
        }
    }
    CodeWriter_CString(&w,";\n");
    return (w.status);
}
Status kek_codegen_stmt_WriteSingleStatement(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    if (kek_syntax_IsPunctuation(program,fileIndex,start,PunctuationKind_LeftBrace)) {
        usize blockEnd=kek_module_FindMatching(program,fileIndex,start);
        Status status=kek_codegen_emit_WriteIndent(out,indent);
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"{\n");
        }
        if (status==Status_Ok) {
            status=kek_codegen_stmt_WriteBlock(program,out,env,fileIndex,start+1,blockEnd,indent+1);
        }
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"}\n");
        }
        outNext[0]=blockEnd+1;
        return (status);
    }
    if (kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_If)) {
        return (kek_codegen_stmt_WriteIfStatement(program,out,env,fileIndex,start,outNext,indent));
    }
    if (kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_While)) {
        return (kek_codegen_stmt_WriteWhileStatement(program,out,env,fileIndex,start,outNext,indent));
    }
    if (kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_Do)) {
        return (kek_codegen_stmt_WriteDoStatement(program,out,env,fileIndex,start,outNext,indent));
    }
    if (kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_For)) {
        return (kek_codegen_stmt_WriteForStatement(program,out,env,fileIndex,start,outNext,indent));
    }
    if (kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_Each)) {
        return (kek_codegen_stmt_WriteEachStatement(program,out,env,fileIndex,start,outNext,indent));
    }
    if (kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_Switch)) {
        return (kek_codegen_stmt_WriteSwitchStatement(program,out,env,fileIndex,start,outNext,indent));
    }
    if (kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_Select)) {
        return (kek_codegen_stmt_WriteSelectStatement(program,out,env,fileIndex,start,outNext,indent));
    }
    usize semicolon=kek_codegen_stmt_FindStatementSemicolon(program,fileIndex,start,kek_syntax_FileTokenCount(program,fileIndex));
    if (kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_Return)) {
        outNext[0]=semicolon+1;
        return (kek_codegen_stmt_WriteReturnStatement(program,out,env,fileIndex,start,semicolon,indent));
    }
    if (kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_Break)) {
        outNext[0]=semicolon+1;
        Status status=kek_codegen_emit_WriteIndent(out,indent);
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"break;\n");
        }
        return (status);
    }
    if (kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_Continue)) {
        outNext[0]=semicolon+1;
        Status status=kek_codegen_emit_WriteIndent(out,indent);
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"continue;\n");
        }
        return (status);
    }
    if (kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_Unreachable)) {
        outNext[0]=semicolon+1;
        Status status=kek_codegen_emit_WriteIndent(out,indent);
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"__builtin_unreachable();\n");
        }
        return (status);
    }
    if (kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_Panic)) {
        outNext[0]=semicolon+1;
        usize groupStart=start+1;
        usize groupEnd=kek_module_FindMatching(program,fileIndex,groupStart);
        struct Expr message={0};
        Status status=kek_codegen_expr_CompileExpressionRange(program,env,fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&message);
        if (status==Status_Ok) {
            status=kek_codegen_emit_WriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"do { fprintf(stderr, \"panic: %s\\n\", ");
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&message.text));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"); abort(); } while(0);\n");
        }
        kek_codegen_expr_ExprDestroy(&message);
        return (status);
    }
    if (kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_Defer)) {
        outNext[0]=semicolon+1;
        return (Status_Ok);
    }
    usize colon=kek_module_FindTopLevelColon(program,fileIndex,start,semicolon);
    if (colon<semicolon) {
        outNext[0]=semicolon+1;
        return (kek_codegen_stmt_WriteVarDecl(program,out,env,fileIndex,start,semicolon,indent));
    }
    outNext[0]=semicolon+1;
    return (kek_codegen_stmt_WriteExprStatement(program,out,env,fileIndex,start,semicolon,indent));
}
Status kek_codegen_stmt_WriteDeferred(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize end,usize indent) {
    if (start>=end||!kek_syntax_IsKeyword(program,fileIndex,start,KeywordKind_Defer)) {
        return (Status_Ok);
    }
    usize body=start+1;
    if (body<end&&kek_syntax_IsPunctuation(program,fileIndex,body,PunctuationKind_LeftBrace)) {
        usize blockEnd=kek_module_FindMatching(program,fileIndex,body);
        return (kek_codegen_stmt_WriteBlock(program,out,env,fileIndex,body+1,blockEnd,indent));
    }
    usize semicolon=kek_codegen_stmt_FindStatementSemicolon(program,fileIndex,body,end);
    return (kek_codegen_stmt_WriteExprStatement(program,out,env,fileIndex,body,semicolon,indent));
}
bool kek_codegen_stmt_IsBlockTransfer(struct CompilerContext* program,usize fileIndex,usize index) {
    return (kek_syntax_IsKeyword(program,fileIndex,index,KeywordKind_Return)||kek_syntax_IsKeyword(program,fileIndex,index,KeywordKind_Break)||kek_syntax_IsKeyword(program,fileIndex,index,KeywordKind_Continue));
}
Status kek_codegen_stmt_WriteDeferredRanges(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,struct Array__DeferRange* defers,usize indent) {
    usize count=defers->len;
    while (count>0) {
        count-=1;
        struct DeferRange* entry=&defers->data[count];
        Status status=kek_codegen_stmt_WriteDeferred(program,out,env,fileIndex,entry->start,entry->end,indent);
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
Status kek_codegen_stmt_WriteBlock(struct CompilerContext* program,struct StringBuilder* out,struct CodegenEnv* env,usize fileIndex,usize start,usize end,usize indent) {
    usize localStart=env->locals.len;
    struct Array__DeferRange defers=std_array_ArrayNew__DeferRange(program->allocator);
    usize index=start;
    Status status=Status_Ok;
    while (index<end&&status==Status_Ok) {
        if (kek_syntax_IsKeyword(program,fileIndex,index,KeywordKind_Defer)) {
            usize deferEnd=index+1;
            if (deferEnd<end&&kek_syntax_IsPunctuation(program,fileIndex,deferEnd,PunctuationKind_LeftBrace)) {
                deferEnd=kek_module_FindMatching(program,fileIndex,deferEnd)+1;
            } else {
                deferEnd=kek_codegen_stmt_FindStatementSemicolon(program,fileIndex,deferEnd,end)+1;
            }
            struct DeferRange entry={0};
            entry.start=index;
            entry.end=deferEnd;
            status=Array__DeferRange_Push(&defers,entry);
            if (status!=Status_Ok) {
                break;
            }
            index=deferEnd;
            continue;
        }
        usize next=index+1;
        if (kek_codegen_stmt_IsBlockTransfer(program,fileIndex,index)) {
            status=kek_codegen_stmt_WriteDeferredRanges(program,out,env,fileIndex,&defers,indent);
        }
        if (status!=Status_Ok) {
            break;
        }
        status=kek_codegen_stmt_WriteSingleStatement(program,out,env,fileIndex,index,&next,indent);
        if (next<=index) {
            next=index+1;
        }
        index=next;
    }
    while (defers.len>0&&status==Status_Ok) {
        struct Result__DeferRange popped=Array__DeferRange_Pop(&defers);
        if (popped.status!=Status_Ok) {
            status=popped.status;
            break;
        }
        status=kek_codegen_stmt_WriteDeferred(program,out,env,fileIndex,popped.value.start,popped.value.end,indent);
    }
    Status restoreStatus=kek_codegen_state_EnvRestore(env,localStart);
    if (status==Status_Ok&&restoreStatus!=Status_Ok) {
        status=restoreStatus;
    }
    Status destroyStatus=Array__DeferRange_Destroy(&defers);
    if (status==Status_Ok&&destroyStatus!=Status_Ok) {
        status=destroyStatus;
    }
    return (status);
}
Status kek_codegen_expr_ExprInitEmpty(struct CompilerContext* program,struct Expr* expr) {
    Status status=kek_syntax_MakeOwnedEmpty(program,&expr->text);
    if (status==Status_Ok) {
        status=kek_syntax_MakeOwnedEmpty(program,&expr->typeKey);
    }
    if (status==Status_Ok) {
        status=kek_syntax_MakeOwnedEmpty(program,&expr->cType);
    }
    if (status==Status_Ok) {
        status=kek_syntax_MakeOwnedEmpty(program,&expr->arrayLen);
    }
    expr->isArray=0;
    expr->isLvalue=0;
    expr->isPointer=0;
    return (status);
}
Status kek_codegen_expr_ExprDestroy(struct Expr* expr) {
    std_string_DestroyOwnedString(&expr->text);
    std_string_DestroyOwnedString(&expr->typeKey);
    std_string_DestroyOwnedString(&expr->cType);
    std_string_DestroyOwnedString(&expr->arrayLen);
    return (Status_Ok);
}
Status kek_codegen_expr_ExprSetText(struct CompilerContext* program,struct Expr* expr,struct String text) {
    struct OwnedString owned={0};
    Status status=kek_syntax_CloneContextString(program,text,&owned);
    if (status!=Status_Ok) {
        return (status);
    }
    return (kek_sema_ReplaceOwned(&expr->text,owned));
}
Status kek_codegen_expr_ExprSetCString(struct CompilerContext* program,struct Expr* expr,str text) {
    struct OwnedString owned={0};
    Status status=kek_syntax_CloneContextCString(program,text,&owned);
    if (status!=Status_Ok) {
        return (status);
    }
    return (kek_sema_ReplaceOwned(&expr->text,owned));
}
bool kek_codegen_expr_IsTripleQuotedString(struct String text) {
    if (text.len<6) {
        return (0);
    }
    return (text.data[0]=='"'&&text.data[1]=='"'&&text.data[2]=='"'&&text.data[text.len-3]=='"'&&text.data[text.len-2]=='"'&&text.data[text.len-1]=='"');
}
Status kek_codegen_expr_WriteOctalEscape(struct StringBuilder* out,byte value) {
    Status status=StringBuilder_WriteByte(out,'\\');
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'0'+value/64);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'0'+(value/8)%8);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'0'+value%8);
    }
    return (status);
}
Status kek_codegen_expr_WriteCStringEscapedByte(struct StringBuilder* out,byte value) {
    if (value=='\n') {
        return (StringBuilder_WriteCString(out,"\\n"));
    }
    if (value=='\r') {
        return (StringBuilder_WriteCString(out,"\\r"));
    }
    if (value=='\t') {
        return (StringBuilder_WriteCString(out,"\\t"));
    }
    if (value=='"') {
        return (StringBuilder_WriteCString(out,"\\\""));
    }
    if (value=='\\') {
        return (StringBuilder_WriteCString(out,"\\\\"));
    }
    if (value<32||value>=127) {
        return (kek_codegen_expr_WriteOctalEscape(out,value));
    }
    return (StringBuilder_WriteByte(out,value));
}
Status kek_codegen_expr_WriteStringLiteralToken(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize index) {
    struct String text=kek_syntax_TokenText(program,fileIndex,index);
    if (!kek_codegen_expr_IsTripleQuotedString(text)) {
        return (StringBuilder_WriteString(out,text));
    }
    Status status=StringBuilder_WriteByte(out,'"');
    for (usize i=3;i+3<text.len&&status==Status_Ok;i++) {
        status=kek_codegen_expr_WriteCStringEscapedByte(out,text.data[i]);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'"');
    }
    return (status);
}
Status kek_codegen_expr_ExprSetStringLiteralToken(struct CompilerContext* program,struct Expr* expr,usize fileIndex,usize index) {
    struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
    Status status=kek_codegen_expr_WriteStringLiteralToken(program,&builder,fileIndex,index);
    if (status==Status_Ok) {
        status=kek_codegen_expr_ExprFromBuilder(expr,&builder);
    }
    StringBuilder_Destroy(&builder);
    return (status);
}
Status kek_codegen_expr_ExprSetType(struct CompilerContext* program,struct Expr* expr,struct String key,struct String cType,bool isPointer) {
    struct OwnedString keyOwned={0};
    Status status=kek_syntax_CloneContextString(program,key,&keyOwned);
    if (status==Status_Ok) {
        status=kek_sema_ReplaceOwned(&expr->typeKey,keyOwned);
    }
    struct OwnedString cOwned={0};
    if (status==Status_Ok) {
        status=kek_syntax_CloneContextString(program,cType,&cOwned);
    }
    if (status==Status_Ok) {
        status=kek_sema_ReplaceOwned(&expr->cType,cOwned);
    }
    expr->isPointer=isPointer;
    return (status);
}
Status kek_codegen_expr_RenderEnvTypeInfo(struct CompilerContext* program,struct CodegenEnv* env,usize fileIndex,usize start,usize end,struct TypeInfo* info) {
    if (env->genericFuncUse==0||env->genericDecl==0) {
        return (kek_sema_RenderTypeInfo(program,fileIndex,start,end,info));
    }
    Status status=kek_sema_TypeInfoInitEmpty(program,info);
    if (status!=Status_Ok) {
        return (status);
    }
    struct OwnedString key={0};
    struct OwnedString cType={0};
    bool isPointer=0;
    status=kek_codegen_types_MakeSubstTypeInfoForFunc(program,env->genericDecl,env->genericFuncUse,fileIndex,start,end,&key,&cType,&isPointer);
    if (status==Status_Ok) {
        status=kek_sema_ReplaceOwned(&info->key,key);
        key.data=0;
    }
    if (status==Status_Ok) {
        status=kek_sema_ReplaceOwned(&info->cType,cType);
        cType.data=0;
    }
    info->isPointer=isPointer;
    return (status);
}
Status kek_codegen_expr_ExprFromBuilder(struct Expr* expr,struct StringBuilder* builder) {
    struct OwnedString owned={0};
    Status status=kek_syntax_DetachBuilder(builder,&owned);
    if (status!=Status_Ok) {
        return (status);
    }
    return (kek_sema_ReplaceOwned(&expr->text,owned));
}
u8 kek_codegen_expr_OperatorPrecedence(struct CompilerContext* program,usize fileIndex,usize index) {
    if (kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_Assign)||kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_PlusAssign)||kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_MinusAssign)) {
        return (1);
    }
    if (kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_LogicalOr)) {
        return (2);
    }
    if (kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_LogicalAnd)) {
        return (3);
    }
    if (kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_Equal)||kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_NotEqual)||kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_Less)||kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_LessEqual)||kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_Greater)||kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_GreaterEqual)) {
        return (4);
    }
    if (kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_Plus)||kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_Minus)) {
        return (5);
    }
    if (kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_Multiply)||kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_Divide)||kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_Modulo)) {
        return (6);
    }
    return (0);
}
bool kek_codegen_expr_OperatorRightAssociative(struct CompilerContext* program,usize fileIndex,usize index) {
    return (kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_Assign)||kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_PlusAssign)||kek_syntax_IsOperator(program,fileIndex,index,OperatorKind_MinusAssign));
}
Status kek_codegen_expr_WriteNumberToken(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize index) {
    struct String text=kek_syntax_TokenText(program,fileIndex,index);
    if (text.len>2&&text.data[0]=='0'&&(text.data[1]=='b'||text.data[1]=='B')) {
        u64 value=0;
        for (usize i=2;i<text.len;i++) {
            if (text.data[i]=='_') {
                continue;
            }
            if (text.data[i]=='0'||text.data[i]=='1') {
                value=value*2+((u64)(text.data[i]-'0'));
            }
        }
        Status status=StringBuilder_WriteCString(out,"0x");
        if (status==Status_Ok) {
            status=std_format_FormatU64ToBuilder(out,value,16);
        }
        return (status);
    }
    for (usize i=0;i<text.len;i++) {
        if (text.data[i]!='_') {
            Status status=StringBuilder_WriteByte(out,text.data[i]);
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    return (Status_Ok);
}
struct ModuleDecl* kek_codegen_expr_FindFieldDecl(struct CompilerContext* program,struct String typeKey) {
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if ((decl->kind==DeclKind_Struct||decl->kind==DeclKind_Union)&&kek_module_ModuleDeclNameMatches(program,decl,typeKey)) {
            return (decl);
        }
    }
    for (usize i=0;i<program->typeUses.len;i++) {
        struct String useKey=std_string_OwnedStringView(&program->typeUses.data[i].key);
        if (String_Equals(&useKey,typeKey)) {
            return (kek_module_FindTypeDecl(program,std_string_OwnedStringView(&program->typeUses.data[i].baseName)));
        }
    }
    return (0);
}
Status kek_codegen_expr_NestedFieldTypeKey(struct CompilerContext* program,usize fieldIndex,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
    Status status=std_format_FormatU64ToBuilder(&builder,((u64)(fieldIndex)),10);
    if (status==Status_Ok) {
        status=kek_syntax_DetachBuilder(&builder,out);
    }
    StringBuilder_Destroy(&builder);
    return (status);
}
bool kek_codegen_expr_NestedFieldIndexFromKey(struct String typeKey,usize* out) {
    if (typeKey.len==0||typeKey.data[0]<'0'||typeKey.data[0]>'9') {
        return (0);
    }
    usize value=0;
    for (usize i=0;i<typeKey.len;i++) {
        byte c=typeKey.data[i];
        if (c<'0'||c>'9') {
            return (0);
        }
        value=value*10+((usize)(c-'0'));
    }
    out[0]=value;
    return (1);
}
struct TypeUse* kek_codegen_expr_FindTypeUse(struct CompilerContext* program,struct String typeKey) {
    for (usize i=0;i<program->typeUses.len;i++) {
        struct String useKey=std_string_OwnedStringView(&program->typeUses.data[i].key);
        if (String_Equals(&useKey,typeKey)) {
            return (&program->typeUses.data[i]);
        }
    }
    return (0);
}
Status kek_codegen_expr_ModuleFieldType(struct CompilerContext* program,struct String typeKey,struct String fieldName,struct TypeInfo* outInfo) {
    kek_sema_TypeInfoInitEmpty(program,outInfo);
    usize firstField=0;
    usize fieldCount=0;
    usize nestedIndex=0;
    if (kek_codegen_expr_NestedFieldIndexFromKey(typeKey,&nestedIndex)) {
        if (nestedIndex>=program->fields.len||!program->fields.data[nestedIndex].isNestedStruct) {
            return (Status_Ok);
        }
        firstField=program->fields.data[nestedIndex].nestedFirstField;
        fieldCount=program->fields.data[nestedIndex].nestedFieldCount;
    } else {
        struct ModuleDecl* fieldDecl=kek_codegen_expr_FindFieldDecl(program,typeKey);
        if (fieldDecl==0) {
            return (Status_Ok);
        }
        if (fieldDecl->kind==DeclKind_Union&&fieldDecl->isTagged) {
            if (String_EqualsCString(&fieldName,"tag")) {
                struct StringBuilder tagKey=std_string_StringBuilderNew(program->allocator);
                Status status=kek_syntax_WriteToken(program,&tagKey,fieldDecl->fileIndex,fieldDecl->nameIndex);
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(&tagKey,"Tag");
                }
                struct OwnedString key={0};
                if (status==Status_Ok) {
                    status=kek_syntax_DetachBuilder(&tagKey,&key);
                }
                StringBuilder_Destroy(&tagKey);
                if (status==Status_Ok) {
                    status=kek_sema_TypeInfoFromKey(program,std_string_OwnedStringView(&key),outInfo);
                }
                std_string_DestroyOwnedString(&key);
                return (status);
            }
            if (String_EqualsCString(&fieldName,"data")) {
                return (kek_sema_TypeInfoFromKey(program,typeKey,outInfo));
            }
        }
        firstField=fieldDecl->firstField;
        fieldCount=fieldDecl->fieldCount;
    }
    struct TypeUse* use=kek_codegen_expr_FindTypeUse(program,typeKey);
    struct ModuleDecl* decl=0;
    if (use!=0) {
        decl=kek_module_FindTypeDecl(program,std_string_OwnedStringView(&use->baseName));
    }
    for (usize i=0;i<fieldCount;i++) {
        usize fieldIndex=firstField+i;
        struct ModuleField* field=&program->fields.data[fieldIndex];
        struct String fieldToken=kek_syntax_TokenText(program,field->fileIndex,field->nameIndex);
        if (String_Equals(&fieldToken,fieldName)) {
            if (field->isNestedStruct) {
                struct OwnedString key={0};
                Status status=kek_codegen_expr_NestedFieldTypeKey(program,fieldIndex,&key);
                if (status==Status_Ok) {
                    status=kek_sema_ReplaceOwned(&outInfo->key,key);
                }
                return (status);
            }
            kek_sema_TypeInfoDestroy(outInfo);
            if (use!=0&&decl!=0) {
                return (kek_codegen_types_MakeSubstTypeInfo(program,decl,use,field->fileIndex,field->typeStart,field->typeEnd,outInfo));
            }
            return (kek_sema_RenderTypeInfo(program,field->fileIndex,field->typeStart,field->typeEnd,outInfo));
        }
    }
    return (kek_module_ModuleError(program,"unknown struct field"));
}
struct ModuleDecl* kek_codegen_expr_FindMethod(struct CompilerContext* program,struct String receiverType,struct String name,bool isOperator,u8 operatorCode,usize argCount) {
    struct ModuleDecl* indexed=kek_sema_FindIndexedMethodDecl(program,receiverType,name,isOperator,operatorCode,argCount);
    if (indexed!=0) {
        return (indexed);
    }
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if (decl->kind!=DeclKind_Function||!decl->hasReceiver) {
            continue;
        }
        struct StringBuilder receiver=std_string_StringBuilderNew(program->allocator);
        Status status=kek_sema_WriteTypeSuffixFromRange(program,&receiver,decl->fileIndex,decl->receiverStart,decl->receiverEnd);
        if (status!=Status_Ok) {
            StringBuilder_Destroy(&receiver);
            continue;
        }
        struct String receiverView=StringBuilder_View(&receiver);
        bool receiverMatches=String_Equals(&receiverView,receiverType);
        StringBuilder_Destroy(&receiver);
        if (!receiverMatches) {
            continue;
        }
        if (isOperator) {
            if (decl->isOperator&&decl->operatorCode==operatorCode&&decl->paramCount==argCount) {
                return (decl);
            }
        } else {
            struct String methodName=kek_syntax_TokenText(program,decl->fileIndex,decl->nameIndex);
            if (!decl->isOperator&&String_Equals(&methodName,name)) {
                return (decl);
            }
        }
    }
    return (0);
}
Status kek_codegen_expr_ParserInit(struct CompilerContext* program,struct CodegenEnv* env,usize fileIndex,usize start,usize end,struct String expectedKey,struct String expectedCType,bool expectedIsArray,struct ExprParser* parser) {
    parser->program=program;
    parser->env=env;
    parser->fileIndex=fileIndex;
    parser->pos=start;
    parser->end=end;
    parser->expectedIsArray=expectedIsArray;
    Status status=kek_syntax_CloneContextString(program,expectedKey,&parser->expectedTypeKey);
    if (status==Status_Ok) {
        status=kek_syntax_CloneContextString(program,expectedCType,&parser->expectedCType);
    }
    return (status);
}
Status kek_codegen_expr_ParserDestroy(struct ExprParser* parser) {
    std_string_DestroyOwnedString(&parser->expectedTypeKey);
    std_string_DestroyOwnedString(&parser->expectedCType);
    return (Status_Ok);
}
Status kek_codegen_expr_CompileExpressionRange(struct CompilerContext* program,struct CodegenEnv* env,usize fileIndex,usize start,usize end,struct String expectedKey,struct String expectedCType,bool expectedIsArray,struct Expr* out) {
    struct ExprParser parser={0};
    Status status=kek_codegen_expr_ParserInit(program,env,fileIndex,start,end,expectedKey,expectedCType,expectedIsArray,&parser);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_codegen_expr_CompileExpression(&parser,1,out);
    kek_codegen_expr_ParserDestroy(&parser);
    return (status);
}
Status kek_codegen_expr_WriteInitializerList(struct ExprParser* parser,usize start,usize end,struct StringBuilder* out) {
    Status status=StringBuilder_WriteByte(out,'{');
    usize itemStart=start;
    while (itemStart<end) {
        if (kek_syntax_IsPunctuation(parser->program,parser->fileIndex,itemStart,PunctuationKind_Comma)) {
            itemStart+=1;
            continue;
        }
        usize itemEnd=kek_module_FindTokenAtDepthZero(parser->program,parser->fileIndex,itemStart,end,PunctuationKind_Comma);
        if (itemStart+1<itemEnd&&kek_syntax_IsOperator(parser->program,parser->fileIndex,itemStart+1,OperatorKind_Assign)&&kek_syntax_IsTokenKind(parser->program,parser->fileIndex,itemStart,TokenKind_Identifier)) {
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(out,'.');
            }
            if (status==Status_Ok) {
                status=kek_syntax_WriteToken(parser->program,out,parser->fileIndex,itemStart);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(out,'=');
            }
            struct Expr value={0};
            if (status==Status_Ok) {
                status=kek_codegen_expr_CompileExpressionRange(parser->program,parser->env,parser->fileIndex,itemStart+2,itemEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&value);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&value.text));
            }
            if (status==Status_Ok) {
                kek_codegen_expr_ExprDestroy(&value);
            }
        } else {
            struct Expr value={0};
            if (status==Status_Ok) {
                status=kek_codegen_expr_CompileExpressionRange(parser->program,parser->env,parser->fileIndex,itemStart,itemEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&value);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&value.text));
            }
            if (status==Status_Ok) {
                kek_codegen_expr_ExprDestroy(&value);
            }
        }
        if (status!=Status_Ok) {
            return (status);
        }
        if (itemEnd>=end) {
            break;
        }
        status=StringBuilder_WriteByte(out,',');
        itemStart=itemEnd+1;
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'}');
    }
    return (status);
}
Status kek_codegen_expr_ExprFromLiteralBlock(struct ExprParser* parser,usize blockStart,usize blockEnd,struct Expr* out) {
    Status status=kek_codegen_expr_ExprInitEmpty(parser->program,out);
    if (status!=Status_Ok) {
        return (status);
    }
    struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
    struct String expectedKey=std_string_OwnedStringView(&parser->expectedTypeKey);
    struct String expectedCType=std_string_OwnedStringView(&parser->expectedCType);
    if (expectedCType.len>0&&!parser->expectedIsArray) {
        status=StringBuilder_WriteByte(&builder,'(');
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&builder,expectedCType);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(&builder,')');
        }
    }
    if (status==Status_Ok) {
        status=kek_codegen_expr_WriteInitializerList(parser,blockStart+1,blockEnd,&builder);
    }
    if (status==Status_Ok) {
        status=kek_codegen_expr_ExprFromBuilder(out,&builder);
    }
    StringBuilder_Destroy(&builder);
    if (status==Status_Ok&&expectedKey.len>0) {
        status=kek_codegen_expr_ExprSetType(parser->program,out,expectedKey,expectedCType,0);
    }
    return (status);
}
Status kek_codegen_expr_CompilePrimary(struct ExprParser* parser,struct Expr* out) {
    Status status=kek_codegen_expr_ExprInitEmpty(parser->program,out);
    if (status!=Status_Ok) {
        return (status);
    }
    if (parser->pos>=parser->end) {
        return (Status_Ok);
    }
    usize index=parser->pos;
    struct Token token=parser->program[0].tokenFiles.data[parser->fileIndex].tokens[index];
    if (token.kind==TokenKind_Number) {
        struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
        status=kek_codegen_expr_WriteNumberToken(parser->program,&builder,parser->fileIndex,index);
        if (status==Status_Ok) {
            status=kek_codegen_expr_ExprFromBuilder(out,&builder);
        }
        StringBuilder_Destroy(&builder);
        if (status==Status_Ok) {
            status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_StringFromCString("int"),std_string_StringFromCString("int"),0);
        }
        parser->pos+=1;
        return (status);
    }
    if (token.kind==TokenKind_String||token.kind==TokenKind_Char) {
        if (token.kind==TokenKind_String) {
            status=kek_codegen_expr_ExprSetStringLiteralToken(parser->program,out,parser->fileIndex,index);
        } else {
            status=kek_codegen_expr_ExprSetText(parser->program,out,kek_syntax_TokenText(parser->program,parser->fileIndex,index));
        }
        if (status==Status_Ok) {
            if (token.kind==TokenKind_String) {
                status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_StringFromCString("str"),std_string_StringFromCString("str"),1);
            } else {
                status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_StringFromCString("int"),std_string_StringFromCString("int"),0);
            }
        }
        parser->pos+=1;
        return (status);
    }
    if (kek_syntax_IsKeyword(parser->program,parser->fileIndex,index,KeywordKind_True)||kek_syntax_IsKeyword(parser->program,parser->fileIndex,index,KeywordKind_False)) {
        if (kek_syntax_IsKeyword(parser->program,parser->fileIndex,index,KeywordKind_True)) {
            status=kek_codegen_expr_ExprSetCString(parser->program,out,"1");
        } else {
            status=kek_codegen_expr_ExprSetCString(parser->program,out,"0");
        }
        if (status==Status_Ok) {
            status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_StringFromCString("bool"),std_string_StringFromCString("bool"),0);
        }
        parser->pos+=1;
        return (status);
    }
    if (kek_syntax_IsPunctuation(parser->program,parser->fileIndex,index,PunctuationKind_LeftParen)) {
        usize match=kek_module_FindMatching(parser->program,parser->fileIndex,index);
        struct Expr inner={0};
        status=kek_codegen_expr_CompileExpressionRange(parser->program,parser->env,parser->fileIndex,index+1,match,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&inner);
        struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(&builder,'(');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&inner.text));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(&builder,')');
        }
        if (status==Status_Ok) {
            status=kek_codegen_expr_ExprFromBuilder(out,&builder);
        }
        StringBuilder_Destroy(&builder);
        if (status==Status_Ok) {
            status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_OwnedStringView(&inner.typeKey),std_string_OwnedStringView(&inner.cType),inner.isPointer);
            out->isArray=inner.isArray;
            out->isLvalue=inner.isLvalue;
            out->isPointer=inner.isPointer;
        }
        kek_codegen_expr_ExprDestroy(&inner);
        parser->pos=match+1;
        return (status);
    }
    if (kek_syntax_IsPunctuation(parser->program,parser->fileIndex,index,PunctuationKind_LeftBrace)) {
        usize match=kek_module_FindMatching(parser->program,parser->fileIndex,index);
        status=kek_codegen_expr_ExprFromLiteralBlock(parser,index,match,out);
        parser->pos=match+1;
        return (status);
    }
    if (kek_syntax_IsOperator(parser->program,parser->fileIndex,index,OperatorKind_Scope)) {
        if (index+1<parser->end) {
            status=kek_codegen_expr_ExprSetText(parser->program,out,kek_syntax_TokenText(parser->program,parser->fileIndex,index+1));
            parser->pos=index+2;
            return (status);
        }
    }
    if (token.kind==TokenKind_Identifier||token.kind==TokenKind_Keyword) {
        if (index+2<parser->end&&kek_syntax_IsPunctuation(parser->program,parser->fileIndex,index+1,PunctuationKind_Colon)&&kek_syntax_IsPunctuation(parser->program,parser->fileIndex,index+2,PunctuationKind_LeftBrace)) {
            usize blockEnd=kek_module_FindMatching(parser->program,parser->fileIndex,index+2);
            struct TypeInfo typeInfo={0};
            status=kek_codegen_expr_RenderEnvTypeInfo(parser->program,parser->env,parser->fileIndex,index,index+1,&typeInfo);
            struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(&builder,'(');
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&typeInfo.cType));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(&builder,')');
            }
            if (status==Status_Ok) {
                status=kek_codegen_expr_WriteInitializerList(parser,index+3,blockEnd,&builder);
            }
            if (status==Status_Ok) {
                status=kek_codegen_expr_ExprFromBuilder(out,&builder);
            }
            StringBuilder_Destroy(&builder);
            if (status==Status_Ok) {
                status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_OwnedStringView(&typeInfo.key),std_string_OwnedStringView(&typeInfo.cType),typeInfo.isPointer);
            }
            kek_sema_TypeInfoDestroy(&typeInfo);
            parser->pos=blockEnd+1;
            return (status);
        }
        if (kek_syntax_IsIdentifierText(parser->program,parser->fileIndex,index,"cast")&&index+1<parser->end&&kek_syntax_IsOperator(parser->program,parser->fileIndex,index+1,OperatorKind_Less)) {
            usize genericEnd=kek_module_FindMatching(parser->program,parser->fileIndex,index+1);
            usize groupStart=genericEnd+1;
            usize groupEnd=kek_module_FindMatching(parser->program,parser->fileIndex,groupStart);
            struct TypeInfo typeInfo={0};
            status=kek_codegen_expr_RenderEnvTypeInfo(parser->program,parser->env,parser->fileIndex,index+2,genericEnd,&typeInfo);
            struct Expr value={0};
            if (status==Status_Ok) {
                status=kek_codegen_expr_CompileExpressionRange(parser->program,parser->env,parser->fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&value);
            }
            struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(&builder,"((");
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&typeInfo.cType));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(&builder,")(");
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&value.text));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(&builder,"))");
            }
            if (status==Status_Ok) {
                status=kek_codegen_expr_ExprFromBuilder(out,&builder);
            }
            StringBuilder_Destroy(&builder);
            if (status==Status_Ok) {
                status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_OwnedStringView(&typeInfo.key),std_string_OwnedStringView(&typeInfo.cType),typeInfo.isPointer);
            }
            kek_codegen_expr_ExprDestroy(&value);
            kek_sema_TypeInfoDestroy(&typeInfo);
            parser->pos=groupEnd+1;
            return (status);
        }
        if ((kek_syntax_IsIdentifierText(parser->program,parser->fileIndex,index,"sizeof")||kek_syntax_IsIdentifierText(parser->program,parser->fileIndex,index,"alignof"))&&index+1<parser->end&&kek_syntax_IsPunctuation(parser->program,parser->fileIndex,index+1,PunctuationKind_LeftParen)) {
            usize groupEnd=kek_module_FindMatching(parser->program,parser->fileIndex,index+1);
            struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
            bool isAlign=kek_syntax_IsIdentifierText(parser->program,parser->fileIndex,index,"alignof");
            if (isAlign) {
                status=StringBuilder_WriteCString(&builder,"_Alignof(");
            } else {
                status=StringBuilder_WriteCString(&builder,"sizeof(");
            }
            struct TypeInfo typeInfo={0};
            if (status==Status_Ok) {
                status=kek_codegen_expr_RenderEnvTypeInfo(parser->program,parser->env,parser->fileIndex,index+2,groupEnd,&typeInfo);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&typeInfo.cType));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(&builder,')');
            }
            if (status==Status_Ok) {
                status=kek_codegen_expr_ExprFromBuilder(out,&builder);
            }
            StringBuilder_Destroy(&builder);
            kek_sema_TypeInfoDestroy(&typeInfo);
            if (status==Status_Ok) {
                status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_StringFromCString("usize"),std_string_StringFromCString("usize"),0);
            }
            parser->pos=groupEnd+1;
            return (status);
        }
        if (kek_syntax_IsIdentifierText(parser->program,parser->fileIndex,index,"offsetof")&&index+1<parser->end&&kek_syntax_IsPunctuation(parser->program,parser->fileIndex,index+1,PunctuationKind_LeftParen)) {
            usize groupEnd=kek_module_FindMatching(parser->program,parser->fileIndex,index+1);
            usize comma=kek_module_FindTokenAtDepthZero(parser->program,parser->fileIndex,index+2,groupEnd,PunctuationKind_Comma);
            struct TypeInfo typeInfo={0};
            status=kek_codegen_expr_RenderEnvTypeInfo(parser->program,parser->env,parser->fileIndex,index+2,comma,&typeInfo);
            struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(&builder,"offsetof(");
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&typeInfo.cType));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(&builder,',');
            }
            if (status==Status_Ok) {
                status=kek_sema_WriteTokenRangeRaw(parser->program,&builder,parser->fileIndex,comma+1,groupEnd);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(&builder,')');
            }
            if (status==Status_Ok) {
                status=kek_codegen_expr_ExprFromBuilder(out,&builder);
            }
            StringBuilder_Destroy(&builder);
            kek_sema_TypeInfoDestroy(&typeInfo);
            if (status==Status_Ok) {
                status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_StringFromCString("usize"),std_string_StringFromCString("usize"),0);
            }
            parser->pos=groupEnd+1;
            return (status);
        }
        struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
        if (index+2<parser->end&&kek_syntax_IsOperator(parser->program,parser->fileIndex,index+1,OperatorKind_Scope)) {
            struct String scopeName=kek_syntax_TokenText(parser->program,parser->fileIndex,index);
            struct String name=kek_syntax_TokenText(parser->program,parser->fileIndex,index+2);
            struct ModuleDecl* scopedDecl=kek_sema_FindFunctionDeclByName(parser->program,name,scopeName);
            if (scopedDecl!=0) {
                status=kek_sema_WriteDeclCName(parser->program,&builder,scopedDecl);
            } else {
                status=kek_syntax_WriteToken(parser->program,&builder,parser->fileIndex,index);
                if (status==Status_Ok) {
                    status=StringBuilder_WriteByte(&builder,'_');
                }
                if (status==Status_Ok) {
                    status=kek_syntax_WriteToken(parser->program,&builder,parser->fileIndex,index+2);
                }
            }
            parser->pos=index+3;
            if (parser->pos<parser->end&&kek_syntax_IsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_Less)) {
                usize gEnd=kek_module_FindMatching(parser->program,parser->fileIndex,parser->pos);
                if (gEnd+1<parser->end&&kek_syntax_IsPunctuation(parser->program,parser->fileIndex,gEnd+1,PunctuationKind_LeftParen)) {
                    usize argStart=parser->pos+1;
                    while (argStart<gEnd) {
                        usize argEnd=kek_module_FindTokenAtDepthZero(parser->program,parser->fileIndex,argStart,gEnd,PunctuationKind_Comma);
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteCString(&builder,"__");
                        }
                        if (status==Status_Ok) {
                            status=kek_sema_WriteTypeSuffixFromRange(parser->program,&builder,parser->fileIndex,argStart,argEnd);
                        }
                        if (argEnd>=gEnd) {
                            break;
                        }
                        argStart=argEnd+1;
                    }
                    parser->pos=gEnd+1;
                }
            }
        } else {
            status=kek_syntax_WriteToken(parser->program,&builder,parser->fileIndex,index);
            parser->pos=index+1;
            if (parser->pos<parser->end&&kek_syntax_IsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_Less)) {
                usize gEnd=kek_module_FindMatching(parser->program,parser->fileIndex,parser->pos);
                if (gEnd+1<parser->end&&kek_syntax_IsPunctuation(parser->program,parser->fileIndex,gEnd+1,PunctuationKind_LeftParen)) {
                    usize argStart=parser->pos+1;
                    while (argStart<gEnd) {
                        usize argEnd=kek_module_FindTokenAtDepthZero(parser->program,parser->fileIndex,argStart,gEnd,PunctuationKind_Comma);
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteCString(&builder,"__");
                        }
                        if (status==Status_Ok) {
                            status=kek_sema_WriteTypeSuffixFromRange(parser->program,&builder,parser->fileIndex,argStart,argEnd);
                        }
                        if (argEnd>=gEnd) {
                            break;
                        }
                        argStart=argEnd+1;
                    }
                    parser->pos=gEnd+1;
                }
            }
        }
        if (status==Status_Ok) {
            status=kek_codegen_expr_ExprFromBuilder(out,&builder);
        }
        StringBuilder_Destroy(&builder);
        struct Local* local=kek_codegen_state_EnvFind(parser->env,kek_syntax_TokenText(parser->program,parser->fileIndex,index));
        if (status==Status_Ok&&local!=0) {
            status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_OwnedStringView(&local->typeKey),std_string_OwnedStringView(&local->cType),local->isPointer);
            out->isArray=local->isArray;
            out->isPointer=local->isPointer;
            out->isLvalue=1;
            struct OwnedString arrLen={0};
            if (status==Status_Ok) {
                status=kek_syntax_CloneContextString(parser->program,std_string_OwnedStringView(&local->arrayLen),&arrLen);
            }
            if (status==Status_Ok) {
                status=kek_sema_ReplaceOwned(&out->arrayLen,arrLen);
            }
        } else {
            if (status==Status_Ok) {
                struct ModuleDecl* functionDecl=kek_sema_FindFunctionDeclByName(parser->program,kek_syntax_TokenText(parser->program,parser->fileIndex,index),std_string_StringFromCString(""));
                if (functionDecl!=0) {
                    struct StringBuilder functionName=std_string_StringBuilderNew(parser->program[0].allocator);
                    status=kek_sema_WriteDeclCName(parser->program,&functionName,functionDecl);
                    struct String currentName=std_string_OwnedStringView(&out->text);
                    struct Result__usize suffixStart=String_FindByte(&currentName,'_');
                    if (status==Status_Ok&&suffixStart.status==Status_Ok&&suffixStart.value+1<currentName.len&&currentName.data[suffixStart.value+1]=='_') {
                        status=StringBuilder_WriteString(&functionName,String_Slice(&currentName,suffixStart.value,currentName.len-suffixStart.value));
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_expr_ExprFromBuilder(out,&functionName);
                    }
                    StringBuilder_Destroy(&functionName);
                }
            }
        }
        return (status);
    }
    parser->pos+=1;
    return (Status_Ok);
}
Status kek_codegen_expr_WriteCallArgs(struct ExprParser* parser,usize start,usize end,struct StringBuilder* out) {
    usize itemStart=start;
    bool first=1;
    Status status=Status_Ok;
    while (itemStart<end) {
        if (kek_syntax_IsPunctuation(parser->program,parser->fileIndex,itemStart,PunctuationKind_Comma)) {
            itemStart+=1;
            continue;
        }
        usize itemEnd=kek_module_FindTokenAtDepthZero(parser->program,parser->fileIndex,itemStart,end,PunctuationKind_Comma);
        usize exprStart=itemStart;
        usize assign=kek_module_FindTokenAtDepthZero(parser->program,parser->fileIndex,itemStart,itemEnd,PunctuationKind_Colon);
        if (assign<itemEnd) {
            exprStart=assign+1;
        } else {
            for (usize i=itemStart;i<itemEnd;i++) {
                if (kek_syntax_IsOperator(parser->program,parser->fileIndex,i,OperatorKind_Assign)) {
                    exprStart=i+1;
                    break;
                }
            }
        }
        struct Expr arg={0};
        if (status==Status_Ok) {
            status=kek_codegen_expr_CompileExpressionRange(parser->program,parser->env,parser->fileIndex,exprStart,itemEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&arg);
        }
        if (status==Status_Ok&&!first) {
            status=StringBuilder_WriteByte(out,',');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&arg.text));
        }
        if (status==Status_Ok) {
            kek_codegen_expr_ExprDestroy(&arg);
        }
        first=0;
        if (status!=Status_Ok||itemEnd>=end) {
            break;
        }
        itemStart=itemEnd+1;
    }
    return (status);
}
usize kek_codegen_expr_CountCallArgs(struct CompilerContext* program,usize fileIndex,usize start,usize end) {
    if (start>=end) {
        return (0);
    }
    usize count=1;
    usize i=start;
    while (i<end) {
        usize comma=kek_module_FindTokenAtDepthZero(program,fileIndex,i,end,PunctuationKind_Comma);
        if (comma>=end) {
            break;
        }
        count+=1;
        i=comma+1;
    }
    return (count);
}
Status kek_codegen_expr_ApplyPostfix(struct ExprParser* parser,struct Expr* expr) {
    while (parser->pos<parser->end) {
        if (kek_syntax_IsPunctuation(parser->program,parser->fileIndex,parser->pos,PunctuationKind_LeftParen)) {
            usize groupStart=parser->pos;
            usize groupEnd=kek_module_FindMatching(parser->program,parser->fileIndex,groupStart);
            struct String funcText=std_string_OwnedStringView(&expr->text);
            if (String_EqualsCString(&funcText,"len")) {
                struct Expr arg={0};
                Status status=kek_codegen_expr_CompileExpressionRange(parser->program,parser->env,parser->fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&arg);
                struct StringBuilder lenBuilder=std_string_StringBuilderNew(parser->program[0].allocator);
                if (status==Status_Ok) {
                    if (arg.isArray) {
                        status=StringBuilder_WriteCString(&lenBuilder,"((void)(");
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteString(&lenBuilder,std_string_OwnedStringView(&arg.text));
                        }
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteCString(&lenBuilder,"),");
                        }
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteString(&lenBuilder,std_string_OwnedStringView(&arg.arrayLen));
                        }
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteByte(&lenBuilder,')');
                        }
                    } else {
                        status=StringBuilder_WriteCString(&lenBuilder,"(sizeof(");
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteString(&lenBuilder,std_string_OwnedStringView(&arg.text));
                        }
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteCString(&lenBuilder,")/sizeof((");
                        }
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteString(&lenBuilder,std_string_OwnedStringView(&arg.text));
                        }
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteCString(&lenBuilder,")[0]))");
                        }
                    }
                }
                if (status==Status_Ok) {
                    status=kek_codegen_expr_ExprFromBuilder(expr,&lenBuilder);
                }
                StringBuilder_Destroy(&lenBuilder);
                kek_codegen_expr_ExprDestroy(&arg);
                if (status==Status_Ok) {
                    status=kek_codegen_expr_ExprSetType(parser->program,expr,std_string_StringFromCString("usize"),std_string_StringFromCString("usize"),0);
                }
                parser->pos=groupEnd+1;
                if (status!=Status_Ok) {
                    return (status);
                }
                continue;
            }
            struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
            Status status=StringBuilder_WriteString(&builder,funcText);
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(&builder,'(');
            }
            if (status==Status_Ok) {
                status=kek_codegen_expr_WriteCallArgs(parser,groupStart+1,groupEnd,&builder);
            }
            usize argCount=kek_codegen_expr_CountCallArgs(parser->program,parser->fileIndex,groupStart+1,groupEnd);
            if (status==Status_Ok&&String_EqualsCString(&funcText,"add")&&argCount==1) {
                status=StringBuilder_WriteCString(&builder,",0");
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(&builder,')');
            }
            if (status==Status_Ok) {
                status=kek_codegen_expr_ExprFromBuilder(expr,&builder);
            }
            StringBuilder_Destroy(&builder);
            expr->isLvalue=0;
            parser->pos=groupEnd+1;
            if (status!=Status_Ok) {
                return (status);
            }
            continue;
        }
        if (kek_syntax_IsPunctuation(parser->program,parser->fileIndex,parser->pos,PunctuationKind_LeftBracket)) {
            usize indexStart=parser->pos;
            usize indexEnd=kek_module_FindMatching(parser->program,parser->fileIndex,indexStart);
            struct Expr indexExpr={0};
            Status status=kek_codegen_expr_CompileExpressionRange(parser->program,parser->env,parser->fileIndex,indexStart+1,indexEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&indexExpr);
            struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&expr->text));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(&builder,'[');
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&indexExpr.text));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(&builder,']');
            }
            if (status==Status_Ok) {
                status=kek_codegen_expr_ExprFromBuilder(expr,&builder);
            }
            StringBuilder_Destroy(&builder);
            kek_codegen_expr_ExprDestroy(&indexExpr);
            expr->isLvalue=1;
            expr->isArray=0;
            struct String indexedKey=std_string_OwnedStringView(&expr->typeKey);
            if (indexedKey.len>0&&indexedKey.data[indexedKey.len-1]=='*') {
                struct OwnedString newKey={0};
                status=kek_syntax_CloneContextString(parser->program,String_Slice(&indexedKey,0,indexedKey.len-1),&newKey);
                if (status==Status_Ok) {
                    status=kek_sema_ReplaceOwned(&expr->typeKey,newKey);
                }
                struct OwnedString newCType={0};
                if (status==Status_Ok) {
                    status=kek_sema_MakeCTypeFromKey(parser->program,std_string_OwnedStringView(&expr->typeKey),&newCType);
                }
                if (status==Status_Ok) {
                    status=kek_sema_ReplaceOwned(&expr->cType,newCType);
                }
                expr->isPointer=0;
            }
            parser->pos=indexEnd+1;
            if (status!=Status_Ok) {
                return (status);
            }
            continue;
        }
        if (kek_syntax_IsPunctuation(parser->program,parser->fileIndex,parser->pos,PunctuationKind_Dot)) {
            usize nameIndex=parser->pos+1;
            struct String fieldName=kek_syntax_TokenText(parser->program,parser->fileIndex,nameIndex);
            if (nameIndex+1<parser->end&&kek_syntax_IsPunctuation(parser->program,parser->fileIndex,nameIndex+1,PunctuationKind_LeftParen)) {
                usize groupStart=nameIndex+1;
                usize groupEnd=kek_module_FindMatching(parser->program,parser->fileIndex,groupStart);
                struct String receiverKey=std_string_OwnedStringView(&expr->typeKey);
                struct String methodReceiverKey=receiverKey;
                if (expr->isPointer&&receiverKey.len>0&&receiverKey.data[receiverKey.len-1]=='*') {
                    methodReceiverKey=String_Slice(&receiverKey,0,receiverKey.len-1);
                }
                struct ModuleDecl* method=kek_codegen_expr_FindMethod(parser->program,methodReceiverKey,fieldName,0,0,0);
                struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
                Status status=Status_Ok;
                if (method!=0) {
                    status=kek_sema_WriteDeclCName(parser->program,&builder,method);
                } else {
                    status=StringBuilder_WriteString(&builder,methodReceiverKey);
                    if (status==Status_Ok) {
                        status=StringBuilder_WriteByte(&builder,'_');
                    }
                    if (status==Status_Ok) {
                        status=StringBuilder_WriteString(&builder,fieldName);
                    }
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteByte(&builder,'(');
                }
                if (status==Status_Ok&&!expr->isPointer) {
                    status=StringBuilder_WriteByte(&builder,'&');
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&expr->text));
                }
                if (groupStart+1<groupEnd) {
                    if (status==Status_Ok) {
                        status=StringBuilder_WriteByte(&builder,',');
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_expr_WriteCallArgs(parser,groupStart+1,groupEnd,&builder);
                    }
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteByte(&builder,')');
                }
                if (status==Status_Ok) {
                    status=kek_codegen_expr_ExprFromBuilder(expr,&builder);
                }
                StringBuilder_Destroy(&builder);
                expr->isLvalue=0;
                if (method!=0) {
                    struct TypeInfo returnInfo={0};
                    if (status==Status_Ok) {
                        status=kek_sema_RenderTypeInfo(parser->program,method->fileIndex,method->returnStart,method->returnEnd,&returnInfo);
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_expr_ExprSetType(parser->program,expr,std_string_OwnedStringView(&returnInfo.key),std_string_OwnedStringView(&returnInfo.cType),returnInfo.isPointer);
                    }
                    if (status==Status_Ok) {
                        kek_sema_TypeInfoDestroy(&returnInfo);
                    }
                }
                parser->pos=groupEnd+1;
                if (status!=Status_Ok) {
                    return (status);
                }
                continue;
            }
            struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
            Status status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&expr->text));
            if (status==Status_Ok) {
                struct String exprText=std_string_OwnedStringView(&expr->text);
                if (expr->isPointer||String_EqualsCString(&exprText,"this")) {
                    status=StringBuilder_WriteCString(&builder,"->");
                } else {
                    status=StringBuilder_WriteByte(&builder,'.');
                }
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(&builder,fieldName);
            }
            if (status==Status_Ok) {
                status=kek_codegen_expr_ExprFromBuilder(expr,&builder);
            }
            StringBuilder_Destroy(&builder);
            struct TypeInfo fieldType={0};
            if (status==Status_Ok) {
                status=kek_codegen_expr_ModuleFieldType(parser->program,std_string_OwnedStringView(&expr->typeKey),fieldName,&fieldType);
            }
            if (status==Status_Ok) {
                status=kek_codegen_expr_ExprSetType(parser->program,expr,std_string_OwnedStringView(&fieldType.key),std_string_OwnedStringView(&fieldType.cType),fieldType.isPointer);
            }
            if (status==Status_Ok) {
                kek_sema_TypeInfoDestroy(&fieldType);
            }
            expr->isLvalue=1;
            parser->pos=nameIndex+1;
            if (status!=Status_Ok) {
                return (status);
            }
            continue;
        }
        break;
    }
    return (Status_Ok);
}
Status kek_codegen_expr_CompileUnary(struct ExprParser* parser,struct Expr* out) {
    if (parser->pos<parser->end&&(kek_syntax_IsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_Minus)||kek_syntax_IsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_LogicalNot)||kek_syntax_IsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_BitwiseNot)||kek_syntax_IsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_BitwiseAnd)||kek_syntax_IsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_Multiply))) {
        usize op=parser->pos;
        parser->pos+=1;
        struct Expr inner={0};
        Status status=kek_codegen_expr_CompileUnary(parser,&inner);
        struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(parser->program,&builder,parser->fileIndex,op);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&inner.text));
        }
        if (status==Status_Ok) {
            status=kek_codegen_expr_ExprInitEmpty(parser->program,out);
        }
        if (status==Status_Ok) {
            status=kek_codegen_expr_ExprFromBuilder(out,&builder);
        }
        StringBuilder_Destroy(&builder);
        if (status==Status_Ok) {
            struct ModuleDecl* unaryMethod=0;
            if (kek_syntax_IsOperator(parser->program,parser->fileIndex,op,OperatorKind_Minus)) {
                unaryMethod=kek_codegen_expr_FindMethod(parser->program,std_string_OwnedStringView(&inner.typeKey),std_string_StringFromCString(""),1,kek_module_OperatorCode(parser->program,parser->fileIndex,op),0);
            }
            if (unaryMethod!=0) {
                struct StringBuilder call=std_string_StringBuilderNew(parser->program[0].allocator);
                status=kek_sema_WriteDeclCName(parser->program,&call,unaryMethod);
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(&call,"(&");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteString(&call,std_string_OwnedStringView(&inner.text));
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteByte(&call,')');
                }
                if (status==Status_Ok) {
                    status=kek_codegen_expr_ExprFromBuilder(out,&call);
                }
                StringBuilder_Destroy(&call);
            } else {
                if (kek_syntax_IsOperator(parser->program,parser->fileIndex,op,OperatorKind_BitwiseAnd)) {
                    struct StringBuilder key=std_string_StringBuilderNew(parser->program[0].allocator);
                    status=StringBuilder_WriteString(&key,std_string_OwnedStringView(&inner.typeKey));
                    if (status==Status_Ok) {
                        status=StringBuilder_WriteByte(&key,'*');
                    }
                    struct OwnedString keyOwned={0};
                    if (status==Status_Ok) {
                        status=kek_syntax_DetachBuilder(&key,&keyOwned);
                    }
                    StringBuilder_Destroy(&key);
                    struct OwnedString cOwned={0};
                    if (status==Status_Ok) {
                        struct StringBuilder ct=std_string_StringBuilderNew(parser->program[0].allocator);
                        status=StringBuilder_WriteString(&ct,std_string_OwnedStringView(&inner.cType));
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteByte(&ct,'*');
                        }
                        if (status==Status_Ok) {
                            status=kek_syntax_DetachBuilder(&ct,&cOwned);
                        }
                        StringBuilder_Destroy(&ct);
                    }
                    if (status==Status_Ok) {
                        status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_OwnedStringView(&keyOwned),std_string_OwnedStringView(&cOwned),1);
                        std_string_DestroyOwnedString(&keyOwned);
                        std_string_DestroyOwnedString(&cOwned);
                    }
                } else {
                    status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_OwnedStringView(&inner.typeKey),std_string_OwnedStringView(&inner.cType),inner.isPointer);
                }
            }
        }
        kek_codegen_expr_ExprDestroy(&inner);
        return (status);
    }
    Status status=kek_codegen_expr_CompilePrimary(parser,out);
    if (status==Status_Ok) {
        status=kek_codegen_expr_ApplyPostfix(parser,out);
    }
    return (status);
}
Status kek_codegen_expr_CompileBinaryOperation(struct ExprParser* parser,struct Expr* left,usize operatorIndex,struct Expr* right,struct Expr* out) {
    Status status=kek_codegen_expr_ExprInitEmpty(parser->program,out);
    if (status!=Status_Ok) {
        return (status);
    }
    u8 opCode=kek_module_OperatorCode(parser->program,parser->fileIndex,operatorIndex);
    struct ModuleDecl* method=0;
    usize operatorArgCount=1;
    if (kek_syntax_IsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_PlusAssign)||kek_syntax_IsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_MinusAssign)) {
        operatorArgCount=1;
    }
    if (opCode!=0&&std_string_OwnedStringView(&left->typeKey).len>0) {
        method=kek_codegen_expr_FindMethod(parser->program,std_string_OwnedStringView(&left->typeKey),std_string_StringFromCString(""),1,opCode,operatorArgCount);
    }
    if (method!=0) {
        struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
        status=kek_sema_WriteDeclCName(parser->program,&builder,method);
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(&builder,"(&");
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&left->text));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(&builder,',');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&right->text));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(&builder,')');
        }
        if (status==Status_Ok) {
            status=kek_codegen_expr_ExprFromBuilder(out,&builder);
        }
        StringBuilder_Destroy(&builder);
        if (status==Status_Ok) {
            struct TypeInfo returnInfo={0};
            status=kek_sema_RenderTypeInfo(parser->program,method->fileIndex,method->returnStart,method->returnEnd,&returnInfo);
            if (status==Status_Ok) {
                status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_OwnedStringView(&returnInfo.key),std_string_OwnedStringView(&returnInfo.cType),returnInfo.isPointer);
            }
            if (status==Status_Ok) {
                kek_sema_TypeInfoDestroy(&returnInfo);
            }
        }
        return (status);
    }
    struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
    status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&left->text));
    if (status==Status_Ok) {
        status=kek_syntax_WriteToken(parser->program,&builder,parser->fileIndex,operatorIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&right->text));
    }
    if (status==Status_Ok) {
        status=kek_codegen_expr_ExprFromBuilder(out,&builder);
    }
    StringBuilder_Destroy(&builder);
    if (status==Status_Ok) {
        if (kek_syntax_IsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_Equal)||kek_syntax_IsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_NotEqual)||kek_syntax_IsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_Less)||kek_syntax_IsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_LessEqual)||kek_syntax_IsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_Greater)||kek_syntax_IsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_GreaterEqual)||kek_syntax_IsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_LogicalAnd)||kek_syntax_IsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_LogicalOr)) {
            status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_StringFromCString("bool"),std_string_StringFromCString("bool"),0);
        } else {
            status=kek_codegen_expr_ExprSetType(parser->program,out,std_string_OwnedStringView(&left->typeKey),std_string_OwnedStringView(&left->cType),left->isPointer);
        }
    }
    return (status);
}
Status kek_codegen_expr_CompileExpression(struct ExprParser* parser,u8 minPrecedence,struct Expr* out) {
    struct Expr left={0};
    Status status=kek_codegen_expr_CompileUnary(parser,&left);
    if (status!=Status_Ok) {
        return (status);
    }
    while (parser->pos<parser->end) {
        u8 precedence=kek_codegen_expr_OperatorPrecedence(parser->program,parser->fileIndex,parser->pos);
        if (precedence==0||precedence<minPrecedence) {
            break;
        }
        usize op=parser->pos;
        parser->pos+=1;
        u8 nextMin=precedence+1;
        if (kek_codegen_expr_OperatorRightAssociative(parser->program,parser->fileIndex,op)) {
            nextMin=precedence;
        }
        struct Expr right={0};
        status=kek_codegen_expr_CompileExpression(parser,nextMin,&right);
        if (status!=Status_Ok) {
            kek_codegen_expr_ExprDestroy(&left);
            return (status);
        }
        struct Expr combined={0};
        status=kek_codegen_expr_CompileBinaryOperation(parser,&left,op,&right,&combined);
        kek_codegen_expr_ExprDestroy(&left);
        kek_codegen_expr_ExprDestroy(&right);
        if (status!=Status_Ok) {
            return (status);
        }
        left=combined;
    }
    out[0]=left;
    return (Status_Ok);
}
Status kek_codegen_types_FuncUseTempTypeUse(struct FuncUse* funcUse,struct TypeUse* out) {
    out->key=funcUse->key;
    out->cName=funcUse->cName;
    out->baseName=funcUse->key;
    out->args=funcUse->args;
    out->emitted=0;
    return (Status_Ok);
}
Status kek_codegen_types_WriteDeclarator(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize typeStart,usize typeEnd,usize nameIndex,bool isArray,usize arrayStart,usize arrayEnd) {
    struct TypeInfo info={0};
    Status status=kek_sema_RenderTypeInfo(program,fileIndex,typeStart,typeEnd,&info);
    if (status!=Status_Ok) {
        return (status);
    }
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    CodeWriter_String(&w,std_string_OwnedStringView(&info.cType));
    CodeWriter_Byte(&w,' ');
    CodeWriter_Token(&w,program,fileIndex,nameIndex);
    if (isArray) {
        CodeWriter_Byte(&w,'[');
        CodeWriter_TokenRangeRaw(&w,program,fileIndex,arrayStart,arrayEnd);
        CodeWriter_Byte(&w,']');
    }
    kek_sema_TypeInfoDestroy(&info);
    return (w.status);
}
Status kek_codegen_types_WriteFields(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl) {
    for (usize i=0;i<decl->fieldCount;i++) {
        struct ModuleField* field=&program->fields.data[decl->firstField+i];
        struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
        CodeWriter_CString(&w,"    ");
        if (w.status!=Status_Ok) {
            return (w.status);
        }
        if (field->isNestedStruct) {
            CodeWriter_CString(&w,"struct {\n");
            if (w.status!=Status_Ok) {
                return (w.status);
            }
            struct ModuleDecl nested={0};
            kek_module_InitNestedStructDecl(&nested,field,field->nestedFirstField,decl);
            Status status=kek_codegen_types_WriteFields(program,out,&nested);
            if (status!=Status_Ok) {
                return (status);
            }
            CodeWriter_CString(&w,"    } ");
            CodeWriter_Token(&w,program,field->fileIndex,field->nameIndex);
            CodeWriter_CString(&w,";\n");
            if (w.status!=Status_Ok) {
                return (w.status);
            }
            continue;
        }
        Status status=kek_codegen_types_WriteDeclarator(program,out,field->fileIndex,field->typeStart,field->typeEnd,field->nameIndex,field->isArray,field->arrayStart,field->arrayEnd);
        if (status==Status_Ok) {
            CodeWriter_CString(&w,";\n");
            status=w.status;
        }
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
bool kek_codegen_types_GenericParamEquals(struct CompilerContext* program,struct ModuleDecl* decl,usize paramIndex,struct String name) {
    if (!decl->isGeneric) {
        return (0);
    }
    if (decl->genericStart<kek_syntax_FileTokenCount(program,decl->fileIndex)) {
        usize index=decl->genericStart+1;
        usize current=0;
        while (index<decl->genericEnd) {
            if (kek_syntax_IsPunctuation(program,decl->fileIndex,index,PunctuationKind_Comma)) {
                index+=1;
                continue;
            }
            if (current==paramIndex) {
                struct String paramName=kek_syntax_TokenText(program,decl->fileIndex,index);
                return (String_Equals(&paramName,name));
            }
            current+=1;
            while (index<decl->genericEnd&&!kek_syntax_IsPunctuation(program,decl->fileIndex,index,PunctuationKind_Comma)) {
                index+=1;
            }
        }
    }
    if (decl->hasReceiver&&decl->receiverStart+1<decl->receiverEnd&&kek_syntax_IsOperator(program,decl->fileIndex,decl->receiverStart+1,OperatorKind_Less)) {
        usize genericEnd=kek_module_FindMatching(program,decl->fileIndex,decl->receiverStart+1);
        usize index=decl->receiverStart+2;
        usize current=0;
        while (index<genericEnd) {
            if (kek_syntax_IsPunctuation(program,decl->fileIndex,index,PunctuationKind_Comma)) {
                index+=1;
                continue;
            }
            if (current==paramIndex) {
                struct String paramName=kek_syntax_TokenText(program,decl->fileIndex,index);
                return (String_Equals(&paramName,name));
            }
            current+=1;
            while (index<genericEnd&&!kek_syntax_IsPunctuation(program,decl->fileIndex,index,PunctuationKind_Comma)) {
                index+=1;
            }
        }
    }
    return (0);
}
Status kek_codegen_types_WriteTypeSuffixSubst(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct TypeUse* use,usize fileIndex,usize start,usize end) {
    if (start>=end) {
        return (Status_Ok);
    }
    struct String first=kek_syntax_TokenText(program,fileIndex,start);
    if (end==start+1) {
        for (usize i=0;i<use->args.len;i++) {
            if (kek_codegen_types_GenericParamEquals(program,decl,i,first)) {
                return (StringBuilder_WriteString(out,std_string_OwnedStringView(&use->args.data[i])));
            }
        }
        return (kek_sema_WriteSanitized(out,first));
    }
    if (start+1<end&&kek_syntax_IsOperator(program,fileIndex,start+1,OperatorKind_Less)) {
        Status status=kek_sema_WriteSanitized(out,first);
        usize genericEnd=kek_module_FindMatching(program,fileIndex,start+1);
        usize argStart=start+2;
        while (argStart<genericEnd) {
            usize argEnd=kek_module_FindTokenAtDepthZero(program,fileIndex,argStart,genericEnd,PunctuationKind_Comma);
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"__");
            }
            if (status==Status_Ok) {
                status=kek_codegen_types_WriteTypeSuffixSubst(program,out,decl,use,fileIndex,argStart,argEnd);
            }
            if (status!=Status_Ok||argEnd>=genericEnd) {
                break;
            }
            argStart=argEnd+1;
        }
        return (status);
    }
    return (kek_sema_WriteTypeSuffixFromRange(program,out,fileIndex,start,end));
}
Status kek_codegen_types_MakeSubstTypeKey(struct CompilerContext* program,struct ModuleDecl* decl,struct TypeUse* use,usize fileIndex,usize start,usize end,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
    Status status=kek_codegen_types_WriteTypeSuffixSubst(program,&builder,decl,use,fileIndex,start,end);
    if (status==Status_Ok) {
        status=kek_syntax_DetachBuilder(&builder,out);
    }
    StringBuilder_Destroy(&builder);
    return (status);
}
Status kek_codegen_types_MakeSubstTypeInfo(struct CompilerContext* program,struct ModuleDecl* decl,struct TypeUse* use,usize fileIndex,usize start,usize end,struct TypeInfo* info) {
    Status status=kek_sema_TypeInfoInitEmpty(program,info);
    if (status!=Status_Ok) {
        return (status);
    }
    struct String first=kek_syntax_TokenText(program,fileIndex,start);
    struct OwnedString key={0};
    if (String_EqualsCString(&first,"ptr")&&start+1<end&&kek_syntax_IsOperator(program,fileIndex,start+1,OperatorKind_Less)) {
        usize genericEnd=kek_module_FindMatching(program,fileIndex,start+1);
        struct OwnedString innerKey={0};
        innerKey.data=0;
        status=kek_codegen_types_MakeSubstTypeKey(program,decl,use,fileIndex,start+2,genericEnd,&innerKey);
        struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&innerKey));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(&builder,'*');
        }
        if (status==Status_Ok) {
            status=kek_syntax_DetachBuilder(&builder,&key);
        }
        StringBuilder_Destroy(&builder);
        if (innerKey.data!=0) {
            std_string_DestroyOwnedString(&innerKey);
        }
    } else {
        status=kek_codegen_types_MakeSubstTypeKey(program,decl,use,fileIndex,start,end,&key);
    }
    if (status==Status_Ok) {
        status=kek_sema_ReplaceOwned(&info->key,key);
    }
    struct OwnedString cOwned={0};
    if (status==Status_Ok) {
        status=kek_sema_MakeCTypeFromKey(program,std_string_OwnedStringView(&info->key),&cOwned);
    }
    if (status==Status_Ok) {
        status=kek_sema_ReplaceOwned(&info->cType,cOwned);
    }
    if (status==Status_Ok) {
        struct String keyView=std_string_OwnedStringView(&info->key);
        struct Result__usize ptrMarker=String_FindByte(&keyView,'*');
        info->isPointer=ptrMarker.status==Status_Ok;
    }
    return (status);
}
Status kek_codegen_types_MakeSubstTypeInfoForFunc(struct CompilerContext* program,struct ModuleDecl* decl,struct FuncUse* funcUse,usize fileIndex,usize start,usize end,struct OwnedString* outKey,struct OwnedString* outCType,bool* outIsPointer) {
    struct TypeUse temp={0};
    kek_codegen_types_FuncUseTempTypeUse(funcUse,&temp);
    struct TypeInfo info={0};
    Status status=kek_codegen_types_MakeSubstTypeInfo(program,decl,&temp,fileIndex,start,end,&info);
    if (status==Status_Ok) {
        outKey[0]=info.key;
        outCType[0]=info.cType;
        outIsPointer[0]=info.isPointer;
        info.key.data=0;
        info.cType.data=0;
        kek_sema_TypeInfoDestroy(&info);
    }
    return (status);
}
Status kek_codegen_types_WriteDeclaratorSubst(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct TypeUse* use,usize fileIndex,usize typeStart,usize typeEnd,usize nameIndex,bool isArray,usize arrayStart,usize arrayEnd) {
    struct String first=kek_syntax_TokenText(program,fileIndex,typeStart);
    struct OwnedString key={0};
    Status status=Status_Ok;
    if (String_EqualsCString(&first,"ptr")&&typeStart+1<typeEnd&&kek_syntax_IsOperator(program,fileIndex,typeStart+1,OperatorKind_Less)) {
        usize genericEnd=kek_module_FindMatching(program,fileIndex,typeStart+1);
        struct OwnedString innerKey={0};
        status=kek_codegen_types_MakeSubstTypeKey(program,decl,use,fileIndex,typeStart+2,genericEnd,&innerKey);
        struct StringBuilder ct=std_string_StringBuilderNew(program->allocator);
        if (status==Status_Ok) {
            status=kek_sema_WriteCTypeFromKey(program,&ct,std_string_OwnedStringView(&innerKey));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(&ct,'*');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,StringBuilder_View(&ct));
        }
        StringBuilder_Destroy(&ct);
        std_string_DestroyOwnedString(&innerKey);
    } else {
        status=kek_codegen_types_MakeSubstTypeKey(program,decl,use,fileIndex,typeStart,typeEnd,&key);
        if (status==Status_Ok) {
            status=kek_sema_WriteCTypeFromKey(program,out,std_string_OwnedStringView(&key));
        }
        if (status==Status_Ok) {
            std_string_DestroyOwnedString(&key);
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,' ');
    }
    if (status==Status_Ok) {
        status=kek_syntax_WriteToken(program,out,fileIndex,nameIndex);
    }
    if (status==Status_Ok&&isArray) {
        status=StringBuilder_WriteByte(out,'[');
        if (status==Status_Ok) {
            status=kek_sema_WriteTokenRangeRaw(program,out,fileIndex,arrayStart,arrayEnd);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,']');
        }
    }
    return (status);
}
Status kek_codegen_types_WriteFieldsSubst(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct TypeUse* use) {
    for (usize i=0;i<decl->fieldCount;i++) {
        struct ModuleField* field=&program->fields.data[decl->firstField+i];
        struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
        CodeWriter_CString(&w,"    ");
        Status status=w.status;
        if (field->isNestedStruct) {
            CodeWriter_CString(&w,"struct {\n");
            status=w.status;
            struct ModuleDecl nested={0};
            kek_module_InitNestedStructDecl(&nested,field,field->nestedFirstField,decl);
            if (status==Status_Ok) {
                status=kek_codegen_types_WriteFieldsSubst(program,out,&nested,use);
            }
            w.status=status;
            CodeWriter_CString(&w,"    } ");
            CodeWriter_Token(&w,program,field->fileIndex,field->nameIndex);
            status=w.status;
        } else {
            if (status==Status_Ok) {
                status=kek_codegen_types_WriteDeclaratorSubst(program,out,decl,use,field->fileIndex,field->typeStart,field->typeEnd,field->nameIndex,field->isArray,field->arrayStart,field->arrayEnd);
            }
        }
        w.status=status;
        CodeWriter_CString(&w,";\n");
        status=w.status;
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
Status kek_codegen_types_WriteStructDecl(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct String specializedName) {
    Status status=StringBuilder_WriteCString(out,"struct ");
    bool hasPacked=0;
    bool hasAligned=0;
    usize alignedValue=0;
    if (decl->start>0) {
        usize scan=decl->start;
        while (scan>0) {
            scan-=1;
            if (kek_syntax_IsPunctuation(program,decl->fileIndex,scan,PunctuationKind_LeftBracket)) {
                for (usize i=scan;i<decl->start;i++) {
                    if (kek_syntax_IsIdentifierText(program,decl->fileIndex,i,"packed")) {
                        hasPacked=1;
                    }
                    if (kek_syntax_IsIdentifierText(program,decl->fileIndex,i,"aligned")) {
                        hasAligned=1;
                        if (i+2<decl->start&&kek_syntax_IsPunctuation(program,decl->fileIndex,i+1,PunctuationKind_LeftParen)) {
                            alignedValue=i+2;
                        }
                    }
                }
                break;
            }
            if (kek_syntax_IsPunctuation(program,decl->fileIndex,scan,PunctuationKind_Semicolon)||kek_syntax_IsPunctuation(program,decl->fileIndex,scan,PunctuationKind_RightBrace)) {
                break;
            }
        }
    }
    if (status==Status_Ok&&(hasPacked||hasAligned)) {
        status=StringBuilder_WriteCString(out,"__attribute__((");
        if (status==Status_Ok&&hasPacked) {
            status=StringBuilder_WriteCString(out,"packed");
        }
        if (status==Status_Ok&&hasAligned) {
            if (hasPacked) {
                status=StringBuilder_WriteByte(out,',');
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"aligned(");
            }
            if (status==Status_Ok&&alignedValue!=0) {
                status=kek_syntax_WriteToken(program,out,decl->fileIndex,alignedValue);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(out,')');
            }
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,")) ");
        }
    }
    if (status!=Status_Ok) {
        return (status);
    }
    if (specializedName.len>0) {
        status=StringBuilder_WriteString(out,specializedName);
    } else {
        status=kek_syntax_WriteToken(program,out,decl->fileIndex,decl->nameIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," {\n");
    }
    if (status==Status_Ok) {
        status=kek_codegen_types_WriteFields(program,out,decl);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"};\n");
    }
    return (status);
}
Status kek_codegen_types_WriteUnionDecl(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl) {
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    CodeWriter_CString(&w,"typedef union ");
    CodeWriter_Token(&w,program,decl->fileIndex,decl->nameIndex);
    CodeWriter_CString(&w," {\n");
    Status status=w.status;
    if (status==Status_Ok) {
        status=kek_codegen_types_WriteFields(program,out,decl);
    }
    w.status=status;
    CodeWriter_CString(&w,"} ");
    CodeWriter_Token(&w,program,decl->fileIndex,decl->nameIndex);
    CodeWriter_CString(&w,";\n");
    return (w.status);
}
Status kek_codegen_types_WriteTaggedUnionTagName(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl) {
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    CodeWriter_Token(&w,program,decl->fileIndex,decl->nameIndex);
    CodeWriter_CString(&w,"Tag");
    return (w.status);
}
Status kek_codegen_types_WriteTaggedUnionVariantName(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct ModuleField* field) {
    Status status=kek_codegen_types_WriteTaggedUnionTagName(program,out,decl);
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    w.status=status;
    CodeWriter_Byte(&w,'_');
    CodeWriter_Token(&w,program,field->fileIndex,field->nameIndex);
    return (w.status);
}
Status kek_codegen_types_WriteTaggedUnionConstructorName(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl,struct ModuleField* field) {
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    CodeWriter_Token(&w,program,decl->fileIndex,decl->nameIndex);
    CodeWriter_Byte(&w,'_');
    CodeWriter_Token(&w,program,field->fileIndex,field->nameIndex);
    return (w.status);
}
Status kek_codegen_types_WriteTaggedUnionDecl(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl) {
    Status status=StringBuilder_WriteCString(out,"typedef enum ");
    if (status==Status_Ok) {
        status=kek_codegen_types_WriteTaggedUnionTagName(program,out,decl);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," {\n");
    }
    for (usize i=0;i<decl->fieldCount;i++) {
        struct ModuleField* field=&program->fields.data[decl->firstField+i];
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"    ");
        }
        if (status==Status_Ok) {
            status=kek_codegen_types_WriteTaggedUnionVariantName(program,out,decl,field);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,",\n");
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"} ");
    }
    if (status==Status_Ok) {
        status=kek_codegen_types_WriteTaggedUnionTagName(program,out,decl);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef struct ");
    }
    if (status==Status_Ok) {
        status=kek_syntax_WriteToken(program,out,decl->fileIndex,decl->nameIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," {\n    ");
    }
    if (status==Status_Ok) {
        status=kek_codegen_types_WriteTaggedUnionTagName(program,out,decl);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," tag;\n    union {\n");
    }
    if (status==Status_Ok) {
        status=kek_codegen_types_WriteFields(program,out,decl);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"    } data;\n} ");
    }
    if (status==Status_Ok) {
        status=kek_syntax_WriteToken(program,out,decl->fileIndex,decl->nameIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    for (usize i=0;i<decl->fieldCount;i++) {
        struct ModuleField* field=&program->fields.data[decl->firstField+i];
        struct TypeInfo fieldType={0};
        if (status==Status_Ok) {
            status=kek_sema_RenderTypeInfo(program,field->fileIndex,field->typeStart,field->typeEnd,&fieldType);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"static inline ");
        }
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,decl->fileIndex,decl->nameIndex);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,' ');
        }
        if (status==Status_Ok) {
            status=kek_codegen_types_WriteTaggedUnionConstructorName(program,out,decl,field);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,'(');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&fieldType.cType));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out," value) {\n    ");
        }
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,decl->fileIndex,decl->nameIndex);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out," out;\n    out.tag = ");
        }
        if (status==Status_Ok) {
            status=kek_codegen_types_WriteTaggedUnionVariantName(program,out,decl,field);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,";\n    out.data.");
        }
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,field->fileIndex,field->nameIndex);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out," = value;\n    return out;\n}\n");
        }
        kek_sema_TypeInfoDestroy(&fieldType);
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (status);
}
Status kek_codegen_types_WriteEnumDecl(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl) {
    Status status=StringBuilder_WriteCString(out,"typedef enum ");
    if (status==Status_Ok) {
        status=kek_syntax_WriteToken(program,out,decl->fileIndex,decl->nameIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," {\n");
    }
    usize i=decl->bodyStart;
    while (i<decl->bodyEnd) {
        if (kek_syntax_IsPunctuation(program,decl->fileIndex,i,PunctuationKind_Comma)) {
            i+=1;
            continue;
        }
        if (!kek_syntax_IsTokenKind(program,decl->fileIndex,i,TokenKind_Identifier)) {
            i+=1;
            continue;
        }
        status=StringBuilder_WriteCString(out,"    ");
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,decl->fileIndex,decl->nameIndex);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,'_');
        }
        if (status==Status_Ok) {
            status=kek_syntax_WriteToken(program,out,decl->fileIndex,i);
        }
        i+=1;
        if (i<decl->bodyEnd&&kek_syntax_IsOperator(program,decl->fileIndex,i,OperatorKind_Assign)) {
            status=StringBuilder_WriteByte(out,'=');
            i+=1;
            while (i<decl->bodyEnd&&!kek_syntax_IsPunctuation(program,decl->fileIndex,i,PunctuationKind_Comma)) {
                if (status==Status_Ok) {
                    status=kek_syntax_WriteToken(program,out,decl->fileIndex,i);
                }
                i+=1;
            }
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,",\n");
        }
        if (status!=Status_Ok) {
            return (status);
        }
    }
    status=StringBuilder_WriteCString(out,"} ");
    if (status==Status_Ok) {
        status=kek_syntax_WriteToken(program,out,decl->fileIndex,decl->nameIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    return (status);
}
Status kek_codegen_types_WriteAliasDecl(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl) {
    struct TypeInfo info={0};
    Status status=kek_sema_RenderTypeInfo(program,decl->fileIndex,decl->returnStart,decl->returnEnd,&info);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef ");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&info.cType));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,' ');
    }
    if (status==Status_Ok) {
        status=kek_syntax_WriteToken(program,out,decl->fileIndex,decl->nameIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    kek_sema_TypeInfoDestroy(&info);
    return (status);
}
Status kek_codegen_types_WriteExternDecl(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl) {
    struct SyntaxFile* file=&program->tokenFiles.data[decl->fileIndex];
    struct Token open=file->tokens[decl->bodyStart-1];
    struct Token close=file->tokens[decl->bodyEnd];
    usize start=open.offset+open.length;
    usize end=close.offset;
    if (end<start) {
        return (Status_Ok);
    }
    struct String sourceText=file->sourceText;
    Status status=StringBuilder_WriteCString(out,"#if defined(__GNUC__) || defined(__clang__)\n#pragma GCC diagnostic push\n#pragma GCC diagnostic ignored ");
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,34);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"-Wunused-function");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,34);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"\n#endif\n");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,String_Slice(&sourceText,start,end-start));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"\n#if defined(__GNUC__) || defined(__clang__)\n#pragma GCC diagnostic pop\n#endif");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'\n');
    }
    return (status);
}
Status kek_codegen_types_WriteTypeUseDeclaration(struct CompilerContext* program,struct StringBuilder* out,struct TypeUse* use) {
    if (use->emitted) {
        return (Status_Ok);
    }
    struct ModuleDecl* decl=kek_module_FindTypeDecl(program,std_string_OwnedStringView(&use->baseName));
    if (decl==0||decl->kind!=DeclKind_Struct) {
        return (Status_Ok);
    }
    Status status=StringBuilder_WriteCString(out,"struct ");
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->key));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," {\n");
    }
    if (status==Status_Ok) {
        status=kek_codegen_types_WriteFieldsSubst(program,out,decl,use);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"};\n");
    }
    if (status==Status_Ok) {
        use->emitted=1;
    }
    return (status);
}
bool kek_codegen_types_ShouldWritePlainTypeDecl(struct ModuleDecl* decl) {
    return (decl->kind==DeclKind_Alias||(decl->kind==DeclKind_Struct&&!decl->isGeneric)||decl->kind==DeclKind_Union||decl->kind==DeclKind_Enum);
}
Status kek_codegen_types_WritePlainTypeDeclaration(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl) {
    Status status=Status_Ok;
    if (decl->kind==DeclKind_Alias) {
        status=kek_codegen_types_WriteAliasDecl(program,out,decl);
    }
    if (decl->kind==DeclKind_Struct&&!decl->isGeneric) {
        status=kek_codegen_types_WriteStructDecl(program,out,decl,std_string_StringFromCString(""));
    }
    if (decl->kind==DeclKind_Union&&!decl->isTagged) {
        status=kek_codegen_types_WriteUnionDecl(program,out,decl);
    }
    if (decl->kind==DeclKind_Union&&decl->isTagged) {
        status=kek_codegen_types_WriteTaggedUnionDecl(program,out,decl);
    }
    if (decl->kind==DeclKind_Enum) {
        status=kek_codegen_types_WriteEnumDecl(program,out,decl);
    }
    if (status==Status_Ok) {
        decl->emitted=1;
    }
    return (status);
}
Status kek_codegen_types_WriteFieldTypeDependencies(struct CompilerContext* program,struct StringBuilder* out,struct ModuleField* field) {
    if (field->isNestedStruct) {
        return (kek_codegen_types_WriteTypeRangeDependencies(program,out,field->fileIndex,field->nestedBodyStart,field->nestedBodyEnd));
    }
    return (kek_codegen_types_WriteTypeRangeDependencies(program,out,field->fileIndex,field->typeStart,field->typeEnd));
}
Status kek_codegen_types_WriteTypeRangeDependencies(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize start,usize end) {
    for (usize i=start;i<end;i++) {
        struct Token token=program->tokenFiles.data[fileIndex].tokens[i];
        if (!(token.kind==TokenKind_Identifier||token.kind==TokenKind_Keyword)) {
            continue;
        }
        struct String name=kek_syntax_TokenText(program,fileIndex,i);
        if (String_EqualsCString(&name,"ptr")&&i+1<end&&kek_syntax_IsOperator(program,fileIndex,i+1,OperatorKind_Less)) {
            usize genericEnd=kek_module_FindMatching(program,fileIndex,i+1);
            if (i+3==genericEnd) {
                struct ModuleDecl* inner=kek_module_FindTypeDecl(program,kek_syntax_TokenText(program,fileIndex,i+2));
                if (inner!=0&&inner->kind!=DeclKind_Struct&&kek_codegen_types_ShouldWritePlainTypeDecl(inner)&&!inner->emitted) {
                    Status status=kek_codegen_types_WriteTypeDeclarationWithDependencies(program,out,inner);
                    if (status!=Status_Ok) {
                        return (status);
                    }
                }
            }
            i=genericEnd;
            continue;
        }
        if (i+1<end&&kek_syntax_IsOperator(program,fileIndex,i+1,OperatorKind_Less)) {
            usize genericEnd=kek_module_FindMatching(program,fileIndex,i+1);
            struct OwnedString key={0};
            Status status=kek_sema_BuildTypeKey(program,fileIndex,i,genericEnd+1,&key);
            if (status==Status_Ok) {
                status=kek_codegen_types_WriteTypeKeyDependencies(program,out,std_string_OwnedStringView(&key));
            }
            std_string_DestroyOwnedString(&key);
            if (status!=Status_Ok) {
                return (status);
            }
            i=genericEnd;
            continue;
        }
        struct ModuleDecl* decl=kek_module_FindTypeDecl(program,name);
        if (decl!=0&&kek_codegen_types_ShouldWritePlainTypeDecl(decl)&&!decl->emitted) {
            Status status=kek_codegen_types_WriteTypeDeclarationWithDependencies(program,out,decl);
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    return (Status_Ok);
}
Status kek_codegen_types_WriteTypeUseWithDependencies(struct CompilerContext* program,struct StringBuilder* out,struct TypeUse* use) {
    if (use->emitted) {
        return (Status_Ok);
    }
    for (usize i=0;i<use->args.len;i++) {
        Status status=kek_codegen_types_WriteTypeKeyDependencies(program,out,std_string_OwnedStringView(&use->args.data[i]));
        if (status!=Status_Ok) {
            return (status);
        }
    }
    struct ModuleDecl* decl=kek_module_FindTypeDecl(program,std_string_OwnedStringView(&use->baseName));
    if (decl!=0&&decl->kind==DeclKind_Struct) {
        for (usize i=0;i<decl->fieldCount;i++) {
            Status status=kek_codegen_types_WriteFieldTypeDependencies(program,out,&program->fields.data[decl->firstField+i]);
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    return (kek_codegen_types_WriteTypeUseDeclaration(program,out,use));
}
Status kek_codegen_types_WriteTypeKeyDependencies(struct CompilerContext* program,struct StringBuilder* out,struct String key) {
    struct String name=key;
    while (name.len>0&&name.data[name.len-1]=='*') {
        name=String_Slice(&name,0,name.len-1);
    }
    struct ModuleDecl* decl=kek_module_FindTypeDecl(program,name);
    if (decl!=0&&kek_codegen_types_ShouldWritePlainTypeDecl(decl)&&!decl->emitted) {
        return (kek_codegen_types_WriteTypeDeclarationWithDependencies(program,out,decl));
    }
    for (usize i=0;i<program->typeUses.len;i++) {
        struct String useKey=std_string_OwnedStringView(&program->typeUses.data[i].key);
        if (String_Equals(&useKey,name)) {
            return (kek_codegen_types_WriteTypeUseWithDependencies(program,out,&program->typeUses.data[i]));
        }
    }
    return (Status_Ok);
}
Status kek_codegen_types_WriteTypeDeclarationWithDependencies(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl) {
    if (decl->emitted) {
        return (Status_Ok);
    }
    Status status=Status_Ok;
    if (decl->kind==DeclKind_Alias) {
        status=kek_codegen_types_WriteTypeRangeDependencies(program,out,decl->fileIndex,decl->returnStart,decl->returnEnd);
    }
    if (decl->kind==DeclKind_Struct||decl->kind==DeclKind_Union) {
        for (usize i=0;i<decl->fieldCount;i++) {
            status=kek_codegen_types_WriteFieldTypeDependencies(program,out,&program->fields.data[decl->firstField+i]);
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    if (status==Status_Ok) {
        status=kek_codegen_types_WritePlainTypeDeclaration(program,out,decl);
    }
    return (status);
}
Status kek_codegen_types_WriteTypeDeclarations(struct CompilerContext* program,struct StringBuilder* out) {
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if (decl->kind==DeclKind_ExternC) {
            Status status=kek_codegen_types_WriteExternDecl(program,out,decl);
            if (status!=Status_Ok) {
                return (status);
            }
            decl->emitted=1;
        }
    }
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if (decl->reachable&&!decl->emitted&&kek_codegen_types_ShouldWritePlainTypeDecl(decl)) {
            Status status=kek_codegen_types_WriteTypeDeclarationWithDependencies(program,out,decl);
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    for (usize i=0;i<program->typeUses.len;i++) {
        struct TypeUse* use=&program->typeUses.data[i];
        Status status=kek_codegen_types_WriteTypeUseWithDependencies(program,out,use);
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
struct CodeWriter kek_codegen_emit_CodeWriterNew(struct StringBuilder* out) {
    struct CodeWriter writer={0};
    writer.out=out;
    writer.status=Status_Ok;
    return (writer);
}
void CodeWriter_CString(struct CodeWriter* this,str text) {
    if (this->status==Status_Ok) {
        this->status=StringBuilder_WriteCString(this->out,text);
    }
}
void CodeWriter_String(struct CodeWriter* this,struct String text) {
    if (this->status==Status_Ok) {
        this->status=StringBuilder_WriteString(this->out,text);
    }
}
void CodeWriter_Byte(struct CodeWriter* this,byte value) {
    if (this->status==Status_Ok) {
        this->status=StringBuilder_WriteByte(this->out,value);
    }
}
void CodeWriter_Indent(struct CodeWriter* this,usize indent) {
    if (this->status==Status_Ok) {
        this->status=kek_codegen_emit_WriteIndent(this->out,indent);
    }
}
void CodeWriter_Token(struct CodeWriter* this,struct CompilerContext* program,usize fileIndex,usize index) {
    if (this->status==Status_Ok) {
        this->status=kek_syntax_WriteToken(program,this->out,fileIndex,index);
    }
}
void CodeWriter_TokenRangeRaw(struct CodeWriter* this,struct CompilerContext* program,usize fileIndex,usize start,usize end) {
    if (this->status==Status_Ok) {
        this->status=kek_sema_WriteTokenRangeRaw(program,this->out,fileIndex,start,end);
    }
}
Status kek_codegen_emit_WritePrelude(struct StringBuilder* out) {
    struct TemplateArg args[1]={std_template_TemplateArgNew(std_string_StringFromCString("ptr_type"),std_string_StringFromCString("void*")),};
    struct TemplateContext ctx=std_template_TemplateContextNew(args,((void)(args),1),0,0);
    return (std_template_RenderTemplateToBuilder(out,std_string_StringFromCString("#include <assert.h>\n#include <stdint.h>\n#include <stddef.h>\n#include <stdbool.h>\n\ntypedef uint8_t u8;\ntypedef uint16_t u16;\ntypedef uint32_t u32;\ntypedef uint64_t u64;\ntypedef int8_t i8;\ntypedef int16_t i16;\ntypedef int32_t i32;\ntypedef int64_t i64;\ntypedef float f32;\ntypedef double f64;\ntypedef {ptr_type} ptr;\ntypedef const char* str;\n\n"),ctx));
}
Status kek_codegen_emit_WriteIndent(struct StringBuilder* out,usize indent) {
    return (StringBuilder_WriteIndent(out,indent*4));
}
Status kek_codegen_emit_WriteManualLine(struct StringBuilder* out,str text) {
    struct CodeWriter w=kek_codegen_emit_CodeWriterNew(out);
    CodeWriter_CString(&w,text);
    CodeWriter_Byte(&w,'\n');
    return (w.status);
}
Status kek_codegen_state_EnvInit(struct CompilerContext* program,struct CodegenEnv* env) {
    env->locals=std_array_ArrayNew__Local(program->allocator);
    env->genericDecl=0;
    env->genericFuncUse=0;
    env->hasThis=0;
    env->deferCounter=0;
    env->eachCounter=0;
    env->selectCounter=0;
    Status status=kek_syntax_MakeOwnedEmpty(program,&env->returnTypeKey);
    if (status==Status_Ok) {
        status=kek_syntax_MakeOwnedEmpty(program,&env->returnCType);
    }
    if (status==Status_Ok) {
        status=kek_syntax_MakeOwnedEmpty(program,&env->thisTypeKey);
    }
    if (status==Status_Ok) {
        status=kek_syntax_MakeOwnedEmpty(program,&env->thisCType);
    }
    return (status);
}
Status kek_codegen_state_EnvDestroy(struct CodegenEnv* env) {
    for (usize i=0;i<env->locals.len;i++) {
        std_string_DestroyOwnedString(&env->locals.data[i].name);
        std_string_DestroyOwnedString(&env->locals.data[i].typeKey);
        std_string_DestroyOwnedString(&env->locals.data[i].cType);
        std_string_DestroyOwnedString(&env->locals.data[i].arrayLen);
    }
    struct Array__Local locals=env->locals;
    Status status=Array__Local_Destroy(&locals);
    env->locals=locals;
    std_string_DestroyOwnedString(&env->returnTypeKey);
    std_string_DestroyOwnedString(&env->returnCType);
    std_string_DestroyOwnedString(&env->thisTypeKey);
    std_string_DestroyOwnedString(&env->thisCType);
    return (status);
}
Status kek_codegen_state_ReserveLocals(struct CodegenEnv* env,usize additional) {
    struct Array__Local locals=env->locals;
    Status status=Array__Local_Reserve(&locals,additional);
    env->locals=locals;
    return (status);
}
Status kek_codegen_state_EnvRestore(struct CodegenEnv* env,usize localCount) {
    while (env->locals.len>localCount) {
        env->locals.len-=1;
        struct Local* local=&env->locals.data[env->locals.len];
        std_string_DestroyOwnedString(&local->name);
        std_string_DestroyOwnedString(&local->typeKey);
        std_string_DestroyOwnedString(&local->cType);
        std_string_DestroyOwnedString(&local->arrayLen);
    }
    return (Status_Ok);
}
struct Local* kek_codegen_state_EnvFind(struct CodegenEnv* env,struct String name) {
    for (usize i=env->locals.len;i>0;i--) {
        struct Local* local=&env->locals.data[i-1];
        struct String localName=std_string_OwnedStringView(&local->name);
        if (String_Equals(&localName,name)) {
            return (local);
        }
    }
    return (0);
}
Status kek_codegen_state_EnvAdd(struct CompilerContext* program,struct CodegenEnv* env,struct String name,struct String typeKey,struct String cType,bool isArray,struct String arrayLen,bool isPointer) {
    Status status=kek_codegen_state_ReserveLocals(env,1);
    if (status!=Status_Ok) {
        return (status);
    }
    struct Local* local=&env->locals.data[env->locals.len];
    status=kek_syntax_CloneContextString(program,name,&local->name);
    if (status==Status_Ok) {
        status=kek_syntax_CloneContextString(program,typeKey,&local->typeKey);
    }
    if (status==Status_Ok) {
        status=kek_syntax_CloneContextString(program,cType,&local->cType);
    }
    if (status==Status_Ok) {
        status=kek_syntax_CloneContextString(program,arrayLen,&local->arrayLen);
    }
    local->isArray=isArray;
    local->isPointer=isPointer;
    if (status==Status_Ok) {
        env->locals.len+=1;
    }
    return (status);
}
bool kek_sema_IsTypeDeclKind(DeclKind kind) {
    return (kind==DeclKind_Struct||kind==DeclKind_Union||kind==DeclKind_Enum||kind==DeclKind_Alias);
}
int kek_sema_DeclNameIndexCompare(struct DeclNameIndexEntry* left,struct DeclNameIndexEntry* right) {
    struct String leftName=std_string_OwnedStringView(&left->name);
    struct String rightName=std_string_OwnedStringView(&right->name);
    int nameCompare=String_Compare(&leftName,rightName);
    if (nameCompare!=0) {
        return (nameCompare);
    }
    if (left->declIndex<right->declIndex) {
        return (-1);
    }
    if (left->declIndex>right->declIndex) {
        return (1);
    }
    return (0);
}
Status kek_sema_SortDeclNameIndex(struct Array__DeclNameIndexEntry* items) {
    for (usize i=1;i<items->len;i++) {
        struct DeclNameIndexEntry value=items->data[i];
        usize j=i;
        while (j>0&&kek_sema_DeclNameIndexCompare(&value,&items->data[j-1])<0) {
            items->data[j]=items->data[j-1];
            j-=1;
        }
        items->data[j]=value;
    }
    return (Status_Ok);
}
Status kek_sema_ClearDeclNameIndex(struct Array__DeclNameIndexEntry* items) {
    for (usize i=0;i<items->len;i++) {
        std_string_DestroyOwnedString(&items->data[i].name);
        std_string_DestroyOwnedString(&items->data[i].scope);
    }
    items->len=0;
    return (Status_Ok);
}
int kek_sema_MethodDeclIndexCompare(struct MethodDeclIndexEntry* left,struct MethodDeclIndexEntry* right) {
    struct String leftReceiver=std_string_OwnedStringView(&left->receiverKey);
    struct String rightReceiver=std_string_OwnedStringView(&right->receiverKey);
    int receiverCompare=String_Compare(&leftReceiver,rightReceiver);
    if (receiverCompare!=0) {
        return (receiverCompare);
    }
    struct String leftName=std_string_OwnedStringView(&left->name);
    struct String rightName=std_string_OwnedStringView(&right->name);
    int nameCompare=String_Compare(&leftName,rightName);
    if (nameCompare!=0) {
        return (nameCompare);
    }
    if (left->isOperator&&!right->isOperator) {
        return (-1);
    }
    if (!left->isOperator&&right->isOperator) {
        return (1);
    }
    if (left->operatorCode<right->operatorCode) {
        return (-1);
    }
    if (left->operatorCode>right->operatorCode) {
        return (1);
    }
    if (left->paramCount<right->paramCount) {
        return (-1);
    }
    if (left->paramCount>right->paramCount) {
        return (1);
    }
    if (left->declIndex<right->declIndex) {
        return (-1);
    }
    if (left->declIndex>right->declIndex) {
        return (1);
    }
    return (0);
}
Status kek_sema_SortMethodDeclIndex(struct Array__MethodDeclIndexEntry* items) {
    for (usize i=1;i<items->len;i++) {
        struct MethodDeclIndexEntry value=items->data[i];
        usize j=i;
        while (j>0&&kek_sema_MethodDeclIndexCompare(&value,&items->data[j-1])<0) {
            items->data[j]=items->data[j-1];
            j-=1;
        }
        items->data[j]=value;
    }
    return (Status_Ok);
}
Status kek_sema_ClearMethodDeclIndex(struct Array__MethodDeclIndexEntry* items) {
    for (usize i=0;i<items->len;i++) {
        std_string_DestroyOwnedString(&items->data[i].receiverKey);
        std_string_DestroyOwnedString(&items->data[i].name);
    }
    items->len=0;
    return (Status_Ok);
}
Status kek_sema_AddDeclNameIndexEntry(struct CompilerContext* program,struct Array__DeclNameIndexEntry* items,struct String name,struct String scope,usize declIndex) {
    struct DeclNameIndexEntry entry={0};
    Status status=kek_syntax_CloneContextString(program,name,&entry.name);
    if (status==Status_Ok) {
        status=kek_syntax_CloneContextString(program,scope,&entry.scope);
    }
    entry.declIndex=declIndex;
    if (status==Status_Ok) {
        struct Array__DeclNameIndexEntry indexItems=items[0];
        status=Array__DeclNameIndexEntry_Push(&indexItems,entry);
        items[0]=indexItems;
    }
    if (status!=Status_Ok) {
        std_string_DestroyOwnedString(&entry.name);
        std_string_DestroyOwnedString(&entry.scope);
    }
    return (status);
}
Status kek_sema_AddMethodDeclIndexEntry(struct CompilerContext* program,struct Array__MethodDeclIndexEntry* items,struct ModuleDecl* decl,usize declIndex) {
    struct MethodDeclIndexEntry entry={0};
    struct StringBuilder receiver=std_string_StringBuilderNew(program->allocator);
    Status status=kek_sema_WriteTypeSuffixFromRange(program,&receiver,decl->fileIndex,decl->receiverStart,decl->receiverEnd);
    if (status==Status_Ok) {
        status=kek_syntax_DetachBuilder(&receiver,&entry.receiverKey);
    }
    StringBuilder_Destroy(&receiver);
    if (status==Status_Ok) {
        if (decl->isOperator) {
            status=kek_syntax_CloneContextCString(program,"",&entry.name);
        } else {
            status=kek_syntax_CloneContextString(program,kek_syntax_TokenText(program,decl->fileIndex,decl->nameIndex),&entry.name);
        }
    }
    entry.declIndex=declIndex;
    entry.operatorCode=decl->operatorCode;
    entry.paramCount=decl->paramCount;
    entry.isOperator=decl->isOperator;
    if (status==Status_Ok) {
        struct Array__MethodDeclIndexEntry indexItems=items[0];
        status=Array__MethodDeclIndexEntry_Push(&indexItems,entry);
        items[0]=indexItems;
    }
    if (status!=Status_Ok) {
        std_string_DestroyOwnedString(&entry.receiverKey);
        std_string_DestroyOwnedString(&entry.name);
    }
    return (status);
}
struct String kek_sema_ModuleDeclScopeName(struct ModuleDecl* decl) {
    struct String packageName=std_string_OwnedStringView(&decl->packageName);
    if (packageName.len>0) {
        return (packageName);
    }
    return (std_string_OwnedStringView(&decl->moduleName));
}
Status kek_sema_BuildDeclIndexes(struct CompilerContext* program) {
    kek_sema_ClearDeclNameIndex(&program->typeDeclIndex);
    kek_sema_ClearDeclNameIndex(&program->functionDeclIndex);
    kek_sema_ClearMethodDeclIndex(&program->methodDeclIndex);
    Status status=kek_syntax_ReserveTypeDeclIndex(program,program->decls.len);
    if (status==Status_Ok) {
        status=kek_syntax_ReserveFunctionDeclIndex(program,program->decls.len);
    }
    if (status==Status_Ok) {
        status=kek_syntax_ReserveMethodDeclIndex(program,program->decls.len);
    }
    for (usize i=0;i<program->decls.len&&status==Status_Ok;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        struct String name=kek_syntax_TokenText(program,decl->fileIndex,decl->nameIndex);
        if (kek_sema_IsTypeDeclKind(decl->kind)) {
            status=kek_sema_AddDeclNameIndexEntry(program,&program->typeDeclIndex,name,std_string_StringFromCString(""),i);
        } else {
            if (decl->kind==DeclKind_Function&&!decl->hasReceiver) {
                status=kek_sema_AddDeclNameIndexEntry(program,&program->functionDeclIndex,name,kek_sema_ModuleDeclScopeName(decl),i);
            } else {
                if (decl->kind==DeclKind_Function&&decl->hasReceiver) {
                    status=kek_sema_AddMethodDeclIndexEntry(program,&program->methodDeclIndex,decl,i);
                }
            }
        }
    }
    if (status==Status_Ok) {
        status=kek_sema_SortDeclNameIndex(&program->typeDeclIndex);
    }
    if (status==Status_Ok) {
        status=kek_sema_SortDeclNameIndex(&program->functionDeclIndex);
    }
    if (status==Status_Ok) {
        status=kek_sema_SortMethodDeclIndex(&program->methodDeclIndex);
    }
    return (status);
}
struct ModuleDecl* kek_sema_FindDeclByIndexedName(struct CompilerContext* program,struct Array__DeclNameIndexEntry* items,struct String name,struct String scope) {
    usize low=0;
    usize high=items->len;
    while (low<high) {
        usize mid=low+(high-low)/2;
        struct String midName=std_string_OwnedStringView(&items->data[mid].name);
        int nameCompare=String_Compare(&midName,name);
        if (nameCompare<0) {
            low=mid+1;
            continue;
        }
        if (nameCompare>0) {
            high=mid;
            continue;
        }
        while (mid>0) {
            struct String prevName=std_string_OwnedStringView(&items->data[mid-1].name);
            if (!String_Equals(&prevName,name)) {
                break;
            }
            mid-=1;
        }
        for (usize i=mid;i<items->len;i++) {
            struct String itemName=std_string_OwnedStringView(&items->data[i].name);
            if (!String_Equals(&itemName,name)) {
                break;
            }
            struct String itemScope=std_string_OwnedStringView(&items->data[i].scope);
            if (scope.len==0||String_Equals(&itemScope,scope)) {
                return (&program->decls.data[items->data[i].declIndex]);
            }
        }
        return (0);
    }
    return (0);
}
struct ModuleDecl* kek_sema_FindIndexedFunctionDeclByName(struct CompilerContext* program,struct String name,struct String scopeName) {
    if (program->functionDeclIndex.len==0) {
        return (0);
    }
    return (kek_sema_FindDeclByIndexedName(program,&program->functionDeclIndex,name,scopeName));
}
struct ModuleDecl* kek_sema_FindIndexedMethodDecl(struct CompilerContext* program,struct String receiverType,struct String name,bool isOperator,u8 operatorCode,usize argCount) {
    if (program->methodDeclIndex.len==0) {
        return (0);
    }
    usize low=0;
    usize high=program->methodDeclIndex.len;
    while (low<high) {
        usize mid=low+(high-low)/2;
        struct String midReceiver=std_string_OwnedStringView(&program->methodDeclIndex.data[mid].receiverKey);
        int compare=String_Compare(&midReceiver,receiverType);
        if (compare<0) {
            low=mid+1;
            continue;
        }
        if (compare>0) {
            high=mid;
            continue;
        }
        while (mid>0) {
            struct String prevReceiver=std_string_OwnedStringView(&program->methodDeclIndex.data[mid-1].receiverKey);
            if (!String_Equals(&prevReceiver,receiverType)) {
                break;
            }
            mid-=1;
        }
        for (usize i=mid;i<program->methodDeclIndex.len;i++) {
            struct MethodDeclIndexEntry* entry=&program->methodDeclIndex.data[i];
            struct String itemReceiver=std_string_OwnedStringView(&entry->receiverKey);
            if (!String_Equals(&itemReceiver,receiverType)) {
                break;
            }
            if (entry->isOperator!=isOperator) {
                continue;
            }
            if (isOperator) {
                if (entry->operatorCode==operatorCode&&entry->paramCount==argCount) {
                    return (&program->decls.data[entry->declIndex]);
                }
            } else {
                struct String itemName=std_string_OwnedStringView(&entry->name);
                if (String_Equals(&itemName,name)) {
                    return (&program->decls.data[entry->declIndex]);
                }
            }
        }
        return (0);
    }
    return (0);
}
bool kek_sema_MarkDeclReachable(struct ModuleDecl* decl) {
    if (decl->reachable) {
        return (0);
    }
    decl->reachable=1;
    return (1);
}
bool kek_sema_MarkReachableRoots(struct CompilerContext* program) {
    bool changed=0;
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if (decl->fileIndex==0&&decl->kind!=DeclKind_ExternC&&!decl->isGeneric) {
            if (kek_sema_MarkDeclReachable(decl)) {
                changed=1;
            }
        }
    }
    return (changed);
}
bool kek_sema_MarkNamedDeclsReachable(struct CompilerContext* program,struct String name) {
    bool changed=0;
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if (decl->kind==DeclKind_ExternC||decl->isOperator) {
            continue;
        }
        if (kek_module_ModuleDeclNameMatches(program,decl,name)) {
            if (kek_sema_MarkDeclReachable(decl)) {
                changed=1;
            }
        }
    }
    return (changed);
}
bool kek_sema_MarkOperatorDeclsReachable(struct CompilerContext* program,u8 operatorCode) {
    if (operatorCode==0) {
        return (0);
    }
    bool changed=0;
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if (decl->kind==DeclKind_Function&&decl->isOperator&&decl->operatorCode==operatorCode) {
            if (kek_sema_MarkDeclReachable(decl)) {
                changed=1;
            }
        }
    }
    return (changed);
}
bool kek_sema_MarkReferencesInRange(struct CompilerContext* program,usize fileIndex,usize start,usize end) {
    bool changed=0;
    for (usize i=start;i<end;i++) {
        struct Token token=program->tokenFiles.data[fileIndex].tokens[i];
        if (token.kind==TokenKind_Identifier||token.kind==TokenKind_Keyword) {
            if (kek_sema_MarkNamedDeclsReachable(program,kek_syntax_TokenText(program,fileIndex,i))) {
                changed=1;
            }
        }
        if (token.kind==TokenKind_Operator) {
            if (kek_sema_MarkOperatorDeclsReachable(program,kek_module_OperatorCode(program,fileIndex,i))) {
                changed=1;
            }
        }
    }
    return (changed);
}
Status kek_sema_MarkReachableDecls(struct CompilerContext* program) {
    bool changed=kek_sema_MarkReachableRoots(program);
    while (changed) {
        changed=0;
        for (usize i=0;i<program->decls.len;i++) {
            struct ModuleDecl* decl=&program->decls.data[i];
            if (!decl->reachable) {
                continue;
            }
            if (kek_sema_MarkReferencesInRange(program,decl->fileIndex,decl->start,decl->end)) {
                changed=1;
            }
        }
    }
    return (Status_Ok);
}
Status kek_sema_WriteTokenRangeRaw(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize start,usize end) {
    for (usize i=start;i<end;i++) {
        Status status=kek_syntax_WriteToken(program,out,fileIndex,i);
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
bool kek_sema_IsBuiltinTypeName(struct String name) {
    return (String_EqualsCString(&name,"void")||String_EqualsCString(&name,"bool")||String_EqualsCString(&name,"int")||String_EqualsCString(&name,"u8")||String_EqualsCString(&name,"u16")||String_EqualsCString(&name,"u32")||String_EqualsCString(&name,"u64")||String_EqualsCString(&name,"i8")||String_EqualsCString(&name,"i16")||String_EqualsCString(&name,"i32")||String_EqualsCString(&name,"i64")||String_EqualsCString(&name,"f32")||String_EqualsCString(&name,"f64")||String_EqualsCString(&name,"ptr")||String_EqualsCString(&name,"str")||String_EqualsCString(&name,"byte")||String_EqualsCString(&name,"usize")||String_EqualsCString(&name,"isize")||String_EqualsCString(&name,"RawHandle"));
}
Status kek_sema_WriteSanitized(struct StringBuilder* out,struct String text) {
    for (usize i=0;i<text.len;i++) {
        byte c=text.data[i];
        if ((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_') {
            Status status=StringBuilder_WriteByte(out,c);
            if (status!=Status_Ok) {
                return (status);
            }
        } else {
            Status status=StringBuilder_WriteByte(out,'_');
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    return (Status_Ok);
}
Status kek_sema_WriteOperatorName(struct StringBuilder* out,u8 operatorCode) {
    u64 kind=((u64)(operatorCode));
    if (kind==((u64)(OperatorKind_Plus))+1) {
        return (StringBuilder_WriteCString(out,"plus"));
    }
    if (kind==((u64)(OperatorKind_Minus))+1) {
        return (StringBuilder_WriteCString(out,"minus"));
    }
    if (kind==((u64)(OperatorKind_Equal))+1) {
        return (StringBuilder_WriteCString(out,"equal"));
    }
    if (kind==((u64)(OperatorKind_PlusAssign))+1) {
        return (StringBuilder_WriteCString(out,"plus_assign"));
    }
    if (kind==((u64)(OperatorKind_MinusAssign))+1) {
        return (StringBuilder_WriteCString(out,"minus_assign"));
    }
    if (kind==((u64)(OperatorKind_Less))+1) {
        return (StringBuilder_WriteCString(out,"less"));
    }
    if (kind==((u64)(OperatorKind_Greater))+1) {
        return (StringBuilder_WriteCString(out,"greater"));
    }
    return (StringBuilder_WriteCString(out,"op"));
}
bool kek_sema_ModuleDeclPackageIsRoot(struct ModuleDecl* decl) {
    struct String packageName=std_string_OwnedStringView(&decl->packageName);
    struct String moduleName=std_string_OwnedStringView(&decl->moduleName);
    return (packageName.len==0&&moduleName.len==0);
}
Status kek_sema_WriteDeclCName(struct CompilerContext* program,struct StringBuilder* out,struct ModuleDecl* decl) {
    if (decl->hasReceiver) {
        Status status=kek_sema_WriteTypeSuffixFromRange(program,out,decl->fileIndex,decl->receiverStart,decl->receiverEnd);
        if (status!=Status_Ok) {
            return (status);
        }
        status=StringBuilder_WriteByte(out,'_');
        if (status!=Status_Ok) {
            return (status);
        }
        if (decl->isOperator) {
            status=StringBuilder_WriteCString(out,"operator_");
            if (status==Status_Ok) {
                status=kek_sema_WriteOperatorName(out,decl->operatorCode);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"_");
            }
            if (status==Status_Ok) {
                status=std_format_FormatU64ToBuilder(out,((u64)(decl->paramCount)),10);
            }
            return (status);
        }
        return (kek_syntax_WriteToken(program,out,decl->fileIndex,decl->nameIndex));
    }
    struct String packageName=std_string_OwnedStringView(&decl->packageName);
    struct String moduleName=std_string_OwnedStringView(&decl->moduleName);
    if (packageName.len>0) {
        Status status=StringBuilder_WriteString(out,packageName);
        if (status!=Status_Ok) {
            return (status);
        }
        status=StringBuilder_WriteByte(out,'_');
        if (status!=Status_Ok) {
            return (status);
        }
    }
    if (moduleName.len>0) {
        Status status=StringBuilder_WriteString(out,moduleName);
        if (status!=Status_Ok) {
            return (status);
        }
        status=StringBuilder_WriteByte(out,'_');
        if (status!=Status_Ok) {
            return (status);
        }
    }
    Status nameStatus=kek_syntax_WriteToken(program,out,decl->fileIndex,decl->nameIndex);
    if (nameStatus!=Status_Ok) {
        return (nameStatus);
    }
    if (decl->isGeneric&&decl->genericStart<kek_syntax_FileTokenCount(program,decl->fileIndex)) {
        return (Status_Ok);
    }
    return (Status_Ok);
}
Status kek_sema_WriteTypeSuffixFromRange(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize start,usize end) {
    if (start>=end) {
        return (Status_Ok);
    }
    if (start+1<end&&kek_syntax_IsOperator(program,fileIndex,start+1,OperatorKind_Less)) {
        Status status=kek_sema_WriteSanitized(out,kek_syntax_TokenText(program,fileIndex,start));
        if (status!=Status_Ok) {
            return (status);
        }
        usize genericEnd=kek_module_FindMatching(program,fileIndex,start+1);
        usize argStart=start+2;
        while (argStart<genericEnd) {
            usize argEnd=kek_module_FindTokenAtDepthZero(program,fileIndex,argStart,genericEnd,PunctuationKind_Comma);
            status=StringBuilder_WriteCString(out,"__");
            if (status!=Status_Ok) {
                return (status);
            }
            status=kek_sema_WriteTypeSuffixFromRange(program,out,fileIndex,argStart,argEnd);
            if (status!=Status_Ok) {
                return (status);
            }
            if (argEnd>=genericEnd) {
                break;
            }
            argStart=argEnd+1;
        }
        return (Status_Ok);
    }
    return (kek_sema_WriteSanitized(out,kek_syntax_TokenText(program,fileIndex,start)));
}
Status kek_sema_TypeInfoDestroy(struct TypeInfo* info) {
    std_string_DestroyOwnedString(&info->key);
    std_string_DestroyOwnedString(&info->cType);
    std_string_DestroyOwnedString(&info->baseName);
    for (usize i=0;i<info->args.len;i++) {
        std_string_DestroyOwnedString(&info->args.data[i]);
    }
    struct Array__OwnedString args=info->args;
    Array__OwnedString_Destroy(&args);
    info->args=args;
    return (Status_Ok);
}
Status kek_sema_TypeInfoInitEmpty(struct CompilerContext* program,struct TypeInfo* info) {
    Status status=kek_syntax_MakeOwnedEmpty(program,&info->key);
    if (status==Status_Ok) {
        status=kek_syntax_MakeOwnedEmpty(program,&info->cType);
    }
    if (status==Status_Ok) {
        status=kek_syntax_MakeOwnedEmpty(program,&info->baseName);
    }
    info->args=std_array_ArrayNew__OwnedString(program->allocator);
    info->isPointer=0;
    return (status);
}
struct String kek_sema_OwnedStringArrayGet(struct Array__OwnedString* args,usize index) {
    if (index>=args->len) {
        return (std_string_StringFromCString(""));
    }
    return (std_string_OwnedStringView(&args->data[index]));
}
Status kek_sema_OwnedStringArrayPushClone(struct CompilerContext* program,struct Array__OwnedString* args,struct String value) {
    struct OwnedString clone={0};
    Status status=kek_syntax_CloneContextString(program,value,&clone);
    if (status!=Status_Ok) {
        return (status);
    }
    status=Array__OwnedString_Push(args,clone);
    if (status!=Status_Ok) {
        std_string_DestroyOwnedString(&clone);
    }
    return (status);
}
Status kek_sema_CloneOwnedStringArray(struct CompilerContext* program,struct Array__OwnedString* source,struct Array__OwnedString* target) {
    target[0]=std_array_ArrayNew__OwnedString(program->allocator);
    for (usize i=0;i<source->len;i++) {
        Status status=kek_sema_OwnedStringArrayPushClone(program,target,std_string_OwnedStringView(&source->data[i]));
        if (status!=Status_Ok) {
            for (usize j=0;j<target->len;j++) {
                std_string_DestroyOwnedString(&target->data[j]);
            }
            struct Array__OwnedString items=target[0];
            Array__OwnedString_Destroy(&items);
            target[0]=items;
            return (status);
        }
    }
    return (Status_Ok);
}
Status kek_sema_ReplaceOwned(struct OwnedString* target,struct OwnedString value) {
    std_string_DestroyOwnedString(target);
    target->data=value.data;
    target->len=value.len;
    target->cap=value.cap;
    target->allocator=value.allocator;
    return (Status_Ok);
}
Status kek_sema_BuildTypeKey(struct CompilerContext* program,usize fileIndex,usize start,usize end,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
    Status status=kek_sema_WriteTypeSuffixFromRange(program,&builder,fileIndex,start,end);
    if (status==Status_Ok) {
        status=kek_syntax_DetachBuilder(&builder,out);
    }
    StringBuilder_Destroy(&builder);
    return (status);
}
Status kek_sema_WriteCTypeFromKey(struct CompilerContext* program,struct StringBuilder* out,struct String key) {
    if (String_EqualsCString(&key,"f32")) {
        return (StringBuilder_WriteCString(out,"float"));
    }
    if (String_EqualsCString(&key,"f64")) {
        return (StringBuilder_WriteCString(out,"double"));
    }
    if (String_EqualsCString(&key,"ptr")) {
        return (StringBuilder_WriteCString(out,"ptr"));
    }
    if (String_EqualsCString(&key,"str")) {
        return (StringBuilder_WriteCString(out,"str"));
    }
    if (kek_sema_IsBuiltinTypeName(key)) {
        return (StringBuilder_WriteString(out,key));
    }
    struct Result__usize ptrMarker=String_FindByte(&key,'*');
    if (ptrMarker.status==Status_Ok) {
        return (StringBuilder_WriteString(out,key));
    }
    struct ModuleDecl* decl=kek_module_FindTypeDecl(program,key);
    if (decl!=0) {
        if (decl->kind==DeclKind_Struct) {
            Status status=StringBuilder_WriteCString(out,"struct ");
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,key);
            }
            return (status);
        }
        return (StringBuilder_WriteString(out,key));
    }
    for (usize i=0;i<program->typeUses.len;i++) {
        struct String useKey=std_string_OwnedStringView(&program->typeUses.data[i].key);
        if (String_Equals(&useKey,key)) {
            struct ModuleDecl* baseDecl=kek_module_FindTypeDecl(program,std_string_OwnedStringView(&program->typeUses.data[i].baseName));
            if (baseDecl!=0&&baseDecl->kind==DeclKind_Struct) {
                Status status=StringBuilder_WriteCString(out,"struct ");
                if (status==Status_Ok) {
                    status=StringBuilder_WriteString(out,key);
                }
                return (status);
            }
            return (StringBuilder_WriteString(out,key));
        }
    }
    if (key.len>0&&key.data[0]>='A'&&key.data[0]<='Z') {
        Status status=StringBuilder_WriteCString(out,"struct ");
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,key);
        }
        return (status);
    }
    return (StringBuilder_WriteString(out,key));
}
Status kek_sema_MakeCTypeFromKey(struct CompilerContext* program,struct String key,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
    Status status=kek_sema_WriteCTypeFromKey(program,&builder,key);
    if (status==Status_Ok) {
        status=kek_syntax_DetachBuilder(&builder,out);
    }
    StringBuilder_Destroy(&builder);
    return (status);
}
Status kek_sema_TypeInfoFromKey(struct CompilerContext* program,struct String key,struct TypeInfo* info) {
    Status status=kek_sema_TypeInfoInitEmpty(program,info);
    if (status!=Status_Ok) {
        return (status);
    }
    struct OwnedString keyOwned={0};
    status=kek_syntax_CloneContextString(program,key,&keyOwned);
    if (status==Status_Ok) {
        status=kek_sema_ReplaceOwned(&info->key,keyOwned);
    }
    struct OwnedString cOwned={0};
    if (status==Status_Ok) {
        status=kek_sema_MakeCTypeFromKey(program,key,&cOwned);
    }
    if (status==Status_Ok) {
        status=kek_sema_ReplaceOwned(&info->cType,cOwned);
    }
    struct Result__usize ptrMarker=String_FindByte(&key,'*');
    info->isPointer=ptrMarker.status==Status_Ok;
    return (status);
}
Status kek_sema_RenderTypeInfo(struct CompilerContext* program,usize fileIndex,usize start,usize end,struct TypeInfo* info) {
    Status status=kek_sema_TypeInfoInitEmpty(program,info);
    if (status!=Status_Ok) {
        return (status);
    }
    if (start>=end) {
        return (Status_Ok);
    }
    struct String first=kek_syntax_TokenText(program,fileIndex,start);
    if (String_EqualsCString(&first,"ptr")&&start+1<end&&kek_syntax_IsOperator(program,fileIndex,start+1,OperatorKind_Less)) {
        usize genericEnd=kek_module_FindMatching(program,fileIndex,start+1);
        struct TypeInfo inner={0};
        status=kek_sema_RenderTypeInfo(program,fileIndex,start+2,genericEnd,&inner);
        if (status!=Status_Ok) {
            return (status);
        }
        struct StringBuilder key=std_string_StringBuilderNew(program->allocator);
        status=StringBuilder_WriteString(&key,std_string_OwnedStringView(&inner.key));
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(&key,'*');
        }
        struct OwnedString keyOwned={0};
        if (status==Status_Ok) {
            status=kek_syntax_DetachBuilder(&key,&keyOwned);
        }
        StringBuilder_Destroy(&key);
        if (status==Status_Ok) {
            status=kek_sema_ReplaceOwned(&info->key,keyOwned);
        }
        struct StringBuilder cType=std_string_StringBuilderNew(program->allocator);
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&cType,std_string_OwnedStringView(&inner.cType));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(&cType,'*');
        }
        struct OwnedString cOwned={0};
        if (status==Status_Ok) {
            status=kek_syntax_DetachBuilder(&cType,&cOwned);
        }
        StringBuilder_Destroy(&cType);
        if (status==Status_Ok) {
            status=kek_sema_ReplaceOwned(&info->cType,cOwned);
        }
        info->isPointer=1;
        kek_sema_TypeInfoDestroy(&inner);
        return (status);
    }
    status=kek_sema_BuildTypeKey(program,fileIndex,start,end,&info->key);
    if (status!=Status_Ok) {
        return (status);
    }
    if (start+1<end&&kek_syntax_IsOperator(program,fileIndex,start+1,OperatorKind_Less)) {
        status=kek_syntax_CloneContextString(program,first,&info->baseName);
        if (status!=Status_Ok) {
            return (status);
        }
        usize genericEnd=kek_module_FindMatching(program,fileIndex,start+1);
        usize argStart=start+2;
        while (argStart<genericEnd) {
            usize argEnd=kek_module_FindTokenAtDepthZero(program,fileIndex,argStart,genericEnd,PunctuationKind_Comma);
            struct TypeInfo argInfo={0};
            status=kek_sema_RenderTypeInfo(program,fileIndex,argStart,argEnd,&argInfo);
            if (status!=Status_Ok) {
                return (status);
            }
            status=kek_sema_OwnedStringArrayPushClone(program,&info->args,std_string_OwnedStringView(&argInfo.key));
            kek_sema_TypeInfoDestroy(&argInfo);
            if (status!=Status_Ok) {
                return (status);
            }
            if (argEnd>=genericEnd) {
                break;
            }
            argStart=argEnd+1;
        }
    }
    status=kek_sema_MakeCTypeFromKey(program,std_string_OwnedStringView(&info->key),&info->cType);
    return (status);
}
bool kek_sema_TypeUseExists(struct CompilerContext* program,struct String key) {
    for (usize i=0;i<program->typeUses.len;i++) {
        struct String useKey=std_string_OwnedStringView(&program->typeUses.data[i].key);
        if (String_Equals(&useKey,key)) {
            return (1);
        }
    }
    return (0);
}
Status kek_sema_AddTypeUse(struct CompilerContext* program,struct TypeInfo* info) {
    if (info->args.len==0) {
        return (Status_Ok);
    }
    struct String key=std_string_OwnedStringView(&info->key);
    if (kek_sema_TypeUseExists(program,key)) {
        return (Status_Ok);
    }
    Status status=kek_syntax_ReserveTypeUses(program,1);
    if (status!=Status_Ok) {
        return (status);
    }
    struct TypeUse* use=&program->typeUses.data[program->typeUses.len];
    status=kek_syntax_CloneContextString(program,key,&use->key);
    if (status==Status_Ok) {
        status=kek_syntax_CloneContextString(program,std_string_OwnedStringView(&info->cType),&use->cName);
    }
    if (status==Status_Ok) {
        status=kek_syntax_CloneContextString(program,std_string_OwnedStringView(&info->baseName),&use->baseName);
    }
    if (status==Status_Ok) {
        status=kek_sema_CloneOwnedStringArray(program,&info->args,&use->args);
    }
    use->emitted=0;
    if (status==Status_Ok) {
        program->typeUses.len+=1;
    }
    return (status);
}
Status kek_sema_AddTypeUseFromBaseArg(struct CompilerContext* program,str baseName,struct String arg0) {
    struct StringBuilder keyBuilder=std_string_StringBuilderNew(program->allocator);
    Status status=StringBuilder_WriteCString(&keyBuilder,baseName);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(&keyBuilder,"__");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(&keyBuilder,arg0);
    }
    struct String key=StringBuilder_View(&keyBuilder);
    if (status==Status_Ok&&kek_sema_TypeUseExists(program,key)) {
        StringBuilder_Destroy(&keyBuilder);
        return (Status_Ok);
    }
    if (status==Status_Ok) {
        status=kek_syntax_ReserveTypeUses(program,1);
    }
    if (status!=Status_Ok) {
        StringBuilder_Destroy(&keyBuilder);
        return (status);
    }
    struct TypeUse* use=&program->typeUses.data[program->typeUses.len];
    if (status==Status_Ok) {
        status=kek_syntax_CloneContextString(program,key,&use->key);
    }
    if (status==Status_Ok) {
        status=kek_syntax_CloneContextString(program,key,&use->cName);
    }
    if (status==Status_Ok) {
        status=kek_syntax_CloneContextString(program,std_string_StringFromCString(baseName),&use->baseName);
    }
    use->args=std_array_ArrayNew__OwnedString(program->allocator);
    if (status==Status_Ok) {
        status=kek_sema_OwnedStringArrayPushClone(program,&use->args,arg0);
    }
    use->emitted=0;
    if (status==Status_Ok) {
        program->typeUses.len+=1;
    }
    StringBuilder_Destroy(&keyBuilder);
    return (status);
}
bool kek_sema_ConcreteTypeKey(struct CompilerContext* program,struct String key) {
    if (key.len==0) {
        return (0);
    }
    if (kek_sema_IsBuiltinTypeName(key)) {
        return (1);
    }
    if (String_ContainsByte(&key,'*')) {
        return (1);
    }
    if (kek_module_FindTypeDecl(program,key)!=0) {
        return (1);
    }
    for (usize i=0;i<program->typeUses.len;i++) {
        struct String useKey=std_string_OwnedStringView(&program->typeUses.data[i].key);
        if (String_Equals(&useKey,key)) {
            return (1);
        }
    }
    return (0);
}
bool kek_sema_TypeInfoConcrete(struct CompilerContext* program,struct TypeInfo* info) {
    if (info->args.len==0) {
        return (kek_sema_ConcreteTypeKey(program,std_string_OwnedStringView(&info->key)));
    }
    for (usize i=0;i<info->args.len;i++) {
        if (!kek_sema_ConcreteTypeKey(program,std_string_OwnedStringView(&info->args.data[i]))) {
            return (0);
        }
    }
    return (1);
}
Status kek_sema_CollectTypeUsesInRange(struct CompilerContext* program,usize fileIndex,usize start,usize end) {
    usize i=start;
    while (i<end) {
        if (i+1<end&&kek_syntax_IsOperator(program,fileIndex,i+1,OperatorKind_Less)) {
            struct String name=kek_syntax_TokenText(program,fileIndex,i);
            struct ModuleDecl* decl=kek_module_FindTypeDecl(program,name);
            if (decl!=0&&decl->isGeneric) {
                usize genericEnd=kek_module_FindMatching(program,fileIndex,i+1);
                struct TypeInfo info={0};
                Status status=kek_sema_RenderTypeInfo(program,fileIndex,i,genericEnd+1,&info);
                if (status!=Status_Ok) {
                    return (status);
                }
                if (kek_sema_TypeInfoConcrete(program,&info)) {
                    status=kek_sema_AddTypeUse(program,&info);
                }
                kek_sema_TypeInfoDestroy(&info);
                if (status!=Status_Ok) {
                    return (status);
                }
                i=genericEnd+1;
                continue;
            }
        }
        i+=1;
    }
    return (Status_Ok);
}
bool kek_sema_ModuleDeclScopeMatches(struct ModuleDecl* decl,struct String scopeName) {
    if (scopeName.len==0) {
        return (1);
    }
    struct String declPackage=std_string_OwnedStringView(&decl->packageName);
    if (String_Equals(&declPackage,scopeName)) {
        return (1);
    }
    struct String declModule=std_string_OwnedStringView(&decl->moduleName);
    return (String_Equals(&declModule,scopeName));
}
struct ModuleDecl* kek_sema_FindFunctionDeclByName(struct CompilerContext* program,struct String name,struct String scopeName) {
    struct ModuleDecl* indexed=kek_sema_FindIndexedFunctionDeclByName(program,name,scopeName);
    if (indexed!=0) {
        return (indexed);
    }
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if (decl->kind!=DeclKind_Function) {
            continue;
        }
        if (decl->hasReceiver) {
            continue;
        }
        if (!kek_module_ModuleDeclNameMatches(program,decl,name)) {
            continue;
        }
        if (kek_sema_ModuleDeclScopeMatches(decl,scopeName)) {
            return (decl);
        }
    }
    return (0);
}
bool kek_sema_FuncUseExists(struct CompilerContext* program,usize declIndex,struct String key) {
    for (usize i=0;i<program->funcUses.len;i++) {
        struct String useKey=std_string_OwnedStringView(&program->funcUses.data[i].key);
        if (program->funcUses.data[i].declIndex==declIndex&&String_Equals(&useKey,key)) {
            return (1);
        }
    }
    return (0);
}
usize kek_sema_ModuleDeclIndex(struct CompilerContext* program,struct ModuleDecl* decl) {
    for (usize i=0;i<program->decls.len;i++) {
        if (&program->decls.data[i]==decl) {
            return (i);
        }
    }
    return (program->decls.len);
}
Status kek_sema_BuildGenericFuncCName(struct CompilerContext* program,struct ModuleDecl* decl,struct Array__TypeInfo* args,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
    Status status=Status_Ok;
    if (decl->hasReceiver&&args->len>=1) {
        status=kek_sema_WriteSanitized(&builder,kek_syntax_TokenText(program,decl->fileIndex,decl->receiverStart));
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(&builder,"__");
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&args->data[0].key));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(&builder,'_');
        }
        if (decl->isOperator) {
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(&builder,"operator_");
            }
            if (status==Status_Ok) {
                status=kek_sema_WriteOperatorName(&builder,decl->operatorCode);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(&builder,"_");
            }
            if (status==Status_Ok) {
                status=std_format_FormatU64ToBuilder(&builder,((u64)(decl->paramCount)),10);
            }
        } else {
            if (status==Status_Ok) {
                status=kek_syntax_WriteToken(program,&builder,decl->fileIndex,decl->nameIndex);
            }
        }
        if (status==Status_Ok) {
            status=kek_syntax_DetachBuilder(&builder,out);
        }
        StringBuilder_Destroy(&builder);
        return (status);
    }
    status=kek_sema_WriteDeclCName(program,&builder,decl);
    for (usize i=0;i<args->len;i++) {
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(&builder,"__");
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&args->data[i].key));
        }
    }
    if (status==Status_Ok) {
        status=kek_syntax_DetachBuilder(&builder,out);
    }
    StringBuilder_Destroy(&builder);
    return (status);
}
Status kek_sema_AddFuncUse(struct CompilerContext* program,struct ModuleDecl* decl,struct Array__TypeInfo* args) {
    if (args->len==0) {
        return (Status_Ok);
    }
    for (usize i=0;i<args->len;i++) {
        if (!kek_sema_TypeInfoConcrete(program,&args->data[i])) {
            return (Status_Ok);
        }
    }
    struct OwnedString cName={0};
    Status status=kek_sema_BuildGenericFuncCName(program,decl,args,&cName);
    if (status!=Status_Ok) {
        return (status);
    }
    usize declIndex=kek_sema_ModuleDeclIndex(program,decl);
    struct String key=std_string_OwnedStringView(&cName);
    if (kek_sema_FuncUseExists(program,declIndex,key)) {
        std_string_DestroyOwnedString(&cName);
        return (Status_Ok);
    }
    status=kek_syntax_ReserveFuncUses(program,1);
    if (status!=Status_Ok) {
        std_string_DestroyOwnedString(&cName);
        return (status);
    }
    struct FuncUse* use=&program->funcUses.data[program->funcUses.len];
    use->declIndex=declIndex;
    status=kek_syntax_CloneContextString(program,key,&use->key);
    if (status==Status_Ok) {
        use->cName=cName;
    } else {
        std_string_DestroyOwnedString(&cName);
    }
    use->args=std_array_ArrayNew__OwnedString(program->allocator);
    for (usize i=0;i<args->len&&status==Status_Ok;i++) {
        status=kek_sema_OwnedStringArrayPushClone(program,&use->args,std_string_OwnedStringView(&args->data[i].key));
    }
    if (status!=Status_Ok) {
        for (usize i=0;i<use->args.len;i++) {
            std_string_DestroyOwnedString(&use->args.data[i]);
        }
        struct Array__OwnedString useArgs=use->args;
        Array__OwnedString_Destroy(&useArgs);
        use->args=useArgs;
    }
    use->emitted=0;
    if (status==Status_Ok) {
        program->funcUses.len+=1;
    }
    return (status);
}
Status kek_sema_CollectGenericFunctionUsesInRange(struct CompilerContext* program,usize fileIndex,usize start,usize end) {
    usize i=start;
    while (i<end) {
        if (i+2<end&&kek_syntax_IsOperator(program,fileIndex,i+1,OperatorKind_Less)) {
            usize genericEnd=kek_module_FindMatching(program,fileIndex,i+1);
            if (genericEnd+1<end&&kek_syntax_IsPunctuation(program,fileIndex,genericEnd+1,PunctuationKind_LeftParen)) {
                struct String name=kek_syntax_TokenText(program,fileIndex,i);
                struct String scopeName=std_string_StringFromCString("");
                if (i>=2&&kek_syntax_IsOperator(program,fileIndex,i-1,OperatorKind_Scope)) {
                    scopeName=kek_syntax_TokenText(program,fileIndex,i-2);
                }
                struct ModuleDecl* decl=kek_sema_FindFunctionDeclByName(program,name,scopeName);
                if (decl!=0&&decl->isGeneric) {
                    struct Array__TypeInfo args=std_array_ArrayNew__TypeInfo(program->allocator);
                    usize argStart=i+2;
                    Status status=Status_Ok;
                    while (argStart<genericEnd) {
                        usize argEnd=kek_module_FindTokenAtDepthZero(program,fileIndex,argStart,genericEnd,PunctuationKind_Comma);
                        struct TypeInfo argInfo={0};
                        status=kek_sema_RenderTypeInfo(program,fileIndex,argStart,argEnd,&argInfo);
                        if (status==Status_Ok) {
                            status=Array__TypeInfo_Push(&args,argInfo);
                        }
                        if (status!=Status_Ok) {
                            kek_sema_TypeInfoDestroy(&argInfo);
                        }
                        if (status!=Status_Ok||argEnd>=genericEnd) {
                            break;
                        }
                        argStart=argEnd+1;
                    }
                    if (status==Status_Ok) {
                        status=kek_sema_AddFuncUse(program,decl,&args);
                    }
                    for (usize argIndex=0;argIndex<args.len;argIndex++) {
                        kek_sema_TypeInfoDestroy(&args.data[argIndex]);
                    }
                    Array__TypeInfo_Destroy(&args);
                    if (status!=Status_Ok) {
                        return (status);
                    }
                }
                i=genericEnd+1;
                continue;
            }
        }
        i+=1;
    }
    return (Status_Ok);
}
struct ModuleDecl* kek_sema_FindGenericMethodDecl(struct CompilerContext* program,struct String receiverBase,str methodName) {
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if (decl->kind!=DeclKind_Function||!decl->hasReceiver||!decl->isGeneric||decl->isOperator) {
            continue;
        }
        struct String base=kek_syntax_TokenText(program,decl->fileIndex,decl->receiverStart);
        if (!String_Equals(&base,receiverBase)) {
            continue;
        }
        struct String name=kek_syntax_TokenText(program,decl->fileIndex,decl->nameIndex);
        if (String_EqualsCString(&name,methodName)) {
            return (decl);
        }
    }
    return (0);
}
Status kek_sema_AddGenericMethodUseByName(struct CompilerContext* program,struct TypeUse* typeUse,str methodName) {
    struct String baseName=std_string_OwnedStringView(&typeUse->baseName);
    struct ModuleDecl* decl=kek_sema_FindGenericMethodDecl(program,baseName,methodName);
    if (decl==0) {
        return (Status_Ok);
    }
    struct Array__TypeInfo args=std_array_ArrayNew__TypeInfo(program->allocator);
    Status status=Status_Ok;
    if (typeUse->args.len>=1) {
        struct TypeInfo argInfo={0};
        status=kek_sema_TypeInfoFromKey(program,std_string_OwnedStringView(&typeUse->args.data[0]),&argInfo);
        if (status==Status_Ok) {
            status=Array__TypeInfo_Push(&args,argInfo);
        }
        if (status!=Status_Ok) {
            kek_sema_TypeInfoDestroy(&argInfo);
        }
    }
    if (status==Status_Ok) {
        status=kek_sema_AddFuncUse(program,decl,&args);
    }
    for (usize i=0;i<args.len;i++) {
        kek_sema_TypeInfoDestroy(&args.data[i]);
    }
    Array__TypeInfo_Destroy(&args);
    return (status);
}
bool kek_sema_MethodCallNameUsed(struct CompilerContext* program,str methodName) {
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if (!decl->reachable) {
            continue;
        }
        for (usize j=decl->start;j+2<decl->end;j++) {
            if (!kek_syntax_IsPunctuation(program,decl->fileIndex,j,PunctuationKind_Dot)) {
                continue;
            }
            if (!kek_syntax_TokenEquals(program,decl->fileIndex,j+1,methodName)) {
                continue;
            }
            if (kek_syntax_IsPunctuation(program,decl->fileIndex,j+2,PunctuationKind_LeftParen)) {
                return (1);
            }
        }
    }
    return (0);
}
Status kek_sema_CollectGenericCollectionMethods(struct CompilerContext* program) {
    bool arrayDestroyUsed=kek_sema_MethodCallNameUsed(program,"Destroy");
    bool arrayClearUsed=kek_sema_MethodCallNameUsed(program,"Clear");
    bool arrayReserveUsed=kek_sema_MethodCallNameUsed(program,"Reserve");
    bool arrayAppendSliceUsed=kek_sema_MethodCallNameUsed(program,"AppendSlice");
    bool arrayPushUsed=kek_sema_MethodCallNameUsed(program,"Push");
    bool arrayPushZeroedUsed=kek_sema_MethodCallNameUsed(program,"PushZeroed");
    bool arrayPopUsed=kek_sema_MethodCallNameUsed(program,"Pop");
    bool arrayGetUsed=kek_sema_MethodCallNameUsed(program,"Get");
    bool arraySetUsed=kek_sema_MethodCallNameUsed(program,"Set");
    bool arrayGetPtrUsed=kek_sema_MethodCallNameUsed(program,"GetPtr");
    bool arrayLastPtrUsed=kek_sema_MethodCallNameUsed(program,"LastPtr");
    bool arraySpanUsed=kek_sema_MethodCallNameUsed(program,"Span");
    bool arraySliceUsed=kek_sema_MethodCallNameUsed(program,"Slice");
    usize typeUseLimit=program->typeUses.len;
    for (usize i=0;i<typeUseLimit;i++) {
        struct TypeUse useValue=program->typeUses.data[i];
        struct TypeUse* use=&useValue;
        struct String base=std_string_OwnedStringView(&use->baseName);
        if (use->args.len<1) {
            continue;
        }
        Status status=Status_Ok;
        if (String_EqualsCString(&base,"Array")) {
            struct String firstArg=kek_sema_OwnedStringArrayGet(&use->args,0);
            if (status==Status_Ok&&(arrayPopUsed||arrayGetUsed)) {
                status=kek_sema_AddTypeUseFromBaseArg(program,"Result",firstArg);
            }
            if (status==Status_Ok&&(arrayAppendSliceUsed||arraySliceUsed)) {
                status=kek_sema_AddTypeUseFromBaseArg(program,"Slice",firstArg);
            }
            if (status==Status_Ok&&arraySpanUsed) {
                status=kek_sema_AddTypeUseFromBaseArg(program,"Span",firstArg);
            }
            if (status==Status_Ok&&arrayDestroyUsed) {
                status=kek_sema_AddGenericMethodUseByName(program,use,"Destroy");
            }
            if (status==Status_Ok&&arrayClearUsed) {
                status=kek_sema_AddGenericMethodUseByName(program,use,"Clear");
            }
            if (status==Status_Ok&&(arrayReserveUsed||arrayAppendSliceUsed||arrayPushUsed||arrayPushZeroedUsed)) {
                status=kek_sema_AddGenericMethodUseByName(program,use,"Reserve");
            }
            if (status==Status_Ok&&arrayAppendSliceUsed) {
                status=kek_sema_AddGenericMethodUseByName(program,use,"AppendSlice");
            }
            if (status==Status_Ok&&arrayPushUsed) {
                status=kek_sema_AddGenericMethodUseByName(program,use,"Push");
            }
            if (status==Status_Ok&&arrayPushZeroedUsed) {
                status=kek_sema_AddGenericMethodUseByName(program,use,"PushZeroed");
            }
            if (status==Status_Ok&&arrayPopUsed) {
                status=kek_sema_AddGenericMethodUseByName(program,use,"Pop");
            }
            if (status==Status_Ok&&arrayGetUsed) {
                status=kek_sema_AddGenericMethodUseByName(program,use,"Get");
            }
            if (status==Status_Ok&&arraySetUsed) {
                status=kek_sema_AddGenericMethodUseByName(program,use,"Set");
            }
            if (status==Status_Ok&&arrayGetPtrUsed) {
                status=kek_sema_AddGenericMethodUseByName(program,use,"GetPtr");
            }
            if (status==Status_Ok&&arrayLastPtrUsed) {
                status=kek_sema_AddGenericMethodUseByName(program,use,"LastPtr");
            }
            if (status==Status_Ok&&arraySpanUsed) {
                status=kek_sema_AddGenericMethodUseByName(program,use,"Span");
            }
            if (status==Status_Ok&&arraySliceUsed) {
                status=kek_sema_AddGenericMethodUseByName(program,use,"Slice");
            }
        } else {
            if (String_EqualsCString(&base,"LinkedList")) {
                struct String firstArg=kek_sema_OwnedStringArrayGet(&use->args,0);
                status=kek_sema_AddTypeUseFromBaseArg(program,"ListNode",firstArg);
                if (status==Status_Ok) {
                    status=kek_sema_AddTypeUseFromBaseArg(program,"Result",firstArg);
                }
                if (status==Status_Ok) {
                    status=kek_sema_AddGenericMethodUseByName(program,use,"PushBack");
                }
                if (status==Status_Ok) {
                    status=kek_sema_AddGenericMethodUseByName(program,use,"PushFront");
                }
                if (status==Status_Ok) {
                    status=kek_sema_AddGenericMethodUseByName(program,use,"PopBack");
                }
                if (status==Status_Ok) {
                    status=kek_sema_AddGenericMethodUseByName(program,use,"Destroy");
                }
                if (status==Status_Ok) {
                    status=kek_sema_AddGenericMethodUseByName(program,use,"PopFront");
                }
            }
        }
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
Status kek_sema_CollectGenericFunctionUses(struct CompilerContext* program) {
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if (!decl->reachable) {
            continue;
        }
        Status status=kek_sema_CollectGenericFunctionUsesInRange(program,decl->fileIndex,decl->start,decl->end);
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (kek_sema_CollectGenericCollectionMethods(program));
}
Status kek_sema_CollectTypeUses(struct CompilerContext* program) {
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if (!decl->reachable) {
            continue;
        }
        Status status=kek_sema_CollectTypeUsesInRange(program,decl->fileIndex,decl->start,decl->end);
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
Status kek_sema_CheckModule(struct CompilerContext* program) {
    Status status=kek_sema_BuildDeclIndexes(program);
    if (status==Status_Ok) {
        status=kek_sema_MarkReachableDecls(program);
    }
    if (status==Status_Ok) {
        status=kek_sema_CollectTypeUses(program);
    }
    if (status==Status_Ok) {
        status=kek_sema_CollectGenericFunctionUses(program);
    }
    return (status);
}
usize kek_module_FindMatching(struct CompilerContext* program,usize fileIndex,usize openIndex) {
    usize count=kek_syntax_FileTokenCount(program,fileIndex);
    usize depth=0;
    bool isParen=kek_syntax_IsPunctuation(program,fileIndex,openIndex,PunctuationKind_LeftParen);
    bool isBrace=kek_syntax_IsPunctuation(program,fileIndex,openIndex,PunctuationKind_LeftBrace);
    bool isBracket=kek_syntax_IsPunctuation(program,fileIndex,openIndex,PunctuationKind_LeftBracket);
    bool isGeneric=kek_syntax_IsOperator(program,fileIndex,openIndex,OperatorKind_Less);
    for (usize i=openIndex;i<count;i++) {
        if ((isParen&&kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftParen))||(isBrace&&kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftBrace))||(isBracket&&kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftBracket))||(isGeneric&&kek_syntax_IsOperator(program,fileIndex,i,OperatorKind_Less))) {
            depth+=1;
            continue;
        }
        if ((isParen&&kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_RightParen))||(isBrace&&kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_RightBrace))||(isBracket&&kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_RightBracket))||(isGeneric&&kek_syntax_IsOperator(program,fileIndex,i,OperatorKind_Greater))) {
            if (depth==0) {
                return (i);
            }
            depth-=1;
            if (depth==0) {
                return (i);
            }
        }
    }
    return (count);
}
usize kek_module_SkipDelimited(struct CompilerContext* program,usize fileIndex,usize index) {
    usize match=kek_module_FindMatching(program,fileIndex,index);
    if (match>=kek_syntax_FileTokenCount(program,fileIndex)) {
        return (index+1);
    }
    return (match+1);
}
usize kek_module_SkipAttributes(struct CompilerContext* program,usize fileIndex,usize index) {
    while (index<kek_syntax_FileTokenCount(program,fileIndex)&&kek_syntax_IsPunctuation(program,fileIndex,index,PunctuationKind_LeftBracket)) {
        index=kek_module_SkipDelimited(program,fileIndex,index);
    }
    return (index);
}
usize kek_module_FindTopLevelColon(struct CompilerContext* program,usize fileIndex,usize start,usize end) {
    usize parenDepth=0;
    usize braceDepth=0;
    usize bracketDepth=0;
    usize genericDepth=0;
    for (usize i=start;i<end;i++) {
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftParen)) {
            parenDepth+=1;
            continue;
        }
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_RightParen)) {
            if (parenDepth>0) {
                parenDepth-=1;
            }
            continue;
        }
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftBrace)) {
            braceDepth+=1;
            continue;
        }
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_RightBrace)) {
            if (braceDepth>0) {
                braceDepth-=1;
            }
            continue;
        }
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftBracket)) {
            bracketDepth+=1;
            continue;
        }
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_RightBracket)) {
            if (bracketDepth>0) {
                bracketDepth-=1;
            }
            continue;
        }
        if (kek_syntax_IsOperator(program,fileIndex,i,OperatorKind_Less)) {
            genericDepth+=1;
            continue;
        }
        if (kek_syntax_IsOperator(program,fileIndex,i,OperatorKind_Greater)) {
            if (genericDepth>0) {
                genericDepth-=1;
            }
            continue;
        }
        if (parenDepth==0&&braceDepth==0&&bracketDepth==0&&genericDepth==0&&kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_Colon)) {
            return (i);
        }
    }
    return (end);
}
usize kek_module_FindTokenAtDepthZero(struct CompilerContext* program,usize fileIndex,usize start,usize end,PunctuationKind punctuation) {
    usize parenDepth=0;
    usize braceDepth=0;
    usize bracketDepth=0;
    usize genericDepth=0;
    for (usize i=start;i<end;i++) {
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftParen)) {
            parenDepth+=1;
            continue;
        }
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_RightParen)) {
            if (parenDepth>0) {
                parenDepth-=1;
            }
            continue;
        }
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftBrace)) {
            braceDepth+=1;
            continue;
        }
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_RightBrace)) {
            if (braceDepth>0) {
                braceDepth-=1;
            }
            continue;
        }
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftBracket)) {
            bracketDepth+=1;
            continue;
        }
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_RightBracket)) {
            if (bracketDepth>0) {
                bracketDepth-=1;
            }
            continue;
        }
        if (parenDepth==0&&braceDepth==0&&bracketDepth==0&&genericDepth==0&&kek_syntax_IsPunctuation(program,fileIndex,i,punctuation)) {
            return (i);
        }
    }
    return (end);
}
usize kek_module_FindOperatorScope(struct CompilerContext* program,usize fileIndex,usize start,usize end) {
    usize genericDepth=0;
    for (usize i=start;i<end;i++) {
        if (kek_syntax_IsOperator(program,fileIndex,i,OperatorKind_Less)) {
            genericDepth+=1;
            continue;
        }
        if (kek_syntax_IsOperator(program,fileIndex,i,OperatorKind_Greater)) {
            if (genericDepth>0) {
                genericDepth-=1;
            }
            continue;
        }
        if (genericDepth==0&&kek_syntax_IsOperator(program,fileIndex,i,OperatorKind_Scope)) {
            return (i);
        }
    }
    return (end);
}
usize kek_module_FindNextGroup(struct CompilerContext* program,usize fileIndex,usize start,usize end) {
    for (usize i=start;i<end;i++) {
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftParen)) {
            return (i);
        }
    }
    return (end);
}
usize kek_module_FindNextBlock(struct CompilerContext* program,usize fileIndex,usize start,usize end) {
    for (usize i=start;i<end;i++) {
        if (kek_syntax_IsPunctuation(program,fileIndex,i,PunctuationKind_LeftBrace)) {
            return (i);
        }
    }
    return (end);
}
u8 kek_module_OperatorCode(struct CompilerContext* program,usize fileIndex,usize index) {
    struct Token token=program->tokenFiles.data[fileIndex].tokens[index];
    if (token.kind==TokenKind_Operator) {
        return (((u8)(token.subkind+1)));
    }
    return (0);
}
Status kek_module_AddDecl(struct CompilerContext* program,struct ModuleDecl* decl) {
    Status status=kek_syntax_ReserveDecls(program,1);
    if (status==Status_Ok) {
        program->decls.data[program->decls.len]=decl[0];
        program->decls.len+=1;
    }
    return (status);
}
Status kek_module_AddParam(struct CompilerContext* program,struct ModuleParam* param) {
    Status status=kek_syntax_ReserveParams(program,1);
    if (status==Status_Ok) {
        program->params.data[program->params.len]=param[0];
        program->params.len+=1;
    }
    return (status);
}
Status kek_module_AddField(struct CompilerContext* program,struct ModuleField* field) {
    Status status=kek_syntax_ReserveFields(program,1);
    if (status==Status_Ok) {
        program->fields.data[program->fields.len]=field[0];
        program->fields.len+=1;
    }
    return (status);
}
Status kek_module_ModuleError(struct CompilerContext* program,str message) {
    Status status=kek_syntax_AddDiagnostic(program,message);
    if (status!=Status_Ok) {
        return (status);
    }
    return (Status_Invalid);
}
void kek_module_KeepFirstStatus(Status* first,Status status) {
    if (first[0]==Status_Ok&&status!=Status_Ok) {
        first[0]=status;
    }
}
void kek_module_KeepError(struct CompilerContext* program,Status* first,str message) {
    kek_module_KeepFirstStatus(first,kek_module_ModuleError(program,message));
}
bool kek_module_DeclHasFieldName(struct CompilerContext* program,struct ModuleDecl* decl,usize fileIndex,usize nameIndex) {
    struct String name=kek_syntax_TokenText(program,fileIndex,nameIndex);
    for (usize i=0;i<decl->fieldCount;i++) {
        struct ModuleField* field=&program->fields.data[decl->firstField+i];
        struct String fieldName=kek_syntax_TokenText(program,field->fileIndex,field->nameIndex);
        if (String_Equals(&fieldName,name)) {
            return (1);
        }
    }
    return (0);
}
void kek_module_InitNestedStructDecl(struct ModuleDecl* nested,struct ModuleField* field,usize firstField,struct ModuleDecl* parent) {
    nested[0]=parent[0];
    nested->kind=DeclKind_Struct;
    nested->fileIndex=field->fileIndex;
    nested->start=field->nestedBodyStart;
    nested->end=field->nestedBodyEnd;
    nested->bodyStart=field->nestedBodyStart;
    nested->bodyEnd=field->nestedBodyEnd;
    nested->firstField=firstField;
    nested->fieldCount=field->nestedFieldCount;
    nested->nameIndex=field->nameIndex;
    nested->emitted=0;
    nested->reachable=1;
    nested->isTagged=0;
}
bool kek_module_AttributeListHasTagged(struct CompilerContext* program,usize fileIndex,usize start,usize end) {
    for (usize i=start;i<end;i++) {
        if (kek_syntax_IsKeyword(program,fileIndex,i,KeywordKind_Tagged)||kek_syntax_IsIdentifierText(program,fileIndex,i,"tagged")) {
            return (1);
        }
    }
    return (0);
}
bool kek_module_DeclHasTaggedAttribute(struct CompilerContext* program,usize fileIndex,usize start) {
    usize index=start;
    while (index<kek_syntax_FileTokenCount(program,fileIndex)&&kek_syntax_IsPunctuation(program,fileIndex,index,PunctuationKind_LeftBracket)) {
        usize end=kek_module_FindMatching(program,fileIndex,index);
        if (end>=kek_syntax_FileTokenCount(program,fileIndex)) {
            return (0);
        }
        if (kek_module_AttributeListHasTagged(program,fileIndex,index+1,end)) {
            return (1);
        }
        index=end+1;
    }
    return (0);
}
Status kek_module_ParseParams(struct CompilerContext* program,struct ModuleDecl* decl,usize fileIndex,usize start,usize end) {
    decl->firstParam=program->params.len;
    decl->paramCount=0;
    usize itemStart=start;
    while (itemStart<end) {
        if (kek_syntax_IsPunctuation(program,fileIndex,itemStart,PunctuationKind_Comma)) {
            itemStart+=1;
            continue;
        }
        usize itemEnd=kek_module_FindTokenAtDepthZero(program,fileIndex,itemStart,end,PunctuationKind_Comma);
        usize colon=kek_module_FindTopLevelColon(program,fileIndex,itemStart,itemEnd);
        if (colon>=itemEnd) {
            break;
        }
        struct ModuleParam param={0};
        param.fileIndex=fileIndex;
        param.typeStart=itemStart;
        param.typeEnd=colon;
        param.nameIndex=colon+1;
        param.defaultStart=itemEnd;
        param.defaultEnd=itemEnd;
        param.hasDefault=0;
        for (usize i=param.nameIndex+1;i<itemEnd;i++) {
            if (kek_syntax_IsOperator(program,fileIndex,i,OperatorKind_Assign)) {
                param.defaultStart=i+1;
                param.defaultEnd=itemEnd;
                param.hasDefault=1;
                break;
            }
        }
        Status status=kek_module_AddParam(program,&param);
        if (status!=Status_Ok) {
            return (status);
        }
        decl->paramCount+=1;
        if (itemEnd>=end) {
            break;
        }
        itemStart=itemEnd+1;
    }
    return (Status_Ok);
}
Status kek_module_ParseFields(struct CompilerContext* program,struct ModuleDecl* decl,usize fileIndex,usize start,usize end) {
    decl->firstField=program->fields.len;
    decl->fieldCount=0;
    usize itemStart=start;
    Status firstStatus=Status_Ok;
    while (itemStart<end) {
        itemStart=kek_module_SkipAttributes(program,fileIndex,itemStart);
        if (itemStart>=end) {
            break;
        }
        if (kek_syntax_IsPunctuation(program,fileIndex,itemStart,PunctuationKind_Semicolon)) {
            itemStart+=1;
            continue;
        }
        struct ModuleField field={0};
        field.fileIndex=fileIndex;
        field.typeStart=itemStart;
        field.typeEnd=itemStart;
        field.nameIndex=itemStart;
        field.defaultStart=end;
        field.defaultEnd=end;
        field.arrayStart=end;
        field.arrayEnd=end;
        field.hasDefault=0;
        field.isArray=0;
        field.isNestedStruct=0;
        field.nestedBodyStart=end;
        field.nestedBodyEnd=end;
        field.nestedFirstField=0;
        field.nestedFieldCount=0;
        if (kek_syntax_IsKeyword(program,fileIndex,itemStart,KeywordKind_Struct)) {
            usize colon=itemStart+1;
            if (colon<end&&kek_syntax_IsPunctuation(program,fileIndex,colon,PunctuationKind_Colon)) {
                usize nameIndex=colon+1;
                usize blockStart=kek_module_FindNextBlock(program,fileIndex,nameIndex+1,end);
                if (nameIndex>=end||!kek_syntax_IsTokenKind(program,fileIndex,nameIndex,TokenKind_Identifier)) {
                    kek_module_KeepError(program,&firstStatus,"expected nested struct field name");
                }
                if (kek_module_DeclHasFieldName(program,decl,fileIndex,nameIndex)) {
                    kek_module_KeepError(program,&firstStatus,"duplicate struct field");
                }
                if (blockStart>=end) {
                    kek_module_KeepError(program,&firstStatus,"expected nested struct body");
                    break;
                }
                usize blockEnd=kek_module_FindMatching(program,fileIndex,blockStart);
                if (blockEnd>=kek_syntax_FileTokenCount(program,fileIndex)) {
                    kek_module_KeepError(program,&firstStatus,"unterminated nested struct body");
                    break;
                }
                field.isNestedStruct=1;
                field.nameIndex=nameIndex;
                field.nestedBodyStart=blockStart+1;
                field.nestedBodyEnd=blockEnd;
                Status addNested=kek_module_AddField(program,&field);
                if (addNested!=Status_Ok) {
                    return (addNested);
                }
                decl->fieldCount+=1;
                itemStart=blockEnd+1;
                if (itemStart<end&&kek_syntax_IsPunctuation(program,fileIndex,itemStart,PunctuationKind_Semicolon)) {
                    itemStart+=1;
                }
                continue;
            }
        }
        usize itemEnd=itemStart;
        while (itemEnd<end&&!kek_syntax_IsPunctuation(program,fileIndex,itemEnd,PunctuationKind_Semicolon)) {
            if (kek_syntax_IsPunctuation(program,fileIndex,itemEnd,PunctuationKind_LeftBrace)||kek_syntax_IsPunctuation(program,fileIndex,itemEnd,PunctuationKind_LeftParen)||kek_syntax_IsPunctuation(program,fileIndex,itemEnd,PunctuationKind_LeftBracket)) {
                itemEnd=kek_module_SkipDelimited(program,fileIndex,itemEnd);
                continue;
            }
            itemEnd+=1;
        }
        usize colon=kek_module_FindTopLevelColon(program,fileIndex,itemStart,itemEnd);
        if (colon>=itemEnd) {
            kek_module_KeepError(program,&firstStatus,"expected ':' in struct field");
            itemStart=itemEnd+1;
            continue;
        }
        field.typeStart=itemStart;
        field.typeEnd=colon;
        field.nameIndex=colon+1;
        if (field.typeStart>=field.typeEnd) {
            kek_module_KeepError(program,&firstStatus,"expected struct field type");
        }
        if (field.nameIndex>=itemEnd||!kek_syntax_IsTokenKind(program,fileIndex,field.nameIndex,TokenKind_Identifier)) {
            kek_module_KeepError(program,&firstStatus,"expected struct field name");
        }
        if (kek_module_DeclHasFieldName(program,decl,fileIndex,field.nameIndex)) {
            kek_module_KeepError(program,&firstStatus,"duplicate struct field");
        }
        usize afterName=field.nameIndex+1;
        if (afterName<itemEnd&&kek_syntax_IsPunctuation(program,fileIndex,afterName,PunctuationKind_LeftBracket)) {
            field.isArray=1;
            field.arrayStart=afterName+1;
            field.arrayEnd=kek_module_FindMatching(program,fileIndex,afterName);
            afterName=field.arrayEnd+1;
        }
        for (usize i=afterName;i<itemEnd;i++) {
            if (kek_syntax_IsOperator(program,fileIndex,i,OperatorKind_Assign)) {
                field.hasDefault=1;
                field.defaultStart=i+1;
                field.defaultEnd=itemEnd;
                break;
            }
        }
        Status status=kek_module_AddField(program,&field);
        if (status!=Status_Ok) {
            return (status);
        }
        decl->fieldCount+=1;
        itemStart=itemEnd+1;
    }
    for (usize i=0;i<decl->fieldCount;i++) {
        usize fieldIndex=decl->firstField+i;
        struct ModuleField* field=&program->fields.data[fieldIndex];
        if (!field->isNestedStruct||field->nestedFieldCount!=0) {
            continue;
        }
        struct ModuleDecl nested={0};
        kek_module_InitNestedStructDecl(&nested,field,program->fields.len,decl);
        Status parseNested=kek_module_ParseFields(program,&nested,field->fileIndex,field->nestedBodyStart,field->nestedBodyEnd);
        kek_module_KeepFirstStatus(&firstStatus,parseNested);
        program->fields.data[fieldIndex].nestedFirstField=nested.firstField;
        program->fields.data[fieldIndex].nestedFieldCount=nested.fieldCount;
    }
    return (firstStatus);
}
bool kek_module_ModuleDeclNameMatches(struct CompilerContext* program,struct ModuleDecl* decl,struct String name) {
    if (decl->nameIndex>=kek_syntax_FileTokenCount(program,decl->fileIndex)) {
        return (0);
    }
    struct String declName=kek_syntax_TokenText(program,decl->fileIndex,decl->nameIndex);
    return (String_Equals(&declName,name));
}
struct ModuleDecl* kek_module_FindTypeDecl(struct CompilerContext* program,struct String name) {
    if (program->typeDeclIndex.len>0) {
        usize low=0;
        usize high=program->typeDeclIndex.len;
        while (low<high) {
            usize mid=low+(high-low)/2;
            struct String midName=std_string_OwnedStringView(&program->typeDeclIndex.data[mid].name);
            int compare=String_Compare(&midName,name);
            if (compare<0) {
                low=mid+1;
                continue;
            }
            if (compare>0) {
                high=mid;
                continue;
            }
            while (mid>0) {
                struct String prevName=std_string_OwnedStringView(&program->typeDeclIndex.data[mid-1].name);
                if (!String_Equals(&prevName,name)) {
                    break;
                }
                mid-=1;
            }
            return (&program->decls.data[program->typeDeclIndex.data[mid].declIndex]);
        }
    }
    for (usize i=0;i<program->decls.len;i++) {
        struct ModuleDecl* decl=&program->decls.data[i];
        if ((decl->kind==DeclKind_Struct||decl->kind==DeclKind_Union||decl->kind==DeclKind_Enum||decl->kind==DeclKind_Alias)&&kek_module_ModuleDeclNameMatches(program,decl,name)) {
            return (decl);
        }
    }
    return (0);
}
Status kek_module_ParseAliasDecl(struct CompilerContext* program,usize fileIndex,usize start,usize* outEnd) {
    usize count=kek_syntax_FileTokenCount(program,fileIndex);
    usize end=start;
    while (end<count&&!kek_syntax_IsPunctuation(program,fileIndex,end,PunctuationKind_Semicolon)&&!kek_syntax_IsEof(program,fileIndex,end)) {
        end+=1;
    }
    struct ModuleDecl decl={0};
    decl.kind=DeclKind_Alias;
    decl.fileIndex=fileIndex;
    decl.start=start;
    decl.end=end+1;
    decl.nameIndex=start+2;
    decl.returnStart=end;
    decl.returnEnd=end;
    decl.receiverStart=end;
    decl.receiverEnd=end;
    decl.hasReceiver=0;
    decl.isOperator=0;
    decl.operatorCode=0;
    decl.genericStart=end;
    decl.genericEnd=end;
    decl.isGeneric=0;
    decl.paramStart=end;
    decl.paramEnd=end;
    decl.bodyStart=end;
    decl.bodyEnd=end;
    decl.hasBody=0;
    decl.firstParam=0;
    decl.paramCount=0;
    decl.firstField=0;
    decl.fieldCount=0;
    decl.docStart=start;
    decl.docEnd=start;
    decl.hasDocComment=0;
    decl.emitted=0;
    decl.reachable=0;
    decl.isTagged=0;
    Status clone=kek_syntax_CloneContextString(program,std_string_OwnedStringView(&program->tokenFiles.data[fileIndex].packageName),&decl.packageName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    clone=kek_syntax_CloneContextString(program,std_string_OwnedStringView(&program->tokenFiles.data[fileIndex].moduleName),&decl.moduleName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    for (usize i=start+3;i<end;i++) {
        if (kek_syntax_IsOperator(program,fileIndex,i,OperatorKind_Assign)) {
            decl.returnStart=i+1;
            decl.returnEnd=end;
            break;
        }
    }
    Status status=kek_module_AddDecl(program,&decl);
    outEnd[0]=end+1;
    return (status);
}
Status kek_module_ParseTypeDecl(struct CompilerContext* program,usize fileIndex,usize start,DeclKind kind,bool isTagged,usize* outEnd) {
    usize count=kek_syntax_FileTokenCount(program,fileIndex);
    usize colon=start+1;
    usize nameIndex=colon+1;
    if (kind==DeclKind_Enum) {
        if (colon<count&&kek_syntax_IsPunctuation(program,fileIndex,colon,PunctuationKind_Colon)) {
            usize secondColon=kek_module_FindTopLevelColon(program,fileIndex,colon+1,count);
            if (secondColon<count) {
                nameIndex=secondColon+1;
            }
        }
    }
    usize genericStart=count;
    usize genericEnd=count;
    if (nameIndex+1<count&&kek_syntax_IsOperator(program,fileIndex,nameIndex+1,OperatorKind_Less)) {
        genericStart=nameIndex+1;
        genericEnd=kek_module_FindMatching(program,fileIndex,genericStart);
    }
    usize blockStart=kek_module_FindNextBlock(program,fileIndex,nameIndex+1,count);
    usize blockEnd=kek_module_FindMatching(program,fileIndex,blockStart);
    struct ModuleDecl decl={0};
    decl.kind=kind;
    decl.fileIndex=fileIndex;
    decl.start=start;
    decl.end=blockEnd+1;
    decl.nameIndex=nameIndex;
    decl.returnStart=start;
    decl.returnEnd=start;
    decl.receiverStart=start;
    decl.receiverEnd=start;
    decl.hasReceiver=0;
    decl.isOperator=0;
    decl.operatorCode=0;
    decl.genericStart=genericStart;
    decl.genericEnd=genericEnd;
    decl.isGeneric=genericStart<count;
    decl.paramStart=start;
    decl.paramEnd=start;
    decl.bodyStart=blockStart+1;
    decl.bodyEnd=blockEnd;
    decl.hasBody=1;
    decl.firstParam=0;
    decl.paramCount=0;
    decl.firstField=0;
    decl.fieldCount=0;
    decl.docStart=start;
    decl.docEnd=start;
    decl.hasDocComment=0;
    decl.emitted=0;
    decl.reachable=0;
    decl.isTagged=isTagged;
    Status clone=kek_syntax_CloneContextString(program,std_string_OwnedStringView(&program->tokenFiles.data[fileIndex].packageName),&decl.packageName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    clone=kek_syntax_CloneContextString(program,std_string_OwnedStringView(&program->tokenFiles.data[fileIndex].moduleName),&decl.moduleName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    if (kind==DeclKind_Struct||kind==DeclKind_Union) {
        Status fields=kek_module_ParseFields(program,&decl,fileIndex,blockStart+1,blockEnd);
        if (fields!=Status_Ok) {
            return (fields);
        }
    }
    Status status=kek_module_AddDecl(program,&decl);
    outEnd[0]=blockEnd+1;
    if (outEnd[0]<count&&kek_syntax_IsPunctuation(program,fileIndex,outEnd[0],PunctuationKind_Semicolon)) {
        outEnd[0]+=1;
    }
    return (status);
}
Status kek_module_ParseExternDecl(struct CompilerContext* program,usize fileIndex,usize start,usize* outEnd) {
    usize count=kek_syntax_FileTokenCount(program,fileIndex);
    usize blockStart=kek_module_FindNextBlock(program,fileIndex,start,count);
    usize blockEnd=kek_module_FindMatching(program,fileIndex,blockStart);
    struct ModuleDecl decl={0};
    decl.kind=DeclKind_ExternC;
    decl.fileIndex=fileIndex;
    decl.start=start;
    decl.end=blockEnd+1;
    decl.nameIndex=start;
    decl.returnStart=start;
    decl.returnEnd=start;
    decl.receiverStart=start;
    decl.receiverEnd=start;
    decl.hasReceiver=0;
    decl.isOperator=0;
    decl.operatorCode=0;
    decl.genericStart=start;
    decl.genericEnd=start;
    decl.isGeneric=0;
    decl.paramStart=start;
    decl.paramEnd=start;
    decl.bodyStart=blockStart+1;
    decl.bodyEnd=blockEnd;
    decl.hasBody=1;
    decl.firstParam=0;
    decl.paramCount=0;
    decl.firstField=0;
    decl.fieldCount=0;
    decl.docStart=start;
    decl.docEnd=start;
    decl.hasDocComment=0;
    decl.emitted=0;
    decl.reachable=0;
    decl.isTagged=0;
    Status clone=kek_syntax_CloneContextString(program,std_string_OwnedStringView(&program->tokenFiles.data[fileIndex].packageName),&decl.packageName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    clone=kek_syntax_CloneContextString(program,std_string_OwnedStringView(&program->tokenFiles.data[fileIndex].moduleName),&decl.moduleName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    Status status=kek_module_AddDecl(program,&decl);
    outEnd[0]=blockEnd+1;
    return (status);
}
Status kek_module_ParseFunctionDecl(struct CompilerContext* program,usize fileIndex,usize start,usize* outEnd) {
    usize count=kek_syntax_FileTokenCount(program,fileIndex);
    usize groupStart=kek_module_FindNextGroup(program,fileIndex,start,count);
    if (groupStart>=count) {
        outEnd[0]=start+1;
        return (Status_Ok);
    }
    usize groupEnd=kek_module_FindMatching(program,fileIndex,groupStart);
    usize colon=kek_module_FindTopLevelColon(program,fileIndex,start,groupStart);
    if (colon>=groupStart) {
        outEnd[0]=groupEnd+1;
        return (Status_Ok);
    }
    usize blockStart=groupEnd+1;
    bool hasBody=0;
    usize blockEnd=groupEnd;
    if (blockStart<count&&kek_syntax_IsPunctuation(program,fileIndex,blockStart,PunctuationKind_LeftBrace)) {
        hasBody=1;
        blockEnd=kek_module_FindMatching(program,fileIndex,blockStart);
    }
    struct ModuleDecl decl={0};
    decl.kind=DeclKind_Function;
    decl.fileIndex=fileIndex;
    decl.start=start;
    decl.end=blockEnd+1;
    decl.nameIndex=colon+1;
    decl.returnStart=start;
    decl.returnEnd=colon;
    decl.receiverStart=colon+1;
    decl.receiverEnd=colon+1;
    decl.hasReceiver=0;
    decl.isOperator=0;
    decl.operatorCode=0;
    decl.genericStart=count;
    decl.genericEnd=count;
    decl.isGeneric=0;
    decl.paramStart=groupStart+1;
    decl.paramEnd=groupEnd;
    decl.bodyStart=blockStart+1;
    decl.bodyEnd=blockEnd;
    decl.hasBody=hasBody;
    decl.firstParam=0;
    decl.paramCount=0;
    decl.firstField=0;
    decl.fieldCount=0;
    decl.docStart=start;
    decl.docEnd=start;
    decl.hasDocComment=0;
    decl.emitted=0;
    decl.reachable=0;
    decl.isTagged=0;
    Status clone=kek_syntax_CloneContextString(program,std_string_OwnedStringView(&program->tokenFiles.data[fileIndex].packageName),&decl.packageName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    clone=kek_syntax_CloneContextString(program,std_string_OwnedStringView(&program->tokenFiles.data[fileIndex].moduleName),&decl.moduleName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    usize scope=kek_module_FindOperatorScope(program,fileIndex,colon+1,groupStart);
    if (scope<groupStart) {
        decl.hasReceiver=1;
        decl.receiverStart=colon+1;
        decl.receiverEnd=scope;
        decl.nameIndex=scope+1;
    }
    if (decl.nameIndex<groupStart&&kek_syntax_IsIdentifierText(program,fileIndex,decl.nameIndex,"operator")) {
        decl.isOperator=1;
        decl.operatorCode=kek_module_OperatorCode(program,fileIndex,decl.nameIndex+1);
    }
    usize genericProbe=decl.nameIndex+1;
    if (decl.isOperator) {
        genericProbe=decl.nameIndex+2;
    }
    if (genericProbe<groupStart&&kek_syntax_IsOperator(program,fileIndex,genericProbe,OperatorKind_Less)) {
        decl.genericStart=genericProbe;
        decl.genericEnd=kek_module_FindMatching(program,fileIndex,genericProbe);
        decl.isGeneric=1;
    }
    for (usize i=decl.receiverStart;i<decl.receiverEnd;i++) {
        if (kek_syntax_IsOperator(program,fileIndex,i,OperatorKind_Less)) {
            decl.isGeneric=1;
        }
    }
    Status params=kek_module_ParseParams(program,&decl,fileIndex,groupStart+1,groupEnd);
    if (params!=Status_Ok) {
        return (params);
    }
    Status status=kek_module_AddDecl(program,&decl);
    outEnd[0]=blockEnd+1;
    if (outEnd[0]<count&&kek_syntax_IsPunctuation(program,fileIndex,outEnd[0],PunctuationKind_Semicolon)) {
        outEnd[0]+=1;
    }
    return (status);
}
Status kek_module_ParseDeclarations(struct CompilerContext* program) {
    for (usize fileIndex=0;fileIndex<program->tokenFiles.len;fileIndex++) {
        usize index=0;
        usize count=kek_syntax_FileTokenCount(program,fileIndex);
        usize docStart=count;
        usize docEnd=count;
        bool hasDocComment=0;
        while (index<count&&!kek_syntax_IsEof(program,fileIndex,index)) {
            while (index<count&&(kek_syntax_IsTokenKind(program,fileIndex,index,TokenKind_Comment)||kek_syntax_IsTokenKind(program,fileIndex,index,TokenKind_DocComment))) {
                if (kek_syntax_IsTokenKind(program,fileIndex,index,TokenKind_DocComment)) {
                    if (!hasDocComment) {
                        docStart=index;
                    }
                    docEnd=index+1;
                    hasDocComment=1;
                }
                index+=1;
            }
            bool isTagged=kek_module_DeclHasTaggedAttribute(program,fileIndex,index);
            index=kek_module_SkipAttributes(program,fileIndex,index);
            if (index>=count||kek_syntax_IsEof(program,fileIndex,index)) {
                break;
            }
            usize next=index+1;
            usize declCountBefore=program->decls.len;
            Status status=Status_Ok;
            if (kek_syntax_IsPunctuation(program,fileIndex,index,PunctuationKind_Hash)) {
                usize groupStart=kek_module_FindNextGroup(program,fileIndex,index,count);
                if (groupStart<count) {
                    next=kek_module_SkipDelimited(program,fileIndex,groupStart);
                    if (next<count&&kek_syntax_IsPunctuation(program,fileIndex,next,PunctuationKind_Semicolon)) {
                        next+=1;
                    }
                }
            } else {
                if (kek_syntax_IsKeyword(program,fileIndex,index,KeywordKind_Extern)) {
                    status=kek_module_ParseExternDecl(program,fileIndex,index,&next);
                } else {
                    if (kek_syntax_IsKeyword(program,fileIndex,index,KeywordKind_Alias)||kek_syntax_IsIdentifierText(program,fileIndex,index,"alias")) {
                        status=kek_module_ParseAliasDecl(program,fileIndex,index,&next);
                    } else {
                        if (kek_syntax_IsKeyword(program,fileIndex,index,KeywordKind_Struct)||kek_syntax_IsIdentifierText(program,fileIndex,index,"struct")) {
                            status=kek_module_ParseTypeDecl(program,fileIndex,index,DeclKind_Struct,0,&next);
                        } else {
                            if (kek_syntax_IsKeyword(program,fileIndex,index,KeywordKind_Union)||kek_syntax_IsIdentifierText(program,fileIndex,index,"union")) {
                                status=kek_module_ParseTypeDecl(program,fileIndex,index,DeclKind_Union,isTagged,&next);
                            } else {
                                if (kek_syntax_IsKeyword(program,fileIndex,index,KeywordKind_Enum)||kek_syntax_IsIdentifierText(program,fileIndex,index,"enum")) {
                                    status=kek_module_ParseTypeDecl(program,fileIndex,index,DeclKind_Enum,0,&next);
                                } else {
                                    status=kek_module_ParseFunctionDecl(program,fileIndex,index,&next);
                                }
                            }
                        }
                    }
                }
            }
            if (status!=Status_Ok) {
                return (status);
            }
            if (hasDocComment&&program->decls.len>declCountBefore) {
                struct ModuleDecl* decl=&program->decls.data[program->decls.len-1];
                decl->docStart=docStart;
                decl->docEnd=docEnd;
                decl->hasDocComment=1;
            }
            hasDocComment=0;
            docStart=count;
            docEnd=count;
            if (next<=index) {
                next=index+1;
            }
            index=next;
        }
    }
    return (Status_Ok);
}
Status kek_module_BuildModule(struct CompilerContext* program) {
    return (kek_module_ParseDeclarations(program));
}
struct String kek_syntax_TokenText(struct CompilerContext* program,usize fileIndex,usize tokenIndex) {
    struct SyntaxFile* file=&program->tokenFiles.data[fileIndex];
    struct Token token=file->tokens[tokenIndex];
    struct String sourceText=file->sourceText;
    return (String_Slice(&sourceText,token.offset,token.length));
}
bool kek_syntax_TokenEquals(struct CompilerContext* program,usize fileIndex,usize tokenIndex,str text) {
    struct String value=kek_syntax_TokenText(program,fileIndex,tokenIndex);
    return (String_EqualsCString(&value,text));
}
bool kek_syntax_IsTokenKind(struct CompilerContext* program,usize fileIndex,usize tokenIndex,TokenKind kind) {
    return (program->tokenFiles.data[fileIndex].tokens[tokenIndex].kind==kind);
}
bool kek_syntax_IsPunctuation(struct CompilerContext* program,usize fileIndex,usize tokenIndex,PunctuationKind kind) {
    struct Token token=program->tokenFiles.data[fileIndex].tokens[tokenIndex];
    return (token.payload.tag==TokenPayloadTag_Punctuation&&token.payload.data.Punctuation==kind);
}
bool kek_syntax_IsOperator(struct CompilerContext* program,usize fileIndex,usize tokenIndex,OperatorKind kind) {
    struct Token token=program->tokenFiles.data[fileIndex].tokens[tokenIndex];
    return (token.payload.tag==TokenPayloadTag_Operator&&token.payload.data.Operator==kind);
}
bool kek_syntax_IsKeyword(struct CompilerContext* program,usize fileIndex,usize tokenIndex,KeywordKind kind) {
    struct Token token=program->tokenFiles.data[fileIndex].tokens[tokenIndex];
    return (token.payload.tag==TokenPayloadTag_Keyword&&token.payload.data.Keyword==kind);
}
bool kek_syntax_IsIdentifierText(struct CompilerContext* program,usize fileIndex,usize tokenIndex,str text) {
    struct Token token=program->tokenFiles.data[fileIndex].tokens[tokenIndex];
    if (!(token.kind==TokenKind_Identifier||token.kind==TokenKind_Keyword)) {
        return (0);
    }
    return (kek_syntax_TokenEquals(program,fileIndex,tokenIndex,text));
}
bool kek_syntax_IsEof(struct CompilerContext* program,usize fileIndex,usize tokenIndex) {
    return (program->tokenFiles.data[fileIndex].tokens[tokenIndex].kind==TokenKind_Eof);
}
usize kek_syntax_FileTokenCount(struct CompilerContext* program,usize fileIndex) {
    return (program->tokenFiles.data[fileIndex].tokenLen);
}
Status kek_syntax_Write(struct StringBuilder* out,str text) {
    return (StringBuilder_WriteCString(out,text));
}
Status kek_syntax_WriteString(struct StringBuilder* out,struct String text) {
    return (StringBuilder_WriteString(out,text));
}
Status kek_syntax_WriteToken(struct CompilerContext* program,struct StringBuilder* out,usize fileIndex,usize tokenIndex) {
    return (StringBuilder_WriteString(out,kek_syntax_TokenText(program,fileIndex,tokenIndex)));
}
Status kek_syntax_CloneContextCString(struct CompilerContext* program,str text,struct OwnedString* out) {
    return (std_string_CloneCString(text,program->allocator,out));
}
Status kek_syntax_CloneContextString(struct CompilerContext* program,struct String text,struct OwnedString* out) {
    return (std_string_CloneString(text,program->allocator,out));
}
Status kek_syntax_DetachBuilder(struct StringBuilder* builder,struct OwnedString* out) {
    struct Result__OwnedString detached=StringBuilder_Detach(builder);
    if (detached.status!=Status_Ok) {
        return (detached.status);
    }
    out->data=detached.value.data;
    out->len=detached.value.len;
    out->cap=detached.value.cap;
    out->allocator=detached.value.allocator;
    return (Status_Ok);
}
Status kek_syntax_MakeOwnedEmpty(struct CompilerContext* program,struct OwnedString* out) {
    return (kek_syntax_CloneContextCString(program,"",out));
}
usize kek_syntax_StringLastSlash(struct String path) {
    usize last=path.len;
    for (usize i=0;i<path.len;i++) {
        if (path.data[i]=='/') {
            last=i;
        }
    }
    return (last);
}
Status kek_syntax_PackageNameFromPath(struct String path,struct Allocator allocator,struct OwnedString* out) {
    usize lastSlash=kek_syntax_StringLastSlash(path);
    if (lastSlash==path.len) {
        return (std_string_CloneCString("",allocator,out));
    }
    usize segmentStart=0;
    for (usize i=0;i<lastSlash;i++) {
        if (path.data[i]=='/') {
            segmentStart=i+1;
        }
    }
    return (std_string_CloneString(String_Slice(&path,segmentStart,lastSlash-segmentStart),allocator,out));
}
Status kek_syntax_ModuleNameFromPath(struct String path,struct Allocator allocator,struct OwnedString* out) {
    usize lastSlash=kek_syntax_StringLastSlash(path);
    if (lastSlash==path.len) {
        return (std_string_CloneCString("",allocator,out));
    }
    struct String name=String_Slice(&path,lastSlash+1,path.len-lastSlash-1);
    if (String_EndsWith(&name,std_string_StringFromCString(".kek"))) {
        return (std_string_CloneString(String_Slice(&name,0,name.len-4),allocator,out));
    }
    return (std_string_CloneString(name,allocator,out));
}
Status kek_syntax_InitTokenFileSlot(struct Allocator allocator,struct SyntaxFile* tokenFile,usize fileIndex) {
    tokenFile->tokens=0;
    tokenFile->tokenLen=0;
    tokenFile->tokenCap=0;
    tokenFile->sourceText=std_string_StringFromCString("");
    tokenFile->path=std_string_StringFromCString("");
    tokenFile->fileIndex=fileIndex;
    Status status=std_string_CloneCString("",allocator,&tokenFile->packageName);
    if (status==Status_Ok) {
        status=std_string_CloneCString("",allocator,&tokenFile->moduleName);
    }
    return (status);
}
Status kek_syntax_SetTokenFileNames(struct Allocator allocator,struct SyntaxFile* tokenFile,bool isRoot) {
    std_string_DestroyOwnedString(&tokenFile->packageName);
    std_string_DestroyOwnedString(&tokenFile->moduleName);
    if (isRoot) {
        Status status=std_string_CloneCString("",allocator,&tokenFile->packageName);
        if (status==Status_Ok) {
            status=std_string_CloneCString("",allocator,&tokenFile->moduleName);
        }
        return (status);
    }
    Status status=kek_syntax_PackageNameFromPath(tokenFile->path,allocator,&tokenFile->packageName);
    if (status==Status_Ok) {
        status=kek_syntax_ModuleNameFromPath(tokenFile->path,allocator,&tokenFile->moduleName);
    }
    return (status);
}
Status kek_syntax_TokenizeSourceFile(struct Allocator allocator,struct SourceFile* sourceFile,struct SyntaxFile* tokenFile,usize fileIndex,bool isRoot,struct DiagnosticBag* diagnostics) {
    tokenFile->sourceText=std_string_OwnedStringView(&sourceFile->content);
    tokenFile->path=std_string_OwnedStringView(&sourceFile->path);
    tokenFile->fileIndex=fileIndex;
    Status status=kek_syntax_SetTokenFileNames(allocator,tokenFile,isRoot);
    if (status!=Status_Ok) {
        return (status);
    }
    struct Array__Token tokens={0};
    status=kek_tokenizer_TokenizeToArray(tokenFile->sourceText,0,allocator,&tokens);
    if (status!=Status_Ok) {
        Array__Token_Destroy(&tokens);
        return (status);
    }
    tokenFile->tokens=tokens.data;
    tokenFile->tokenLen=tokens.len;
    tokenFile->tokenCap=tokens.cap;
    struct AstTree tree={0};
    status=kek_ast_ParseTokens(tokenFile->tokens,tokenFile->tokenLen,tokenFile->sourceText,((i64)(fileIndex)),allocator,diagnostics,&tree);
    if (status==Status_Ok) {
        kek_ast_AstTreeDestroy(&tree);
    }
    return (status);
}
void kek_syntax_TokenizeJobRun(struct TokenizeJob* job) {
    job->status=kek_syntax_TokenizeSourceFile(job->allocator,job->sourceFile,job->tokenFile,job->fileIndex,job->isRoot,&job->diagnostics);
}
ptr kek_syntax_TokenizeFileThreadEntry(ptr arg) {
    struct TokenizeJob* job=((struct TokenizeJob*)(arg));
    kek_syntax_TokenizeJobRun(job);
    return (0);
}
Status kek_syntax_MergeDiagnostics(struct DiagnosticBag* target,struct DiagnosticBag* source) {
    for (usize i=0;i<source->items.len;i++) {
        struct Diagnostic* item=&source->items.data[i];
        Status status=kek_diagnostics_DiagnosticAdd(target,item->severity,item->phase,item->fileIndex,item->location,std_string_OwnedStringView(&item->message));
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
Status kek_syntax_ProgramInit(struct CompilerContext* program,struct Allocator allocator) {
    program->allocator=allocator;
    kek_diagnostics_DiagnosticBagInit(&program->diagnostics,allocator);
    kek_source_FileTableInit(&program->files,allocator);
    program->tokenFiles=std_array_ArrayNew__SyntaxFile(allocator);
    program->decls=std_array_ArrayNew__ModuleDecl(allocator);
    program->params=std_array_ArrayNew__ModuleParam(allocator);
    program->fields=std_array_ArrayNew__ModuleField(allocator);
    program->typeUses=std_array_ArrayNew__TypeUse(allocator);
    program->funcUses=std_array_ArrayNew__FuncUse(allocator);
    program->typeDeclIndex=std_array_ArrayNew__DeclNameIndexEntry(allocator);
    program->functionDeclIndex=std_array_ArrayNew__DeclNameIndexEntry(allocator);
    program->methodDeclIndex=std_array_ArrayNew__MethodDeclIndexEntry(allocator);
    return (Status_Ok);
}
Status kek_syntax_ProgramDestroy(struct CompilerContext* program) {
    for (usize i=0;i<program->tokenFiles.len;i++) {
        struct Array__Token tokens={0};
        tokens.data=program->tokenFiles.data[i].tokens;
        tokens.len=program->tokenFiles.data[i].tokenLen;
        tokens.cap=program->tokenFiles.data[i].tokenCap;
        tokens.allocator=program->allocator;
        Array__Token_Destroy(&tokens);
        std_string_DestroyOwnedString(&program->tokenFiles.data[i].packageName);
        std_string_DestroyOwnedString(&program->tokenFiles.data[i].moduleName);
    }
    for (usize i=0;i<program->decls.len;i++) {
        std_string_DestroyOwnedString(&program->decls.data[i].packageName);
        std_string_DestroyOwnedString(&program->decls.data[i].moduleName);
    }
    for (usize i=0;i<program->typeUses.len;i++) {
        std_string_DestroyOwnedString(&program->typeUses.data[i].key);
        std_string_DestroyOwnedString(&program->typeUses.data[i].cName);
        std_string_DestroyOwnedString(&program->typeUses.data[i].baseName);
        for (usize j=0;j<program->typeUses.data[i].args.len;j++) {
            std_string_DestroyOwnedString(&program->typeUses.data[i].args.data[j]);
        }
        struct Array__OwnedString args=program->typeUses.data[i].args;
        Array__OwnedString_Destroy(&args);
        program->typeUses.data[i].args=args;
    }
    for (usize i=0;i<program->funcUses.len;i++) {
        std_string_DestroyOwnedString(&program->funcUses.data[i].key);
        std_string_DestroyOwnedString(&program->funcUses.data[i].cName);
        for (usize j=0;j<program->funcUses.data[i].args.len;j++) {
            std_string_DestroyOwnedString(&program->funcUses.data[i].args.data[j]);
        }
        struct Array__OwnedString args=program->funcUses.data[i].args;
        Array__OwnedString_Destroy(&args);
        program->funcUses.data[i].args=args;
    }
    for (usize i=0;i<program->typeDeclIndex.len;i++) {
        std_string_DestroyOwnedString(&program->typeDeclIndex.data[i].name);
        std_string_DestroyOwnedString(&program->typeDeclIndex.data[i].scope);
    }
    for (usize i=0;i<program->functionDeclIndex.len;i++) {
        std_string_DestroyOwnedString(&program->functionDeclIndex.data[i].name);
        std_string_DestroyOwnedString(&program->functionDeclIndex.data[i].scope);
    }
    for (usize i=0;i<program->methodDeclIndex.len;i++) {
        std_string_DestroyOwnedString(&program->methodDeclIndex.data[i].receiverKey);
        std_string_DestroyOwnedString(&program->methodDeclIndex.data[i].name);
    }
    struct Array__SyntaxFile tokenFiles=program->tokenFiles;
    Status status=Array__SyntaxFile_Destroy(&tokenFiles);
    program->tokenFiles=tokenFiles;
    struct Array__ModuleDecl decls=program->decls;
    if (status==Status_Ok) {
        status=Array__ModuleDecl_Destroy(&decls);
    }
    program->decls=decls;
    struct Array__ModuleParam params=program->params;
    if (status==Status_Ok) {
        status=Array__ModuleParam_Destroy(&params);
    }
    program->params=params;
    struct Array__ModuleField fields=program->fields;
    if (status==Status_Ok) {
        status=Array__ModuleField_Destroy(&fields);
    }
    program->fields=fields;
    struct Array__TypeUse typeUses=program->typeUses;
    if (status==Status_Ok) {
        status=Array__TypeUse_Destroy(&typeUses);
    }
    program->typeUses=typeUses;
    struct Array__FuncUse funcUses=program->funcUses;
    if (status==Status_Ok) {
        status=Array__FuncUse_Destroy(&funcUses);
    }
    program->funcUses=funcUses;
    struct Array__DeclNameIndexEntry typeDeclIndex=program->typeDeclIndex;
    if (status==Status_Ok) {
        status=Array__DeclNameIndexEntry_Destroy(&typeDeclIndex);
    }
    program->typeDeclIndex=typeDeclIndex;
    struct Array__DeclNameIndexEntry functionDeclIndex=program->functionDeclIndex;
    if (status==Status_Ok) {
        status=Array__DeclNameIndexEntry_Destroy(&functionDeclIndex);
    }
    program->functionDeclIndex=functionDeclIndex;
    struct Array__MethodDeclIndexEntry methodDeclIndex=program->methodDeclIndex;
    if (status==Status_Ok) {
        status=Array__MethodDeclIndexEntry_Destroy(&methodDeclIndex);
    }
    program->methodDeclIndex=methodDeclIndex;
    if (status==Status_Ok) {
        status=kek_source_FileTableDestroy(&program->files);
    }
    if (status==Status_Ok) {
        status=kek_diagnostics_DiagnosticBagDestroy(&program->diagnostics);
    }
    return (status);
}
Status kek_syntax_AddDiagnostic(struct CompilerContext* program,str message) {
    return (kek_diagnostics_DiagnosticAddCString(&program->diagnostics,DiagnosticSeverity_Error,DiagnosticPhase_Semantic,-1,kek_diagnostics_SourceLocationNew(0,0,0,0),message));
}
Status kek_syntax_ReserveTokenFiles(struct CompilerContext* program,usize additional) {
    struct Array__SyntaxFile tokenFiles=program->tokenFiles;
    Status status=Array__SyntaxFile_Reserve(&tokenFiles,additional);
    program->tokenFiles=tokenFiles;
    return (status);
}
Status kek_syntax_ReserveDecls(struct CompilerContext* program,usize additional) {
    struct Array__ModuleDecl decls=program->decls;
    Status status=Array__ModuleDecl_Reserve(&decls,additional);
    program->decls=decls;
    return (status);
}
Status kek_syntax_ReserveParams(struct CompilerContext* program,usize additional) {
    struct Array__ModuleParam params=program->params;
    Status status=Array__ModuleParam_Reserve(&params,additional);
    program->params=params;
    return (status);
}
Status kek_syntax_ReserveFields(struct CompilerContext* program,usize additional) {
    struct Array__ModuleField fields=program->fields;
    Status status=Array__ModuleField_Reserve(&fields,additional);
    program->fields=fields;
    return (status);
}
Status kek_syntax_ReserveTypeUses(struct CompilerContext* program,usize additional) {
    struct Array__TypeUse typeUses=program->typeUses;
    Status status=Array__TypeUse_Reserve(&typeUses,additional);
    program->typeUses=typeUses;
    return (status);
}
Status kek_syntax_ReserveFuncUses(struct CompilerContext* program,usize additional) {
    struct Array__FuncUse funcUses=program->funcUses;
    Status status=Array__FuncUse_Reserve(&funcUses,additional);
    program->funcUses=funcUses;
    return (status);
}
Status kek_syntax_ReserveTypeDeclIndex(struct CompilerContext* program,usize additional) {
    struct Array__DeclNameIndexEntry items=program->typeDeclIndex;
    Status status=Array__DeclNameIndexEntry_Reserve(&items,additional);
    program->typeDeclIndex=items;
    return (status);
}
Status kek_syntax_ReserveFunctionDeclIndex(struct CompilerContext* program,usize additional) {
    struct Array__DeclNameIndexEntry items=program->functionDeclIndex;
    Status status=Array__DeclNameIndexEntry_Reserve(&items,additional);
    program->functionDeclIndex=items;
    return (status);
}
Status kek_syntax_ReserveMethodDeclIndex(struct CompilerContext* program,usize additional) {
    struct Array__MethodDeclIndexEntry items=program->methodDeclIndex;
    Status status=Array__MethodDeclIndexEntry_Reserve(&items,additional);
    program->methodDeclIndex=items;
    return (status);
}
Status kek_syntax_LoadAndTokenize(struct CompilerContext* program,str entryPath) {
    Status status=kek_source_LoadCompilationSources(entryPath,program->allocator,&program->files,&program->diagnostics);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_syntax_ReserveTokenFiles(program,program->files.files.len);
    if (status!=Status_Ok) {
        return (status);
    }
    for (usize i=0;i<program->files.files.len;i++) {
        status=kek_syntax_InitTokenFileSlot(program->allocator,&program->tokenFiles.data[i],i);
        if (status!=Status_Ok) {
            program->tokenFiles.len=i;
            return (status);
        }
    }
    program->tokenFiles.len=program->files.files.len;
    struct Array__TokenizeJob jobs=std_array_ArrayNew__TokenizeJob(program->allocator);
    if (program->files.files.len>0) {
        status=Array__TokenizeJob_Reserve(&jobs,program->files.files.len);
        if (status!=Status_Ok) {
            return (status);
        }
        jobs.len=program->files.files.len;
    }
    for (usize i=0;i<program->files.files.len;i++) {
        struct TokenizeJob* job=&jobs.data[i];
        job->allocator=program->allocator;
        job->sourceFile=&program->files.files.data[i];
        job->tokenFile=&program->tokenFiles.data[i];
        kek_diagnostics_DiagnosticBagInit(&job->diagnostics,program->allocator);
        job->status=Status_Ok;
        job->fileIndex=i;
        job->isRoot=i==0;
        job->threaded=0;
        Status startStatus=std_thread_ThreadHandleStart(&job->threadHandle,((ptr)(kek_syntax_TokenizeFileThreadEntry)),((ptr)(job)));
        if (startStatus==Status_Ok) {
            job->threaded=1;
        } else {
            kek_syntax_TokenizeJobRun(job);
        }
    }
    Status firstStatus=Status_Ok;
    for (usize i=0;i<program->files.files.len;i++) {
        struct TokenizeJob* job=&jobs.data[i];
        if (job->threaded) {
            Status joinStatus=std_thread_ThreadHandleJoin(&job->threadHandle);
            if (joinStatus!=Status_Ok&&job->status==Status_Ok) {
                job->status=joinStatus;
            }
        }
        Status mergeStatus=kek_syntax_MergeDiagnostics(&program->diagnostics,&job->diagnostics);
        if (firstStatus==Status_Ok&&mergeStatus!=Status_Ok) {
            firstStatus=mergeStatus;
        }
        if (firstStatus==Status_Ok&&job->status!=Status_Ok) {
            firstStatus=job->status;
        }
        kek_diagnostics_DiagnosticBagDestroy(&job->diagnostics);
    }
    Status jobsStatus=Array__TokenizeJob_Destroy(&jobs);
    if (firstStatus==Status_Ok&&jobsStatus!=Status_Ok) {
        firstStatus=jobsStatus;
    }
    return (firstStatus);
}
Status kek_syntax_LoadSyntaxPackage(str entryPath,struct CompilerContext* program) {
    return (kek_syntax_LoadAndTokenize(program,entryPath));
}
Status std_thread_ThreadHandleStart(usize* handle,ptr entry,ptr arg) {
    handle[0]=0;
    if (kek_std_thread_start(handle,entry,arg)!=0) {
        return (Status_IoError);
    }
    return (Status_Ok);
}
Status std_thread_ThreadHandleJoin(usize* handle) {
    if (handle[0]==0) {
        return (Status_Invalid);
    }
    usize value=handle[0];
    handle[0]=0;
    if (kek_std_thread_join(value)!=0) {
        return (Status_IoError);
    }
    return (Status_Ok);
}
