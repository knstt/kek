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

typedef u8 byte;
typedef u64 usize;
typedef i64 isize;
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
struct Directory {
    RawHandle handle;
};
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
typedef enum SelfDiagnosticSeverity {
    SelfDiagnosticSeverity_Note,
    SelfDiagnosticSeverity_Warning,
    SelfDiagnosticSeverity_Error,
} SelfDiagnosticSeverity;
typedef enum SelfDiagnosticPhase {
    SelfDiagnosticPhase_Source,
    SelfDiagnosticPhase_Lex,
    SelfDiagnosticPhase_Parse,
    SelfDiagnosticPhase_TypedParse,
    SelfDiagnosticPhase_Semantic,
    SelfDiagnosticPhase_Codegen,
} SelfDiagnosticPhase;
struct SelfSourceLocation {
    usize line;
    usize column;
    usize offset;
    usize length;
};
struct SelfDiagnostic {
    SelfDiagnosticSeverity severity;
    SelfDiagnosticPhase phase;
    i64 fileIndex;
    struct SelfSourceLocation location;
    struct OwnedString message;
};
struct SelfDiagnosticBag {
    struct SelfDiagnostic items[256];
    usize count;
    usize errorCount;
    struct Allocator allocator;
};
struct SelfSourceFile {
    struct OwnedString path;
    struct OwnedString content;
    usize fileIndex;
};
struct SelfFileTable {
    struct SelfSourceFile files[64];
    usize count;
    struct Allocator allocator;
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
struct Token {
    TokenKind kind;
    u64 subkind;
    usize line;
    usize column;
    usize offset;
    usize length;
};
struct Tokenizer {
    struct ByteCursor cursor;
    bool emitComments;
};
typedef enum SelfAstKind {
    SelfAstKind_File,
    SelfAstKind_Statement,
    SelfAstKind_Block,
    SelfAstKind_Group,
    SelfAstKind_Index,
    SelfAstKind_Generic,
    SelfAstKind_Token,
} SelfAstKind;
struct SelfAstNode {
    SelfAstKind kind;
    struct SelfSourceLocation location;
    struct Token token;
    usize firstChild;
    usize lastChild;
    usize nextSibling;
    usize childCount;
};
struct SelfAstTree {
    struct SelfAstNode* nodes;
    usize len;
    usize cap;
    usize root;
    struct Allocator allocator;
};
struct SelfParser {
    struct Token* tokens;
    usize count;
    usize position;
    struct String source;
    i64 fileIndex;
    struct SelfDiagnosticBag* diagnostics;
    struct SelfAstTree tree;
    usize errorCount;
};
typedef enum SelfCDeclKind {
    SelfCDeclKind_Unknown,
    SelfCDeclKind_Alias,
    SelfCDeclKind_Struct,
    SelfCDeclKind_Union,
    SelfCDeclKind_Enum,
    SelfCDeclKind_Function,
    SelfCDeclKind_ExternC,
} SelfCDeclKind;
typedef enum SelfCTypeKind {
    SelfCTypeKind_Unknown,
    SelfCTypeKind_Void,
    SelfCTypeKind_Bool,
    SelfCTypeKind_Integer,
    SelfCTypeKind_Float,
    SelfCTypeKind_Pointer,
    SelfCTypeKind_Struct,
    SelfCTypeKind_Union,
    SelfCTypeKind_Enum,
    SelfCTypeKind_Alias,
} SelfCTypeKind;
struct SelfCTokenFile {
    struct Token* tokens;
    usize tokenLen;
    usize tokenCap;
    struct String sourceText;
    struct String path;
    struct OwnedString packageName;
    struct OwnedString moduleName;
    usize fileIndex;
};
struct SelfCTokenizeJob {
    struct Allocator allocator;
    struct SelfSourceFile* sourceFile;
    struct SelfCTokenFile* tokenFile;
    struct SelfDiagnosticBag diagnostics;
    usize threadHandle;
    Status status;
    usize fileIndex;
    bool isRoot;
    bool threaded;
};
struct SelfCTypeUse {
    struct OwnedString key;
    struct OwnedString cName;
    struct OwnedString baseName;
    struct OwnedString arg0;
    struct OwnedString arg1;
    struct OwnedString arg2;
    usize argCount;
    bool emitted;
};
struct SelfCFuncUse {
    usize declIndex;
    struct OwnedString key;
    struct OwnedString cName;
    struct OwnedString arg0;
    struct OwnedString arg1;
    struct OwnedString arg2;
    usize argCount;
    bool emitted;
};
struct SelfCParam {
    usize fileIndex;
    usize typeStart;
    usize typeEnd;
    usize nameIndex;
    usize defaultStart;
    usize defaultEnd;
    bool hasDefault;
};
struct SelfCField {
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
};
struct SelfCDecl {
    SelfCDeclKind kind;
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
    bool emitted;
};
struct SelfCLocal {
    struct OwnedString name;
    struct OwnedString typeKey;
    struct OwnedString cType;
    struct OwnedString arrayLen;
    bool isArray;
    bool isPointer;
};
struct SelfCEnv {
    struct SelfCLocal locals[512];
    usize localCount;
    struct OwnedString returnTypeKey;
    struct OwnedString returnCType;
    struct OwnedString thisTypeKey;
    struct OwnedString thisCType;
    bool hasThis;
    usize deferCounter;
    usize eachCounter;
};
struct SelfCExpr {
    struct OwnedString text;
    struct OwnedString typeKey;
    struct OwnedString cType;
    struct OwnedString arrayLen;
    bool isArray;
    bool isLvalue;
    bool isPointer;
};
struct SelfCExprParser {
    struct SelfCProgram* program;
    struct SelfCEnv* env;
    usize fileIndex;
    usize pos;
    usize end;
    struct OwnedString expectedTypeKey;
    struct OwnedString expectedCType;
    bool expectedIsArray;
};
struct SelfCTypeInfo {
    struct OwnedString key;
    struct OwnedString cType;
    struct OwnedString baseName;
    struct OwnedString arg0;
    struct OwnedString arg1;
    struct OwnedString arg2;
    usize argCount;
    bool isPointer;
};
struct SelfCProgram {
    struct Allocator allocator;
    struct SelfDiagnosticBag diagnostics;
    struct SelfFileTable files;
    struct SelfCTokenFile tokenFiles[96];
    usize tokenFileCount;
    struct SelfCDecl decls[4096];
    usize declCount;
    struct SelfCParam params[4096];
    usize paramCount;
    struct SelfCField fields[8192];
    usize fieldCount;
    struct SelfCTypeUse typeUses[2048];
    usize typeUseCount;
    struct SelfCFuncUse funcUses[2048];
    usize funcUseCount;
};
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
struct Thread {
    usize handle;
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
struct Result__Directory {
    Status status;
    struct Directory value;
};
struct Result__String {
    Status status;
    struct String value;
};
struct Slice__Token {
    struct Token* data;
    usize len;
};
struct Span__Token {
    struct Token* data;
    usize len;
};
struct Result__Token {
    Status status;
    struct Token value;
};
struct Array__Token {
    struct Token* data;
    usize len;
    usize cap;
    struct Allocator allocator;
};

int main(int argc,str* argv);
struct Allocator std_mem_DefaultAllocator(void);
void std_mem_SetBytes(byte* dest,byte value,usize count);
struct String std_string_StringFromBytes(struct Slice__byte bytes);
struct String std_string_StringFromCString(str text);
struct Slice__byte String_Bytes(struct String* this);
struct String String_Slice(struct String* this,usize start,usize length);
bool String_Equals(struct String* this,struct String other);
bool String_EqualsCString(struct String* this,str text);
int String_Compare(struct String* this,struct String other);
bool String_EqualsBytes(struct String* this,byte* data,usize len);
struct Result__usize String_FindByte(struct String* this,byte value);
bool String_ContainsByte(struct String* this,byte value);
bool String_StartsWith(struct String* this,struct String prefix);
bool String_EndsWith(struct String* this,struct String suffix);
bool std_string_IsAsciiSpace(byte c);
bool std_string_IsAsciiAlpha(byte c);
bool std_string_IsAsciiDigit(byte c);
bool std_string_IsAsciiWord(byte c);
bool std_string_IsAsciiOperator(byte c);
struct String OwnedString_View(struct OwnedString* this);
struct String std_string_OwnedStringView(struct OwnedString* owned);
Status std_string_DestroyOwnedString(struct OwnedString* owned);
Status std_string_CloneString(struct String text,struct Allocator allocator,struct OwnedString* out);
Status std_string_CloneCString(str text,struct Allocator allocator,struct OwnedString* out);
Status OwnedString_Destroy(struct OwnedString* this);
struct StringBuilder std_string_StringBuilderNew(struct Allocator allocator);
Status StringBuilder_Destroy(struct StringBuilder* this);
Status StringBuilder_Clear(struct StringBuilder* this);
Status StringBuilder_Reserve(struct StringBuilder* this,usize additional);
struct Result__usize StringBuilder_Write(struct StringBuilder* this,struct Slice__byte data);
Status StringBuilder_WriteByte(struct StringBuilder* this,byte value);
Status StringBuilder_WriteString(struct StringBuilder* this,struct String text);
Status StringBuilder_WriteCString(struct StringBuilder* this,str text);
Status StringBuilder_WriteRepeatByte(struct StringBuilder* this,byte value,usize count);
Status StringBuilder_WriteIndent(struct StringBuilder* this,usize count);
struct String StringBuilder_View(struct StringBuilder* this);
struct Result__OwnedString StringBuilder_Detach(struct StringBuilder* this);
struct Result__OwnedString StringBuilder_ToOwnedString(struct StringBuilder* this);
Status std_string_WriteStringToBuilder(struct StringBuilder* builder,struct String text);
Status std_string_WriteSliceToBuilder(struct StringBuilder* builder,struct Slice__byte data);
Status std_string_DestroyStringBuilder(struct StringBuilder* builder);
struct ByteCursor std_scan_ByteCursorNew(struct String input);
bool ByteCursor_AtEnd(struct ByteCursor* this);
byte ByteCursor_Peek(struct ByteCursor* this);
byte ByteCursor_PeekAt(struct ByteCursor* this,usize offset);
byte ByteCursor_Advance(struct ByteCursor* this);
bool ByteCursor_MatchByte(struct ByteCursor* this,byte value);
Status ByteCursor_SkipAsciiWhitespace(struct ByteCursor* this);
struct MemoryReader std_io_MemoryReaderNew(struct Slice__byte data);
struct MemoryWriter std_io_MemoryWriterNew(struct Span__byte data);
struct Result__usize MemoryReader_Read(struct MemoryReader* this,struct Span__byte out);
struct Result__usize MemoryWriter_Write(struct MemoryWriter* this,struct Slice__byte data);
usize MemoryWriter_Written(struct MemoryWriter* this);
Status std_io_WriteByteToMemory(struct MemoryWriter* writer,byte value);
struct Result__File std_file_FileOpen(str path,FileMode mode);
struct Result__usize File_Read(struct File* this,struct Span__byte out);
struct Result__usize File_Write(struct File* this,struct Slice__byte data);
Status File_Flush(struct File* this);
Status std_file_ReadFileToOwnedString(str path,struct Allocator allocator,struct OwnedString* out);
Status std_file_WriteFile(str path,struct String text);
Status File_Close(struct File* this);
struct File std_file_Stdin(void);
struct File std_file_Stdout(void);
struct File std_file_Stderr(void);
Status std_file_ReadAllToOwnedString(struct File file,struct Allocator allocator,struct OwnedString* out);
struct Result__Directory std_dir_DirectoryOpen(str path);
struct Result__String Directory_ReadName(struct Directory* this);
Status Directory_Close(struct Directory* this);
Status std_format_WriteByteToBuilder(struct StringBuilder* writer,byte value);
Status std_format_WriteByteToFile(struct File* writer,byte value);
Status std_format_WriteStringToMemory(struct MemoryWriter* writer,struct String text);
Status std_format_WriteStringToFile(struct File* writer,struct String text);
Status std_format_WriteBoolToBuilder(struct StringBuilder* writer,bool value);
Status std_format_WriteBoolToMemory(struct MemoryWriter* writer,bool value);
Status std_format_WriteBoolToFile(struct File* writer,bool value);
Status std_format_FormatU64ToBuilder(struct StringBuilder* writer,u64 value,u8 base);
Status std_format_FormatI64ToBuilder(struct StringBuilder* writer,i64 value);
Status std_format_FormatBoolToBuilder(struct StringBuilder* writer,bool value);
int std_process_ProcessRun(str command);
struct SelfSourceLocation kek_diagnostics_SelfSourceLocationNew(usize line,usize column,usize offset,usize length);
void kek_diagnostics_SelfDiagnosticBagInit(struct SelfDiagnosticBag* bag,struct Allocator allocator);
Status kek_diagnostics_SelfDiagnosticBagDestroy(struct SelfDiagnosticBag* bag);
Status kek_diagnostics_SelfDiagnosticAdd(struct SelfDiagnosticBag* bag,SelfDiagnosticSeverity severity,SelfDiagnosticPhase phase,i64 fileIndex,struct SelfSourceLocation location,struct String message);
Status kek_diagnostics_SelfDiagnosticAddCString(struct SelfDiagnosticBag* bag,SelfDiagnosticSeverity severity,SelfDiagnosticPhase phase,i64 fileIndex,struct SelfSourceLocation location,str message);
Status kek_diagnostics_SelfDiagnosticAddPathMessage(struct SelfDiagnosticBag* bag,SelfDiagnosticSeverity severity,SelfDiagnosticPhase phase,i64 fileIndex,struct SelfSourceLocation location,str prefix,struct String path);
Status kek_diagnostics_SelfWriteU64Field(struct StringBuilder* out,u64 value,bool separator);
Status kek_diagnostics_SelfWriteI64Field(struct StringBuilder* out,i64 value,bool separator);
Status kek_diagnostics_SelfWriteDiagnosticDump(struct SelfDiagnosticBag* bag,struct StringBuilder* out);
void kek_source_SelfFileTableInit(struct SelfFileTable* table,struct Allocator allocator);
Status kek_source_SelfFileTableDestroy(struct SelfFileTable* table);
bool kek_source_SelfStringEndsWithCString(struct String text,str suffixText);
Status kek_source_SelfNormalizePath(struct String path,struct Allocator allocator,struct OwnedString* out);
bool kek_source_SelfFileAlreadyLoaded(struct SelfFileTable* table,struct String path);
Status kek_source_SelfAddSourceDiagnostic(struct SelfDiagnosticBag* diagnostics,str message);
Status kek_source_SelfReadFile(struct String path,struct SelfFileTable* table,struct SelfDiagnosticBag* diagnostics);
bool kek_source_SelfImportDirectiveAt(struct String source,usize cursor);
Status kek_source_SelfLoadImportFile(struct SelfFileTable* table,struct String path,struct SelfDiagnosticBag* diagnostics,i64 fileIndex,struct SelfSourceLocation location);
Status kek_source_SelfLoadImports(struct SelfFileTable* table,struct SelfSourceFile* file,struct SelfDiagnosticBag* diagnostics);
Status kek_source_SelfLoadCompilationSources(str entryPath,struct Allocator allocator,struct SelfFileTable* table,struct SelfDiagnosticBag* diagnostics);
Status kek_source_SelfWriteSourceDump(struct SelfFileTable* table,struct SelfDiagnosticBag* diagnostics,struct StringBuilder* out);
usize kek_tokenizer_TokenizerGenericFootprint(void);
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
bool Tokenizer_ReadOperator(struct Tokenizer* this,struct Token* out,usize start,usize line,usize column);
bool Tokenizer_ReadPunctuation(struct Tokenizer* this,struct Token* out,usize start,usize line,usize column);
struct Token Tokenizer_Next(struct Tokenizer* this);
Status kek_tokenizer_TokenArrayReserve(struct Array__Token* tokens,usize additional);
Status kek_tokenizer_TokenArrayPush(struct Array__Token* tokens,struct Token token);
Status kek_tokenizer_TokenArrayDestroy(struct Array__Token* tokens);
Status kek_tokenizer_TokenizeToArray(struct String source,bool emitComments,struct Allocator allocator,struct Array__Token* out);
Status kek_tokenizer_WriteTokenField(struct StringBuilder* out,u64 value,bool separator);
Status kek_tokenizer_WriteTokenDumpLine(struct Token token,struct StringBuilder* out);
Status kek_tokenizer_WriteTokenDump(struct String source,bool emitComments,struct StringBuilder* out);
struct SelfSourceLocation kek_ast_TokenLocation(struct Token token);
Status kek_ast_SelfAstTreeReserve(struct SelfAstTree* tree,usize additional);
Status kek_ast_SelfAstTreeInit(struct SelfAstTree* tree,struct Allocator allocator);
Status kek_ast_SelfAstTreeDestroy(struct SelfAstTree* tree);
bool kek_ast_IsPunctuationToken(struct Token* token,PunctuationKind punctuation);
bool kek_ast_IsOperatorToken(struct Token* token,OperatorKind operatorKind);
bool kek_ast_IsTriviaToken(struct Token* token);
bool kek_ast_IsClosingPunctuation(struct Token* token);
bool kek_ast_IsAstTerminator(struct Token* token,u64 closePunctuation);
bool kek_ast_IsGenericTerminator(struct Token* token);
bool kek_ast_SelfTokenTextEquals(struct SelfParser* parser,struct Token* token,str text);
usize kek_ast_SelfCreateAstNode(struct SelfParser* parser,SelfAstKind kind,struct SelfSourceLocation location);
void kek_ast_SelfAddAstChild(struct SelfAstTree* tree,usize parentIndex,usize childIndex);
void kek_ast_SelfFinishLocationFromChildren(struct SelfAstTree* tree,usize nodeIndex);
usize kek_ast_SelfParseTokenNode(struct SelfParser* parser);
void kek_ast_SelfReportParseError(struct SelfParser* parser,struct Token* token,str message);
str kek_ast_SelfPunctuationName(u64 punctuation);
void kek_ast_SelfReportExpected(struct SelfParser* parser,struct Token* token,u64 punctuation);
bool kek_ast_SelfShouldParseGenericList(struct SelfParser* parser,usize previousChildIndex);
void kek_ast_SelfParseChildrenInto(struct SelfParser* parser,usize parentIndex,u64 closePunctuation);
usize kek_ast_SelfParseDelimited(struct SelfParser* parser,SelfAstKind kind,u64 closePunctuation);
usize kek_ast_SelfParseGenericDelimited(struct SelfParser* parser);
usize kek_ast_SelfParseStatement(struct SelfParser* parser,u64 closePunctuation);
usize kek_ast_SelfParseList(struct SelfParser* parser,SelfAstKind listKind,u64 closePunctuation);
Status kek_ast_SelfParseTokens(struct Token* tokens,usize count,struct String source,i64 fileIndex,struct Allocator allocator,struct SelfDiagnosticBag* diagnostics,struct SelfAstTree* out);
Status kek_ast_SelfWriteAstNodeDump(struct SelfAstTree* tree,usize nodeIndex,usize depth,struct StringBuilder* out);
Status kek_ast_SelfWriteAstBridgeDump(struct String source,struct StringBuilder* out);
bool kek_compiler_SelfCIsOk(Status status);
struct String kek_compiler_SelfCTokenText(struct SelfCProgram* program,usize fileIndex,usize tokenIndex);
bool kek_compiler_SelfCTokenEquals(struct SelfCProgram* program,usize fileIndex,usize tokenIndex,str text);
bool kek_compiler_SelfCStringEquals(struct String value,str text);
bool kek_compiler_SelfCIsTokenKind(struct SelfCProgram* program,usize fileIndex,usize tokenIndex,TokenKind kind);
bool kek_compiler_SelfCIsPunctuation(struct SelfCProgram* program,usize fileIndex,usize tokenIndex,PunctuationKind kind);
bool kek_compiler_SelfCIsOperator(struct SelfCProgram* program,usize fileIndex,usize tokenIndex,OperatorKind kind);
bool kek_compiler_SelfCIsKeyword(struct SelfCProgram* program,usize fileIndex,usize tokenIndex,KeywordKind kind);
bool kek_compiler_SelfCIsIdentifierText(struct SelfCProgram* program,usize fileIndex,usize tokenIndex,str text);
bool kek_compiler_SelfCIsEof(struct SelfCProgram* program,usize fileIndex,usize tokenIndex);
usize kek_compiler_SelfCFileTokenCount(struct SelfCProgram* program,usize fileIndex);
Status kek_compiler_SelfCWrite(struct StringBuilder* out,str text);
Status kek_compiler_SelfCWriteString(struct StringBuilder* out,struct String text);
Status kek_compiler_SelfCWriteToken(struct SelfCProgram* program,struct StringBuilder* out,usize fileIndex,usize tokenIndex);
Status kek_compiler_SelfCCloneCString(struct SelfCProgram* program,str text,struct OwnedString* out);
Status kek_compiler_SelfCCloneString(struct SelfCProgram* program,struct String text,struct OwnedString* out);
struct String kek_compiler_SelfCOwnedView(struct OwnedString* value);
bool kek_compiler_SelfCOwnedEqualsCString(struct OwnedString* value,str text);
bool kek_compiler_SelfCOwnedEquals(struct OwnedString* left,struct OwnedString* right);
Status kek_compiler_SelfCDetachBuilder(struct StringBuilder* builder,struct OwnedString* out);
Status kek_compiler_SelfCMakeOwnedEmpty(struct SelfCProgram* program,struct OwnedString* out);
Status kek_compiler_SelfCWriteOwned(struct StringBuilder* out,struct OwnedString* text);
usize kek_compiler_SelfCStringLastSlash(struct String path);
Status kek_compiler_SelfCPackageNameFromPath(struct String path,struct Allocator allocator,struct OwnedString* out);
Status kek_compiler_SelfCModuleNameFromPath(struct String path,struct Allocator allocator,struct OwnedString* out);
Status kek_compiler_SelfCInitTokenFileSlot(struct Allocator allocator,struct SelfCTokenFile* tokenFile,usize fileIndex);
Status kek_compiler_SelfCSetTokenFileNames(struct Allocator allocator,struct SelfCTokenFile* tokenFile,bool isRoot);
Status kek_compiler_SelfCTokenizeSourceFile(struct Allocator allocator,struct SelfSourceFile* sourceFile,struct SelfCTokenFile* tokenFile,usize fileIndex,bool isRoot,struct SelfDiagnosticBag* diagnostics);
void kek_compiler_SelfCTokenizeJobRun(struct SelfCTokenizeJob* job);
ptr kek_compiler_SelfCTokenizeFileThreadEntry(ptr arg);
Status kek_compiler_SelfCMergeDiagnostics(struct SelfDiagnosticBag* target,struct SelfDiagnosticBag* source);
Status kek_compiler_SelfCProgramInit(struct SelfCProgram* program,struct Allocator allocator);
Status kek_compiler_SelfCProgramDestroy(struct SelfCProgram* program);
Status kek_compiler_SelfCAddDiagnostic(struct SelfCProgram* program,str message);
Status kek_compiler_SelfCLoadAndTokenize(struct SelfCProgram* program,str entryPath);
int kek_compiler_SelfCompilerFail(str text);
int kek_compiler_SelfCompilerPrintHelp(void);
Status kek_compiler_SelfCWriteDiagnostics(struct SelfCProgram* program);
bool kek_compiler_SelfCIsOpenDelimiter(struct SelfCProgram* program,usize fileIndex,usize index);
bool kek_compiler_SelfCDelimiterMatches(struct SelfCProgram* program,usize fileIndex,usize openIndex,usize closeIndex);
usize kek_compiler_SelfCFindMatching(struct SelfCProgram* program,usize fileIndex,usize openIndex);
usize kek_compiler_SelfCSkipDelimited(struct SelfCProgram* program,usize fileIndex,usize index);
usize kek_compiler_SelfCSkipAttributes(struct SelfCProgram* program,usize fileIndex,usize index);
usize kek_compiler_SelfCFindTopLevelColon(struct SelfCProgram* program,usize fileIndex,usize start,usize end);
usize kek_compiler_SelfCFindTokenAtDepthZero(struct SelfCProgram* program,usize fileIndex,usize start,usize end,PunctuationKind punctuation);
usize kek_compiler_SelfCFindOperatorScope(struct SelfCProgram* program,usize fileIndex,usize start,usize end);
usize kek_compiler_SelfCFindNextGroup(struct SelfCProgram* program,usize fileIndex,usize start,usize end);
usize kek_compiler_SelfCFindNextBlock(struct SelfCProgram* program,usize fileIndex,usize start,usize end);
u8 kek_compiler_SelfCOperatorCode(struct SelfCProgram* program,usize fileIndex,usize index);
Status kek_compiler_SelfCAddDecl(struct SelfCProgram* program,struct SelfCDecl* decl);
Status kek_compiler_SelfCAddParam(struct SelfCProgram* program,struct SelfCParam* param);
Status kek_compiler_SelfCAddField(struct SelfCProgram* program,struct SelfCField* field);
Status kek_compiler_SelfCParseParams(struct SelfCProgram* program,struct SelfCDecl* decl,usize fileIndex,usize start,usize end);
Status kek_compiler_SelfCParseFields(struct SelfCProgram* program,struct SelfCDecl* decl,usize fileIndex,usize start,usize end);
bool kek_compiler_SelfCDeclNameMatches(struct SelfCProgram* program,struct SelfCDecl* decl,struct String name);
struct SelfCDecl* kek_compiler_SelfCFindTypeDecl(struct SelfCProgram* program,struct String name);
Status kek_compiler_SelfCParseAliasDecl(struct SelfCProgram* program,usize fileIndex,usize start,usize* outEnd);
Status kek_compiler_SelfCParseTypeDecl(struct SelfCProgram* program,usize fileIndex,usize start,SelfCDeclKind kind,usize* outEnd);
Status kek_compiler_SelfCParseExternDecl(struct SelfCProgram* program,usize fileIndex,usize start,usize* outEnd);
Status kek_compiler_SelfCParseFunctionDecl(struct SelfCProgram* program,usize fileIndex,usize start,usize* outEnd);
Status kek_compiler_SelfCParseDeclarations(struct SelfCProgram* program);
Status kek_compiler_SelfCWriteTokenRangeRaw(struct SelfCProgram* program,struct StringBuilder* out,usize fileIndex,usize start,usize end);
bool kek_compiler_SelfCIsBuiltinTypeName(struct String name);
Status kek_compiler_SelfCWriteSanitized(struct StringBuilder* out,struct String text);
Status kek_compiler_SelfCWriteOperatorName(struct StringBuilder* out,u8 operatorCode);
bool kek_compiler_SelfCDeclPackageIsRoot(struct SelfCDecl* decl);
Status kek_compiler_SelfCWriteDeclCName(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl);
Status kek_compiler_SelfCWriteTypeSuffixFromRange(struct SelfCProgram* program,struct StringBuilder* out,usize fileIndex,usize start,usize end);
Status kek_compiler_SelfCTypeInfoDestroy(struct SelfCTypeInfo* info);
Status kek_compiler_SelfCTypeInfoInitEmpty(struct SelfCProgram* program,struct SelfCTypeInfo* info);
Status kek_compiler_SelfCReplaceOwned(struct OwnedString* target,struct OwnedString value);
Status kek_compiler_SelfCBuildTypeKey(struct SelfCProgram* program,usize fileIndex,usize start,usize end,struct OwnedString* out);
Status kek_compiler_SelfCWriteCTypeFromKey(struct SelfCProgram* program,struct StringBuilder* out,struct String key);
Status kek_compiler_SelfCMakeCTypeFromKey(struct SelfCProgram* program,struct String key,struct OwnedString* out);
Status kek_compiler_SelfCTypeInfoFromKey(struct SelfCProgram* program,struct String key,struct SelfCTypeInfo* info);
Status kek_compiler_SelfCRenderTypeInfo(struct SelfCProgram* program,usize fileIndex,usize start,usize end,struct SelfCTypeInfo* info);
bool kek_compiler_SelfCTypeUseExists(struct SelfCProgram* program,struct String key);
Status kek_compiler_SelfCAddTypeUse(struct SelfCProgram* program,struct SelfCTypeInfo* info);
Status kek_compiler_SelfCAddTypeUseFromBaseArg(struct SelfCProgram* program,str baseName,struct String arg0);
bool kek_compiler_SelfCConcreteTypeKey(struct SelfCProgram* program,struct String key);
bool kek_compiler_SelfCTypeInfoConcrete(struct SelfCProgram* program,struct SelfCTypeInfo* info);
Status kek_compiler_SelfCCollectTypeUsesInRange(struct SelfCProgram* program,usize fileIndex,usize start,usize end);
bool kek_compiler_SelfCDeclScopeMatches(struct SelfCDecl* decl,struct String scopeName);
struct SelfCDecl* kek_compiler_SelfCFindFunctionDeclByName(struct SelfCProgram* program,struct String name,struct String scopeName);
bool kek_compiler_SelfCFuncUseExists(struct SelfCProgram* program,usize declIndex,struct String key);
usize kek_compiler_SelfCDeclIndex(struct SelfCProgram* program,struct SelfCDecl* decl);
Status kek_compiler_SelfCBuildGenericFuncCName(struct SelfCProgram* program,struct SelfCDecl* decl,struct SelfCTypeInfo* arg0,struct SelfCTypeInfo* arg1,struct SelfCTypeInfo* arg2,usize argCount,struct OwnedString* out);
Status kek_compiler_SelfCAddFuncUse(struct SelfCProgram* program,struct SelfCDecl* decl,struct SelfCTypeInfo* arg0,struct SelfCTypeInfo* arg1,struct SelfCTypeInfo* arg2,usize argCount);
Status kek_compiler_SelfCCollectGenericFunctionUsesInRange(struct SelfCProgram* program,usize fileIndex,usize start,usize end);
struct SelfCDecl* kek_compiler_SelfCFindGenericMethodDecl(struct SelfCProgram* program,struct String receiverBase,str methodName);
Status kek_compiler_SelfCAddGenericMethodUseByName(struct SelfCProgram* program,struct SelfCTypeUse* typeUse,str methodName);
Status kek_compiler_SelfCCollectGenericCollectionMethods(struct SelfCProgram* program);
Status kek_compiler_SelfCCollectGenericFunctionUses(struct SelfCProgram* program);
Status kek_compiler_SelfCCollectTypeUses(struct SelfCProgram* program);
Status kek_compiler_SelfCWritePrelude(struct StringBuilder* out);
Status kek_compiler_SelfCWriteDeclarator(struct SelfCProgram* program,struct StringBuilder* out,usize fileIndex,usize typeStart,usize typeEnd,usize nameIndex,bool isArray,usize arrayStart,usize arrayEnd);
Status kek_compiler_SelfCWriteFields(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl);
bool kek_compiler_SelfCGenericParamEquals(struct SelfCProgram* program,struct SelfCDecl* decl,usize paramIndex,struct String name);
Status kek_compiler_SelfCWriteTypeSuffixSubst(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCTypeUse* use,usize fileIndex,usize start,usize end);
Status kek_compiler_SelfCMakeSubstTypeKey(struct SelfCProgram* program,struct SelfCDecl* decl,struct SelfCTypeUse* use,usize fileIndex,usize start,usize end,struct OwnedString* out);
Status kek_compiler_SelfCWriteDeclaratorSubst(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCTypeUse* use,usize fileIndex,usize typeStart,usize typeEnd,usize nameIndex,bool isArray,usize arrayStart,usize arrayEnd);
Status kek_compiler_SelfCWriteFieldsSubst(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCTypeUse* use);
Status kek_compiler_SelfCWriteStructDecl(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct String specializedName);
Status kek_compiler_SelfCWriteUnionDecl(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl);
Status kek_compiler_SelfCWriteEnumDecl(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl);
Status kek_compiler_SelfCWriteAliasDecl(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl);
Status kek_compiler_SelfCWriteExternDecl(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl);
Status kek_compiler_SelfCWriteTypeDeclarations(struct SelfCProgram* program,struct StringBuilder* out);
Status kek_compiler_SelfCEnvInit(struct SelfCProgram* program,struct SelfCEnv* env);
Status kek_compiler_SelfCEnvDestroy(struct SelfCEnv* env);
struct SelfCLocal* kek_compiler_SelfCEnvFind(struct SelfCEnv* env,struct String name);
Status kek_compiler_SelfCEnvAdd(struct SelfCProgram* program,struct SelfCEnv* env,struct String name,struct String typeKey,struct String cType,bool isArray,struct String arrayLen,bool isPointer);
Status kek_compiler_SelfCExprInitEmpty(struct SelfCProgram* program,struct SelfCExpr* expr);
Status kek_compiler_SelfCExprDestroy(struct SelfCExpr* expr);
Status kek_compiler_SelfCExprSetText(struct SelfCProgram* program,struct SelfCExpr* expr,struct String text);
Status kek_compiler_SelfCExprSetCString(struct SelfCProgram* program,struct SelfCExpr* expr,str text);
Status kek_compiler_SelfCExprSetType(struct SelfCProgram* program,struct SelfCExpr* expr,struct String key,struct String cType,bool isPointer);
Status kek_compiler_SelfCExprFromBuilder(struct SelfCExpr* expr,struct StringBuilder* builder);
u8 kek_compiler_SelfCOperatorPrecedence(struct SelfCProgram* program,usize fileIndex,usize index);
bool kek_compiler_SelfCOperatorRightAssociative(struct SelfCProgram* program,usize fileIndex,usize index);
Status kek_compiler_SelfCWriteNumberToken(struct SelfCProgram* program,struct StringBuilder* out,usize fileIndex,usize index);
struct SelfCDecl* kek_compiler_SelfCFindFieldDecl(struct SelfCProgram* program,struct String typeKey);
Status kek_compiler_SelfCFieldType(struct SelfCProgram* program,struct String typeKey,struct String fieldName,struct SelfCTypeInfo* outInfo);
struct SelfCDecl* kek_compiler_SelfCFindMethod(struct SelfCProgram* program,struct String receiverType,struct String name,bool isOperator,u8 operatorCode,usize argCount);
Status kek_compiler_SelfCParserInit(struct SelfCProgram* program,struct SelfCEnv* env,usize fileIndex,usize start,usize end,struct String expectedKey,struct String expectedCType,bool expectedIsArray,struct SelfCExprParser* parser);
Status kek_compiler_SelfCParserDestroy(struct SelfCExprParser* parser);
Status kek_compiler_SelfCCompileExpressionRange(struct SelfCProgram* program,struct SelfCEnv* env,usize fileIndex,usize start,usize end,struct String expectedKey,struct String expectedCType,bool expectedIsArray,struct SelfCExpr* out);
Status kek_compiler_SelfCWriteInitializerList(struct SelfCExprParser* parser,usize start,usize end,struct StringBuilder* out);
Status kek_compiler_SelfCExprFromLiteralBlock(struct SelfCExprParser* parser,usize blockStart,usize blockEnd,struct SelfCExpr* out);
Status kek_compiler_SelfCCompilePrimary(struct SelfCExprParser* parser,struct SelfCExpr* out);
Status kek_compiler_SelfCWriteCallArgs(struct SelfCExprParser* parser,usize start,usize end,struct StringBuilder* out);
usize kek_compiler_SelfCCountCallArgs(struct SelfCProgram* program,usize fileIndex,usize start,usize end);
Status kek_compiler_SelfCApplyPostfix(struct SelfCExprParser* parser,struct SelfCExpr* expr);
Status kek_compiler_SelfCCompileUnary(struct SelfCExprParser* parser,struct SelfCExpr* out);
Status kek_compiler_SelfCCompileBinaryOperation(struct SelfCExprParser* parser,struct SelfCExpr* left,usize operatorIndex,struct SelfCExpr* right,struct SelfCExpr* out);
Status kek_compiler_SelfCCompileExpression(struct SelfCExprParser* parser,u8 minPrecedence,struct SelfCExpr* out);
Status kek_compiler_SelfCWriteIndent(struct StringBuilder* out,usize indent);
usize kek_compiler_SelfCFindStatementSemicolon(struct SelfCProgram* program,usize fileIndex,usize start,usize end);
Status kek_compiler_SelfCArrayLenString(struct SelfCProgram* program,usize fileIndex,usize start,usize end,struct OwnedString* out);
bool kek_compiler_SelfCFieldDefaultIsZero(struct SelfCProgram* program,struct SelfCField* field);
Status kek_compiler_SelfCWriteDefaultInitializer(struct SelfCProgram* program,struct StringBuilder* out,struct String typeKey);
Status kek_compiler_SelfCWriteVarDecl(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize semicolon,usize indent);
Status kek_compiler_SelfCWriteExprStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize semicolon,usize indent);
Status kek_compiler_SelfCWriteIfStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_compiler_SelfCWriteWhileStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_compiler_SelfCWriteDoStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_compiler_SelfCWriteForStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_compiler_SelfCWriteEachStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_compiler_SelfCWriteSwitchStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_compiler_SelfCWriteReturnStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize semicolon,usize indent);
Status kek_compiler_SelfCWriteSingleStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent);
Status kek_compiler_SelfCWriteDeferred(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize end,usize indent);
Status kek_compiler_SelfCWriteBlock(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize end,usize indent);
Status kek_compiler_SelfCFuncUseTempTypeUse(struct SelfCFuncUse* funcUse,struct SelfCTypeUse* out);
Status kek_compiler_SelfCWriteSubstTypeForFunc(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCFuncUse* funcUse,usize fileIndex,usize start,usize end);
Status kek_compiler_SelfCWriteFunctionSignature(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct String nameOverride,struct SelfCFuncUse* funcUse);
Status kek_compiler_SelfCWriteFunctionPrototype(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl);
Status kek_compiler_SelfCWriteFunctionBody(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl);
Status kek_compiler_SelfCWriteManualLine(struct StringBuilder* out,str text);
Status kek_compiler_SelfCWriteArrayMethodRef(struct StringBuilder* out,struct SelfCFuncUse* use,str name);
Status kek_compiler_SelfCWriteLinkedListMethodRef(struct StringBuilder* out,struct SelfCFuncUse* use,str name);
Status kek_compiler_SelfCWriteResultTypeRef(struct StringBuilder* out,struct SelfCFuncUse* use);
Status kek_compiler_SelfCWriteListNodeTypeRef(struct StringBuilder* out,struct SelfCFuncUse* use);
Status kek_compiler_SelfCWriteManualArrayGenericBody(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCFuncUse* use,struct String argC);
Status kek_compiler_SelfCWriteManualLinkedListGenericBody(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCFuncUse* use);
Status kek_compiler_SelfCWriteManualGenericBody(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCFuncUse* use);
Status kek_compiler_SelfCWriteFunctionDeclarations(struct SelfCProgram* program,struct StringBuilder* out);
Status kek_compiler_SelfCWriteFunctionDefinitions(struct SelfCProgram* program,struct StringBuilder* out);
Status kek_compiler_SelfCWriteProgram(struct SelfCProgram* program,struct StringBuilder* out);
Status kek_compiler_SelfCompileToCString(str inputPath,struct Allocator allocator,struct StringBuilder* out,struct SelfDiagnosticBag* diagnostics);
Status kek_compiler_SelfCompileToC(str inputPath,str outputPath,struct Allocator allocator,struct SelfDiagnosticBag* diagnostics);
Status kek_compiler_SelfCompileToCStringWithoutDiagnostics(str inputPath,struct Allocator allocator,struct StringBuilder* out);
Status kek_compiler_SelfWriteCompileDiagnostics(str inputPath,struct Allocator allocator,struct StringBuilder* out);
Status kek_compiler_SelfCompilerRunBuild(str inputPath,str outputPath);
int kek_compiler_SelfCompilerMain(int argc,str* argv);
Status std_thread_ThreadHandleStart(usize* handle,ptr entry,ptr arg);
Status std_thread_ThreadHandleJoin(usize* handle);
Status std_thread_ThreadStart(struct Thread* thread,ptr entry,ptr arg);
Status std_thread_ThreadJoin(struct Thread* thread);
Status Thread_Join(struct Thread* this);
void std_mem_Free__byte(struct Allocator allocator,byte* data,usize count);
byte* std_mem_Alloc__byte(struct Allocator allocator,usize count);
byte* std_mem_Resize__byte(struct Allocator allocator,byte* oldData,usize oldCount,usize newCount);
struct Slice__byte std_core_FixedSlice__byte(byte* data,usize len);
struct Span__byte std_core_FixedSpan__byte(byte* data,usize len);
struct Token* std_mem_Resize__Token(struct Allocator allocator,struct Token* oldData,usize oldCount,usize newCount);
void std_mem_Free__Token(struct Allocator allocator,struct Token* data,usize count);
struct SelfAstNode* std_mem_Resize__SelfAstNode(struct Allocator allocator,struct SelfAstNode* oldData,usize oldCount,usize newCount);
void std_mem_Free__SelfAstNode(struct Allocator allocator,struct SelfAstNode* data,usize count);
Status Array__Token_Destroy(struct Array__Token* this);
Status Array__Token_Clear(struct Array__Token* this);
Status Array__Token_Reserve(struct Array__Token* this,usize additional);
Status Array__Token_AppendSlice(struct Array__Token* this,struct Slice__Token items);
Status Array__Token_Push(struct Array__Token* this,struct Token value);
Status Array__Token_PushZeroed(struct Array__Token* this);
struct Result__Token Array__Token_Pop(struct Array__Token* this);
struct Result__Token Array__Token_Get(struct Array__Token* this,usize index);
Status Array__Token_Set(struct Array__Token* this,usize index,struct Token value);
struct Token* Array__Token_GetPtr(struct Array__Token* this,usize index);
struct Token* Array__Token_LastPtr(struct Array__Token* this);
struct Span__Token Array__Token_Span(struct Array__Token* this);
struct Slice__Token Array__Token_Slice(struct Array__Token* this);

void std_mem_Free__byte(struct Allocator allocator,byte* data,usize count) {
    (void)allocator;
    (void)count;
    kek_std_free(data);
}
byte* std_mem_Alloc__byte(struct Allocator allocator,usize count) {
    (void)allocator;
    return (byte*)kek_std_alloc(count*sizeof(byte));
}
byte* std_mem_Resize__byte(struct Allocator allocator,byte* oldData,usize oldCount,usize newCount) {
    (void)allocator;
    (void)oldCount;
    return (byte*)kek_std_resize(oldData,newCount*sizeof(byte));
}
struct Slice__byte std_core_FixedSlice__byte(byte* data,usize len) {
    struct Slice__byte out={0};
    out.data=data;
    out.len=len;
    return out;
}
struct Span__byte std_core_FixedSpan__byte(byte* data,usize len) {
    struct Span__byte out={0};
    out.data=data;
    out.len=len;
    return out;
}
struct Token* std_mem_Resize__Token(struct Allocator allocator,struct Token* oldData,usize oldCount,usize newCount) {
    (void)allocator;
    (void)oldCount;
    return (struct Token*)kek_std_resize(oldData,newCount*sizeof(struct Token));
}
void std_mem_Free__Token(struct Allocator allocator,struct Token* data,usize count) {
    (void)allocator;
    (void)count;
    kek_std_free(data);
}
struct SelfAstNode* std_mem_Resize__SelfAstNode(struct Allocator allocator,struct SelfAstNode* oldData,usize oldCount,usize newCount) {
    (void)allocator;
    (void)oldCount;
    return (struct SelfAstNode*)kek_std_resize(oldData,newCount*sizeof(struct SelfAstNode));
}
void std_mem_Free__SelfAstNode(struct Allocator allocator,struct SelfAstNode* data,usize count) {
    (void)allocator;
    (void)count;
    kek_std_free(data);
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
Status Array__Token_Clear(struct Array__Token* this) {
    this->len=0;
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
Status Array__Token_AppendSlice(struct Array__Token* this,struct Slice__Token items) {
    Status status=Array__Token_Reserve(this,items.len);
    if(status!=Status_Ok){return status;}
    for(usize i=0;i<items.len;i++){
        this->data[this->len+i]=items.data[i];
    }
    this->len+=items.len;
    return Status_Ok;
}
Status Array__Token_Push(struct Array__Token* this,struct Token value) {
    Status status=Array__Token_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    this->data[this->len]=value;
    this->len+=1;
    return Status_Ok;
}
Status Array__Token_PushZeroed(struct Array__Token* this) {
    Status status=Array__Token_Reserve(this,1);
    if(status!=Status_Ok){return status;}
    kek_std_mem_set((void*)(&this->data[this->len]),0,sizeof(struct Token));
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
struct Result__Token Array__Token_Get(struct Array__Token* this,usize index) {
    struct Result__Token result={0};
    if(index>=this->len){result.status=Status_Invalid;return result;}
    result.status=Status_Ok;
    result.value=this->data[index];
    return result;
}
Status Array__Token_Set(struct Array__Token* this,usize index,struct Token value) {
    if(index>=this->len){return Status_Invalid;}
    this->data[index]=value;
    return Status_Ok;
}
struct Token* Array__Token_GetPtr(struct Array__Token* this,usize index) {
    if(index>=this->len){return 0;}
    return &this->data[index];
}
struct Token* Array__Token_LastPtr(struct Array__Token* this) {
    if(this->len==0){return 0;}
    return &this->data[this->len-1];
}
struct Span__Token Array__Token_Span(struct Array__Token* this) {
    struct Span__Token out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
struct Slice__Token Array__Token_Slice(struct Array__Token* this) {
    struct Slice__Token out={0};
    out.data=this->data;
    out.len=this->len;
    return out;
}
int main(int argc,str* argv) {
    return (kek_compiler_SelfCompilerMain(argc,argv));
}
struct Allocator std_mem_DefaultAllocator(void) {
    return ((struct Allocator){.context=0});
}
void std_mem_SetBytes(byte* dest,byte value,usize count) {
    kek_std_mem_set(dest,value,count);
}
struct String std_string_StringFromBytes(struct Slice__byte bytes) {
    struct String out={0};
    out.data=bytes.data;
    out.len=bytes.len;
    return (out);
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
bool String_EqualsBytes(struct String* this,byte* data,usize len) {
    if (this->len!=len) {
        return (0);
    }
    for (usize i=0;i<len;i++) {
        if (this->data[i]!=data[i]) {
            return (0);
        }
    }
    return (1);
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
bool std_string_IsAsciiOperator(byte c) {
    return (c==33||c==37||c==38||c==42||c==43||c==45||c==47||c==60||c==61||c==62||c==94||c==124);
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
Status StringBuilder_Clear(struct StringBuilder* this) {
    this->len=0;
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
struct Result__OwnedString StringBuilder_ToOwnedString(struct StringBuilder* this) {
    return (StringBuilder_Detach(this));
}
Status std_string_WriteStringToBuilder(struct StringBuilder* builder,struct String text) {
    struct Result__usize result=StringBuilder_Write(builder,String_Bytes(&text));
    return (result.status);
}
Status std_string_WriteSliceToBuilder(struct StringBuilder* builder,struct Slice__byte data) {
    struct Result__usize result=StringBuilder_Write(builder,data);
    return (result.status);
}
Status std_string_DestroyStringBuilder(struct StringBuilder* builder) {
    return (StringBuilder_Destroy(builder));
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
bool ByteCursor_MatchByte(struct ByteCursor* this,byte value) {
    if (ByteCursor_Peek(this)!=value) {
        return (0);
    }
    ByteCursor_Advance(this);
    return (1);
}
Status ByteCursor_SkipAsciiWhitespace(struct ByteCursor* this) {
    while (!ByteCursor_AtEnd(this)&&std_string_IsAsciiSpace(ByteCursor_Peek(this))) {
        ByteCursor_Advance(this);
    }
    return (Status_Ok);
}
struct MemoryReader std_io_MemoryReaderNew(struct Slice__byte data) {
    struct MemoryReader reader={0};
    reader.data=data.data;
    reader.len=data.len;
    reader.pos=0;
    return (reader);
}
struct MemoryWriter std_io_MemoryWriterNew(struct Span__byte data) {
    struct MemoryWriter writer={0};
    writer.data=data.data;
    writer.len=data.len;
    writer.pos=0;
    return (writer);
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
usize MemoryWriter_Written(struct MemoryWriter* this) {
    return (this->pos);
}
Status std_io_WriteByteToMemory(struct MemoryWriter* writer,byte value) {
    byte buffer[1]={value};
    struct Result__usize result=MemoryWriter_Write(writer,std_core_FixedSlice__byte(buffer,1));
    return (result.status);
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
Status File_Flush(struct File* this) {
    if (this->handle==0) {
        return (Status_Invalid);
    }
    if (kek_std_file_flush(this->handle)!=0) {
        return (Status_IoError);
    }
    return (Status_Ok);
}
Status std_file_ReadFileToOwnedString(str path,struct Allocator allocator,struct OwnedString* out) {
    struct Result__File opened=std_file_FileOpen(path,FileMode_Read);
    if (opened.status!=Status_Ok) {
        return (opened.status);
    }
    struct File file=opened.value;
    Status status=std_file_ReadAllToOwnedString(file,allocator,out);
    Status closeStatus=File_Close(&file);
    if (status!=Status_Ok) {
        return (status);
    }
    return (closeStatus);
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
struct File std_file_Stdin(void) {
    struct File file={0};
    file.handle=kek_std_stdin();
    file.owned=0;
    return (file);
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
struct Result__Directory std_dir_DirectoryOpen(str path) {
    struct Result__Directory result={0};
    RawHandle handle=kek_std_dir_open(path);
    if (handle==0) {
        result.status=Status_NotFound;
        return (result);
    }
    struct Directory directory={0};
    directory.handle=handle;
    result.status=Status_Ok;
    result.value=directory;
    return (result);
}
struct Result__String Directory_ReadName(struct Directory* this) {
    struct Result__String result={0};
    if (this->handle==0) {
        result.status=Status_Invalid;
        return (result);
    }
    str name=kek_std_dir_read_name(this->handle);
    if (name==0) {
        result.status=Status_End;
        return (result);
    }
    result.status=Status_Ok;
    result.value=std_string_StringFromCString(name);
    return (result);
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
Status std_format_WriteByteToFile(struct File* writer,byte value) {
    byte buffer[1]={value};
    struct Result__usize result=File_Write(writer,std_core_FixedSlice__byte(buffer,1));
    return (result.status);
}
Status std_format_WriteStringToMemory(struct MemoryWriter* writer,struct String text) {
    struct Result__usize result=MemoryWriter_Write(writer,String_Bytes(&text));
    return (result.status);
}
Status std_format_WriteStringToFile(struct File* writer,struct String text) {
    struct Result__usize result=File_Write(writer,String_Bytes(&text));
    return (result.status);
}
Status std_format_WriteBoolToBuilder(struct StringBuilder* writer,bool value) {
    byte trueText[4]={116,114,117,101};
    byte falseText[5]={102,97,108,115,101};
    if (value) {
        return (std_string_WriteStringToBuilder(writer,std_string_StringFromBytes(std_core_FixedSlice__byte(trueText,((void)(trueText),4)))));
    }
    return (std_string_WriteStringToBuilder(writer,std_string_StringFromBytes(std_core_FixedSlice__byte(falseText,((void)(falseText),5)))));
}
Status std_format_WriteBoolToMemory(struct MemoryWriter* writer,bool value) {
    byte trueText[4]={116,114,117,101};
    byte falseText[5]={102,97,108,115,101};
    if (value) {
        return (std_format_WriteStringToMemory(writer,std_string_StringFromBytes(std_core_FixedSlice__byte(trueText,((void)(trueText),4)))));
    }
    return (std_format_WriteStringToMemory(writer,std_string_StringFromBytes(std_core_FixedSlice__byte(falseText,((void)(falseText),5)))));
}
Status std_format_WriteBoolToFile(struct File* writer,bool value) {
    byte trueText[4]={116,114,117,101};
    byte falseText[5]={102,97,108,115,101};
    if (value) {
        return (std_format_WriteStringToFile(writer,std_string_StringFromBytes(std_core_FixedSlice__byte(trueText,((void)(trueText),4)))));
    }
    return (std_format_WriteStringToFile(writer,std_string_StringFromBytes(std_core_FixedSlice__byte(falseText,((void)(falseText),5)))));
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
Status std_format_FormatBoolToBuilder(struct StringBuilder* writer,bool value) {
    return (std_format_WriteBoolToBuilder(writer,value));
}
int std_process_ProcessRun(str command) {
    return (kek_std_system(command));
}
struct SelfSourceLocation kek_diagnostics_SelfSourceLocationNew(usize line,usize column,usize offset,usize length) {
    struct SelfSourceLocation location={0};
    location.line=line;
    location.column=column;
    location.offset=offset;
    location.length=length;
    return (location);
}
void kek_diagnostics_SelfDiagnosticBagInit(struct SelfDiagnosticBag* bag,struct Allocator allocator) {
    bag->count=0;
    bag->errorCount=0;
    bag->allocator=allocator;
}
Status kek_diagnostics_SelfDiagnosticBagDestroy(struct SelfDiagnosticBag* bag) {
    for (usize i=0;i<bag->count;i++) {
        std_string_DestroyOwnedString(&bag->items[i].message);
    }
    bag->count=0;
    bag->errorCount=0;
    return (Status_Ok);
}
Status kek_diagnostics_SelfDiagnosticAdd(struct SelfDiagnosticBag* bag,SelfDiagnosticSeverity severity,SelfDiagnosticPhase phase,i64 fileIndex,struct SelfSourceLocation location,struct String message) {
    if (severity==SelfDiagnosticSeverity_Error) {
        bag->errorCount+=1;
    }
    if (bag->count>=(sizeof(bag->items)/sizeof((bag->items)[0]))) {
        return (Status_NoMemory);
    }
    struct SelfDiagnostic* diagnostic=&bag->items[bag->count];
    diagnostic->severity=severity;
    diagnostic->phase=phase;
    diagnostic->fileIndex=fileIndex;
    diagnostic->location=location;
    Status status=std_string_CloneString(message,bag->allocator,&diagnostic->message);
    if (status!=Status_Ok) {
        return (status);
    }
    bag->count+=1;
    return (Status_Ok);
}
Status kek_diagnostics_SelfDiagnosticAddCString(struct SelfDiagnosticBag* bag,SelfDiagnosticSeverity severity,SelfDiagnosticPhase phase,i64 fileIndex,struct SelfSourceLocation location,str message) {
    return (kek_diagnostics_SelfDiagnosticAdd(bag,severity,phase,fileIndex,location,std_string_StringFromCString(message)));
}
Status kek_diagnostics_SelfDiagnosticAddPathMessage(struct SelfDiagnosticBag* bag,SelfDiagnosticSeverity severity,SelfDiagnosticPhase phase,i64 fileIndex,struct SelfSourceLocation location,str prefix,struct String path) {
    struct StringBuilder builder=std_string_StringBuilderNew(bag->allocator);
    Status status=StringBuilder_WriteCString(&builder,prefix);
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(&builder,path);
    }
    if (status!=Status_Ok) {
        StringBuilder_Destroy(&builder);
        return (status);
    }
    struct String message=StringBuilder_View(&builder);
    status=kek_diagnostics_SelfDiagnosticAdd(bag,severity,phase,fileIndex,location,message);
    StringBuilder_Destroy(&builder);
    return (status);
}
Status kek_diagnostics_SelfWriteU64Field(struct StringBuilder* out,u64 value,bool separator) {
    Status status=std_format_FormatU64ToBuilder(out,value,10);
    if (status!=Status_Ok) {
        return (status);
    }
    if (separator) {
        return (StringBuilder_WriteByte(out,'|'));
    }
    return (Status_Ok);
}
Status kek_diagnostics_SelfWriteI64Field(struct StringBuilder* out,i64 value,bool separator) {
    Status status=std_format_FormatI64ToBuilder(out,value);
    if (status!=Status_Ok) {
        return (status);
    }
    if (separator) {
        return (StringBuilder_WriteByte(out,'|'));
    }
    return (Status_Ok);
}
Status kek_diagnostics_SelfWriteDiagnosticDump(struct SelfDiagnosticBag* bag,struct StringBuilder* out) {
    for (usize i=0;i<bag->count;i++) {
        struct SelfDiagnostic* diagnostic=&bag->items[i];
        Status status=StringBuilder_WriteCString(out,"diag|");
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_SelfWriteU64Field(out,((u64)(diagnostic->severity)),1);
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_SelfWriteU64Field(out,((u64)(diagnostic->phase)),1);
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_SelfWriteI64Field(out,diagnostic->fileIndex,1);
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_SelfWriteU64Field(out,((u64)(diagnostic->location.line)),1);
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_SelfWriteU64Field(out,((u64)(diagnostic->location.column)),1);
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_SelfWriteU64Field(out,((u64)(diagnostic->location.offset)),1);
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_SelfWriteU64Field(out,((u64)(diagnostic->location.length)),1);
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
void kek_source_SelfFileTableInit(struct SelfFileTable* table,struct Allocator allocator) {
    table->count=0;
    table->allocator=allocator;
}
Status kek_source_SelfFileTableDestroy(struct SelfFileTable* table) {
    for (usize i=0;i<table->count;i++) {
        std_string_DestroyOwnedString(&table->files[i].path);
        std_string_DestroyOwnedString(&table->files[i].content);
    }
    table->count=0;
    return (Status_Ok);
}
bool kek_source_SelfStringEndsWithCString(struct String text,str suffixText) {
    struct String suffix=std_string_StringFromCString(suffixText);
    return (String_EndsWith(&text,suffix));
}
Status kek_source_SelfNormalizePath(struct String path,struct Allocator allocator,struct OwnedString* out) {
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
bool kek_source_SelfFileAlreadyLoaded(struct SelfFileTable* table,struct String path) {
    for (usize i=0;i<table->count;i++) {
        struct String filePath=std_string_OwnedStringView(&table->files[i].path);
        if (String_Equals(&filePath,path)) {
            return (1);
        }
    }
    return (0);
}
Status kek_source_SelfAddSourceDiagnostic(struct SelfDiagnosticBag* diagnostics,str message) {
    return (kek_diagnostics_SelfDiagnosticAddCString(diagnostics,SelfDiagnosticSeverity_Error,SelfDiagnosticPhase_Source,-1,kek_diagnostics_SelfSourceLocationNew(0,0,0,0),message));
}
Status kek_source_SelfReadFile(struct String path,struct SelfFileTable* table,struct SelfDiagnosticBag* diagnostics) {
    struct OwnedString normalized={0};
    Status status=kek_source_SelfNormalizePath(path,table->allocator,&normalized);
    if (status!=Status_Ok) {
        return (status);
    }
    struct String normalizedPath=std_string_OwnedStringView(&normalized);
    if (kek_source_SelfFileAlreadyLoaded(table,normalizedPath)) {
        std_string_DestroyOwnedString(&normalized);
        return (Status_Ok);
    }
    if (table->count>=(sizeof(table->files)/sizeof((table->files)[0]))) {
        std_string_DestroyOwnedString(&normalized);
        return (kek_source_SelfAddSourceDiagnostic(diagnostics,"file table is full"));
    }
    struct Result__File opened=std_file_FileOpen(((str)(normalized.data)),FileMode_Read);
    if (opened.status!=Status_Ok) {
        status=kek_diagnostics_SelfDiagnosticAddPathMessage(diagnostics,SelfDiagnosticSeverity_Error,SelfDiagnosticPhase_Source,-1,kek_diagnostics_SelfSourceLocationNew(0,0,0,0),"could not open file ",normalizedPath);
        std_string_DestroyOwnedString(&normalized);
        return (status);
    }
    struct OwnedString content={0};
    struct File file=opened.value;
    status=std_file_ReadAllToOwnedString(file,table->allocator,&content);
    Status closeStatus=File_Close(&file);
    if (status!=Status_Ok) {
        kek_diagnostics_SelfDiagnosticAddPathMessage(diagnostics,SelfDiagnosticSeverity_Error,SelfDiagnosticPhase_Source,-1,kek_diagnostics_SelfSourceLocationNew(0,0,0,0),"could not read file ",normalizedPath);
        std_string_DestroyOwnedString(&normalized);
        return (status);
    }
    if (closeStatus!=Status_Ok) {
        std_string_DestroyOwnedString(&content);
        std_string_DestroyOwnedString(&normalized);
        return (closeStatus);
    }
    struct SelfSourceFile* source=&table->files[table->count];
    source->path=normalized;
    source->content=content;
    source->fileIndex=table->count;
    table->count+=1;
    return (Status_Ok);
}
bool kek_source_SelfImportDirectiveAt(struct String source,usize cursor) {
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
Status kek_source_SelfLoadImportFile(struct SelfFileTable* table,struct String path,struct SelfDiagnosticBag* diagnostics,i64 fileIndex,struct SelfSourceLocation location) {
    if (!kek_source_SelfStringEndsWithCString(path,".kek")) {
        return (kek_diagnostics_SelfDiagnosticAddPathMessage(diagnostics,SelfDiagnosticSeverity_Error,SelfDiagnosticPhase_Source,fileIndex,location,"import path must name a .kek file ",path));
    }
    usize before=table->count;
    Status status=kek_source_SelfReadFile(path,table,diagnostics);
    if (status!=Status_Ok) {
        return (status);
    }
    if (table->count>before) {
        return (kek_source_SelfLoadImports(table,&table->files[before],diagnostics));
    }
    return (Status_Ok);
}
Status kek_source_SelfLoadImports(struct SelfFileTable* table,struct SelfSourceFile* file,struct SelfDiagnosticBag* diagnostics) {
    struct String source=std_string_OwnedStringView(&file->content);
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
        if (!kek_source_SelfImportDirectiveAt(source,cursor)) {
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
        struct SelfSourceLocation location=kek_diagnostics_SelfSourceLocationNew(1,1,cursor,prefix.len);
        if (end>=source.len) {
            return (kek_diagnostics_SelfDiagnosticAddCString(diagnostics,SelfDiagnosticSeverity_Error,SelfDiagnosticPhase_Source,((i64)(file->fileIndex)),location,"unterminated import"));
        }
        usize length=end-start;
        if (length==0) {
            return (kek_diagnostics_SelfDiagnosticAddCString(diagnostics,SelfDiagnosticSeverity_Error,SelfDiagnosticPhase_Source,((i64)(file->fileIndex)),location,"invalid import path"));
        }
        struct String importPath=String_Slice(&source,start,length);
        Status status=kek_source_SelfLoadImportFile(table,importPath,diagnostics,((i64)(file->fileIndex)),location);
        if (status!=Status_Ok) {
            return (status);
        }
        cursor=end+1;
    }
    return (Status_Ok);
}
Status kek_source_SelfLoadCompilationSources(str entryPath,struct Allocator allocator,struct SelfFileTable* table,struct SelfDiagnosticBag* diagnostics) {
    kek_source_SelfFileTableInit(table,allocator);
    Status status=kek_source_SelfReadFile(std_string_StringFromCString(entryPath),table,diagnostics);
    if (status!=Status_Ok) {
        return (status);
    }
    return (kek_source_SelfLoadImports(table,&table->files[0],diagnostics));
}
Status kek_source_SelfWriteSourceDump(struct SelfFileTable* table,struct SelfDiagnosticBag* diagnostics,struct StringBuilder* out) {
    for (usize i=0;i<table->count;i++) {
        struct SelfSourceFile* file=&table->files[i];
        Status status=StringBuilder_WriteCString(out,"file|");
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_SelfWriteU64Field(out,((u64)(file->fileIndex)),1);
        if (status!=Status_Ok) {
            return (status);
        }
        struct String filePath=std_string_OwnedStringView(&file->path);
        status=StringBuilder_WriteString(out,filePath);
        if (status!=Status_Ok) {
            return (status);
        }
        status=StringBuilder_WriteByte(out,'|');
        if (status!=Status_Ok) {
            return (status);
        }
        status=kek_diagnostics_SelfWriteU64Field(out,((u64)(file->content.len)),0);
        if (status!=Status_Ok) {
            return (status);
        }
        status=StringBuilder_WriteByte(out,'\n');
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (kek_diagnostics_SelfWriteDiagnosticDump(diagnostics,out));
}
usize kek_tokenizer_TokenizerGenericFootprint(void) {
    struct Slice__Token slice={0};
    struct Span__Token span={0};
    struct Result__Token result={0};
    slice.len=0;
    span.len=slice.len;
    result.status=Status_Ok;
    return (span.len+((usize)(result.status)));
}
struct Token kek_tokenizer_TokenNew(TokenKind kind,u64 subkind,usize line,usize column,usize offset,usize length) {
    struct Token token={0};
    token.kind=kind;
    token.subkind=subkind;
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
    return (this->cursor.pos>=this->cursor.input.len);
}
byte Tokenizer_Peek(struct Tokenizer* this) {
    if (Tokenizer_AtEnd(this)) {
        return (0);
    }
    return (this->cursor.input.data[this->cursor.pos]);
}
byte Tokenizer_PeekAt(struct Tokenizer* this,usize offset) {
    usize target=this->cursor.pos+offset;
    if (target>=this->cursor.input.len) {
        return (0);
    }
    return (this->cursor.input.data[target]);
}
byte Tokenizer_Advance(struct Tokenizer* this) {
    byte value=Tokenizer_Peek(this);
    if (Tokenizer_AtEnd(this)) {
        return (0);
    }
    this->cursor.pos+=1;
    if (value=='\n') {
        this->cursor.line+=1;
        this->cursor.column=1;
    } else {
        this->cursor.column+=1;
    }
    return (value);
}
bool kek_tokenizer_TextAtEquals(str text,struct String source,usize start,usize length) {
    usize textLength=0;
    while (text[textLength]!=0) {
        textLength+=1;
    }
    if (textLength!=length) {
        return (0);
    }
    if (start+length>source.len) {
        return (0);
    }
    for (usize i=0;i<length;i++) {
        if (source.data[start+i]!=text[i]) {
            return (0);
        }
    }
    return (1);
}
bool Tokenizer_StartsWith(struct Tokenizer* this,str text) {
    usize i=0;
    while (text[i]!=0) {
        if (Tokenizer_PeekAt(this,i)!=text[i]) {
            return (0);
        }
        i+=1;
    }
    return (1);
}
void Tokenizer_AdvanceMany(struct Tokenizer* this,usize count) {
    for (usize i=0;i<count;i++) {
        Tokenizer_Advance(this);
    }
}
void Tokenizer_SkipWhitespace(struct Tokenizer* this) {
    while (!Tokenizer_AtEnd(this)&&std_string_IsAsciiSpace(Tokenizer_Peek(this))) {
        Tokenizer_Advance(this);
    }
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
Status kek_tokenizer_TokenArrayReserve(struct Array__Token* tokens,usize additional) {
    usize needed=tokens->len+additional;
    if (needed<=tokens->cap) {
        return (Status_Ok);
    }
    usize newCap=tokens->cap;
    if (newCap==0) {
        newCap=64;
    }
    while (newCap<needed) {
        newCap=newCap*2;
    }
    struct Token* newData=std_mem_Resize__Token(tokens->allocator,tokens->data,tokens->cap,newCap);
    if (newData==0) {
        return (Status_NoMemory);
    }
    tokens->data=newData;
    tokens->cap=newCap;
    return (Status_Ok);
}
Status kek_tokenizer_TokenArrayPush(struct Array__Token* tokens,struct Token token) {
    Status status=kek_tokenizer_TokenArrayReserve(tokens,1);
    if (status!=Status_Ok) {
        return (status);
    }
    tokens->data[tokens->len]=token;
    tokens->len+=1;
    return (Status_Ok);
}
Status kek_tokenizer_TokenArrayDestroy(struct Array__Token* tokens) {
    if (tokens->data!=0) {
        std_mem_Free__Token(tokens->allocator,tokens->data,tokens->cap);
    }
    tokens->data=0;
    tokens->len=0;
    tokens->cap=0;
    return (Status_Ok);
}
Status kek_tokenizer_TokenizeToArray(struct String source,bool emitComments,struct Allocator allocator,struct Array__Token* out) {
    out->data=0;
    out->len=0;
    out->cap=0;
    out->allocator=allocator;
    struct Tokenizer tokenizer=kek_tokenizer_TokenizerNew(source,emitComments);
    while (1) {
        struct Token token=Tokenizer_Next(&tokenizer);
        Status status=kek_tokenizer_TokenArrayPush(out,token);
        if (status!=Status_Ok) {
            return (status);
        }
        if (token.kind==TokenKind_Eof) {
            break;
        }
    }
    return (Status_Ok);
}
Status kek_tokenizer_WriteTokenField(struct StringBuilder* out,u64 value,bool separator) {
    Status status=std_format_FormatU64ToBuilder(out,value,10);
    if (status!=Status_Ok) {
        return (status);
    }
    if (separator) {
        return (StringBuilder_WriteByte(out,'|'));
    }
    return (Status_Ok);
}
Status kek_tokenizer_WriteTokenDumpLine(struct Token token,struct StringBuilder* out) {
    Status status=kek_tokenizer_WriteTokenField(out,((u64)(token.kind)),1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_tokenizer_WriteTokenField(out,token.subkind,1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_tokenizer_WriteTokenField(out,((u64)(token.line)),1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_tokenizer_WriteTokenField(out,((u64)(token.column)),1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_tokenizer_WriteTokenField(out,((u64)(token.offset)),1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_tokenizer_WriteTokenField(out,((u64)(token.length)),0);
    if (status!=Status_Ok) {
        return (status);
    }
    return (StringBuilder_WriteByte(out,'\n'));
}
Status kek_tokenizer_WriteTokenDump(struct String source,bool emitComments,struct StringBuilder* out) {
    struct Array__Token tokens={0};
    Status status=kek_tokenizer_TokenizeToArray(source,emitComments,out->allocator,&tokens);
    if (status!=Status_Ok) {
        return (status);
    }
    for (usize i=0;i<tokens.len;i++) {
        status=kek_tokenizer_WriteTokenDumpLine(tokens.data[i],out);
        if (status!=Status_Ok) {
            kek_tokenizer_TokenArrayDestroy(&tokens);
            return (status);
        }
    }
    return (kek_tokenizer_TokenArrayDestroy(&tokens));
}
struct SelfSourceLocation kek_ast_TokenLocation(struct Token token) {
    return (kek_diagnostics_SelfSourceLocationNew(token.line,token.column,token.offset,token.length));
}
Status kek_ast_SelfAstTreeReserve(struct SelfAstTree* tree,usize additional) {
    usize needed=tree->len+additional;
    if (needed<=tree->cap) {
        return (Status_Ok);
    }
    usize newCap=tree->cap;
    if (newCap==0) {
        newCap=128;
    }
    while (newCap<needed) {
        newCap=newCap*2;
    }
    struct SelfAstNode* newData=std_mem_Resize__SelfAstNode(tree->allocator,tree->nodes,tree->cap,newCap);
    if (newData==0) {
        return (Status_NoMemory);
    }
    tree->nodes=newData;
    tree->cap=newCap;
    return (Status_Ok);
}
Status kek_ast_SelfAstTreeInit(struct SelfAstTree* tree,struct Allocator allocator) {
    tree->nodes=0;
    tree->len=0;
    tree->cap=0;
    tree->root=0;
    tree->allocator=allocator;
    Status status=kek_ast_SelfAstTreeReserve(tree,1);
    if (status!=Status_Ok) {
        return (status);
    }
    struct SelfAstNode sentinel={0};
    tree->nodes[0]=sentinel;
    tree->len=1;
    return (Status_Ok);
}
Status kek_ast_SelfAstTreeDestroy(struct SelfAstTree* tree) {
    if (tree->nodes!=0) {
        std_mem_Free__SelfAstNode(tree->allocator,tree->nodes,tree->cap);
    }
    tree->nodes=0;
    tree->len=0;
    tree->cap=0;
    tree->root=0;
    return (Status_Ok);
}
bool kek_ast_IsPunctuationToken(struct Token* token,PunctuationKind punctuation) {
    return (token->kind==TokenKind_Punctuation&&token->subkind==((u64)(punctuation)));
}
bool kek_ast_IsOperatorToken(struct Token* token,OperatorKind operatorKind) {
    return (token->kind==TokenKind_Operator&&token->subkind==((u64)(operatorKind)));
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
bool kek_ast_SelfTokenTextEquals(struct SelfParser* parser,struct Token* token,str text) {
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
usize kek_ast_SelfCreateAstNode(struct SelfParser* parser,SelfAstKind kind,struct SelfSourceLocation location) {
    Status status=kek_ast_SelfAstTreeReserve(&parser->tree,1);
    if (status!=Status_Ok) {
        kek_diagnostics_SelfDiagnosticAddCString(parser->diagnostics,SelfDiagnosticSeverity_Error,SelfDiagnosticPhase_Parse,parser->fileIndex,location,"AST node storage capacity exceeded");
        parser->errorCount+=1;
        if (parser->tree.len>0) {
            return (parser->tree.len-1);
        }
        return (0);
    }
    usize index=parser->tree.len;
    struct SelfAstNode node={0};
    node.kind=kind;
    node.location=location;
    parser->tree.nodes[index]=node;
    parser->tree.len+=1;
    return (index);
}
void kek_ast_SelfAddAstChild(struct SelfAstTree* tree,usize parentIndex,usize childIndex) {
    if (parentIndex==0||childIndex==0) {
        return;
    }
    tree->nodes[childIndex].nextSibling=0;
    if (tree->nodes[parentIndex].lastChild!=0) {
        usize last=tree->nodes[parentIndex].lastChild;
        tree->nodes[last].nextSibling=childIndex;
    } else {
        tree->nodes[parentIndex].firstChild=childIndex;
    }
    tree->nodes[parentIndex].lastChild=childIndex;
    tree->nodes[parentIndex].childCount+=1;
}
void kek_ast_SelfFinishLocationFromChildren(struct SelfAstTree* tree,usize nodeIndex) {
    if (nodeIndex==0||tree->nodes[nodeIndex].childCount==0) {
        return;
    }
    usize firstIndex=tree->nodes[nodeIndex].firstChild;
    usize lastIndex=tree->nodes[nodeIndex].lastChild;
    struct SelfSourceLocation first=tree->nodes[firstIndex].location;
    struct SelfSourceLocation last=tree->nodes[lastIndex].location;
    tree->nodes[nodeIndex].location=first;
    if (last.offset+last.length>=first.offset) {
        tree->nodes[nodeIndex].location.length=(last.offset+last.length)-first.offset;
    }
}
usize kek_ast_SelfParseTokenNode(struct SelfParser* parser) {
    struct Token token=parser->tokens[parser->position];
    parser->position+=1;
    usize nodeIndex=kek_ast_SelfCreateAstNode(parser,SelfAstKind_Token,kek_ast_TokenLocation(token));
    parser->tree.nodes[nodeIndex].token=token;
    return (nodeIndex);
}
void kek_ast_SelfReportParseError(struct SelfParser* parser,struct Token* token,str message) {
    kek_diagnostics_SelfDiagnosticAddCString(parser->diagnostics,SelfDiagnosticSeverity_Error,SelfDiagnosticPhase_Parse,parser->fileIndex,kek_ast_TokenLocation(token[0]),message);
    parser->errorCount+=1;
}
str kek_ast_SelfPunctuationName(u64 punctuation) {
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
void kek_ast_SelfReportExpected(struct SelfParser* parser,struct Token* token,u64 punctuation) {
    struct StringBuilder builder=std_string_StringBuilderNew(parser->tree.allocator);
    StringBuilder_WriteCString(&builder,"expected '");
    StringBuilder_WriteCString(&builder,kek_ast_SelfPunctuationName(punctuation));
    StringBuilder_WriteByte(&builder,'\'');
    struct String message=StringBuilder_View(&builder);
    kek_diagnostics_SelfDiagnosticAdd(parser->diagnostics,SelfDiagnosticSeverity_Error,SelfDiagnosticPhase_Parse,parser->fileIndex,kek_ast_TokenLocation(token[0]),message);
    StringBuilder_Destroy(&builder);
    parser->errorCount+=1;
}
bool kek_ast_SelfShouldParseGenericList(struct SelfParser* parser,usize previousChildIndex) {
    if (previousChildIndex==0||parser->tree.nodes[previousChildIndex].kind!=SelfAstKind_Token) {
        return (0);
    }
    struct Token* previousToken=&parser->tree.nodes[previousChildIndex].token;
    if (previousToken->kind!=TokenKind_Identifier&&previousToken->kind!=TokenKind_Keyword) {
        return (0);
    }
    if (kek_ast_SelfTokenTextEquals(parser,previousToken,"cast")) {
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
void kek_ast_SelfParseChildrenInto(struct SelfParser* parser,usize parentIndex,u64 closePunctuation) {
    while (parser->position<parser->count&&!kek_ast_IsAstTerminator(&parser->tokens[parser->position],closePunctuation)) {
        if (kek_ast_IsTriviaToken(&parser->tokens[parser->position])) {
            parser->position+=1;
            continue;
        }
        usize previousPosition=parser->position;
        usize statement=kek_ast_SelfParseStatement(parser,closePunctuation);
        if (parser->tree.nodes[statement].childCount>0) {
            kek_ast_SelfAddAstChild(&parser->tree,parentIndex,statement);
        } else {
            if (parser->position==previousPosition) {
                break;
            }
        }
    }
}
usize kek_ast_SelfParseDelimited(struct SelfParser* parser,SelfAstKind kind,u64 closePunctuation) {
    struct Token open=parser->tokens[parser->position];
    parser->position+=1;
    usize node=kek_ast_SelfCreateAstNode(parser,kind,kek_ast_TokenLocation(open));
    kek_ast_SelfParseChildrenInto(parser,node,closePunctuation);
    if (parser->position<parser->count&&parser->tokens[parser->position].kind==TokenKind_Punctuation&&parser->tokens[parser->position].subkind==closePunctuation) {
        struct Token close=parser->tokens[parser->position];
        parser->position+=1;
        parser->tree.nodes[node].location=kek_ast_TokenLocation(open);
        parser->tree.nodes[node].location.length=(close.offset+close.length)-open.offset;
    } else {
        if (parser->position<parser->count) {
            kek_ast_SelfReportExpected(parser,&parser->tokens[parser->position],closePunctuation);
            if (kek_ast_IsClosingPunctuation(&parser->tokens[parser->position])) {
                parser->position+=1;
            }
        } else {
            kek_ast_SelfReportParseError(parser,&open,"unterminated delimiter");
        }
    }
    if (parser->tree.nodes[node].childCount>0&&parser->tree.nodes[node].location.length==open.length) {
        kek_ast_SelfFinishLocationFromChildren(&parser->tree,node);
        parser->tree.nodes[node].location.offset=open.offset;
        parser->tree.nodes[node].location.line=open.line;
        parser->tree.nodes[node].location.column=open.column;
    }
    return (node);
}
usize kek_ast_SelfParseGenericDelimited(struct SelfParser* parser) {
    struct Token open=parser->tokens[parser->position];
    parser->position+=1;
    usize node=kek_ast_SelfCreateAstNode(parser,SelfAstKind_Generic,kek_ast_TokenLocation(open));
    while (parser->position<parser->count&&!kek_ast_IsGenericTerminator(&parser->tokens[parser->position])) {
        if (kek_ast_IsTriviaToken(&parser->tokens[parser->position])) {
            parser->position+=1;
            continue;
        }
        usize statement=kek_ast_SelfCreateAstNode(parser,SelfAstKind_Statement,kek_ast_TokenLocation(parser->tokens[parser->position]));
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
                kek_ast_SelfAddAstChild(&parser->tree,statement,kek_ast_SelfParseDelimited(parser,SelfAstKind_Group,((u64)(PunctuationKind_RightParen))));
                continue;
            }
            if (kek_ast_IsPunctuationToken(token,PunctuationKind_LeftBracket)) {
                kek_ast_SelfAddAstChild(&parser->tree,statement,kek_ast_SelfParseDelimited(parser,SelfAstKind_Index,((u64)(PunctuationKind_RightBracket))));
                continue;
            }
            if (kek_ast_SelfShouldParseGenericList(parser,parser->tree.nodes[statement].lastChild)) {
                kek_ast_SelfAddAstChild(&parser->tree,statement,kek_ast_SelfParseGenericDelimited(parser));
                continue;
            }
            kek_ast_SelfAddAstChild(&parser->tree,statement,kek_ast_SelfParseTokenNode(parser));
        }
        kek_ast_SelfFinishLocationFromChildren(&parser->tree,statement);
        if (parser->tree.nodes[statement].childCount>0) {
            kek_ast_SelfAddAstChild(&parser->tree,node,statement);
        }
    }
    if (parser->position<parser->count&&kek_ast_IsOperatorToken(&parser->tokens[parser->position],OperatorKind_Greater)) {
        struct Token close=parser->tokens[parser->position];
        parser->position+=1;
        parser->tree.nodes[node].location=kek_ast_TokenLocation(open);
        parser->tree.nodes[node].location.length=(close.offset+close.length)-open.offset;
    } else {
        if (parser->position<parser->count) {
            kek_ast_SelfReportParseError(parser,&parser->tokens[parser->position],"expected '>'");
        } else {
            kek_ast_SelfReportParseError(parser,&open,"unterminated generic list");
        }
    }
    if (parser->tree.nodes[node].childCount>0&&parser->tree.nodes[node].location.length==open.length) {
        kek_ast_SelfFinishLocationFromChildren(&parser->tree,node);
        parser->tree.nodes[node].location.offset=open.offset;
        parser->tree.nodes[node].location.line=open.line;
        parser->tree.nodes[node].location.column=open.column;
    }
    return (node);
}
usize kek_ast_SelfParseStatement(struct SelfParser* parser,u64 closePunctuation) {
    usize statement=kek_ast_SelfCreateAstNode(parser,SelfAstKind_Statement,kek_ast_TokenLocation(parser->tokens[parser->position]));
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
            kek_ast_SelfAddAstChild(&parser->tree,statement,kek_ast_SelfParseDelimited(parser,SelfAstKind_Block,((u64)(PunctuationKind_RightBrace))));
            break;
        }
        if (kek_ast_IsPunctuationToken(token,PunctuationKind_LeftParen)) {
            kek_ast_SelfAddAstChild(&parser->tree,statement,kek_ast_SelfParseDelimited(parser,SelfAstKind_Group,((u64)(PunctuationKind_RightParen))));
            continue;
        }
        if (kek_ast_IsPunctuationToken(token,PunctuationKind_LeftBracket)) {
            kek_ast_SelfAddAstChild(&parser->tree,statement,kek_ast_SelfParseDelimited(parser,SelfAstKind_Index,((u64)(PunctuationKind_RightBracket))));
            continue;
        }
        if (kek_ast_SelfShouldParseGenericList(parser,parser->tree.nodes[statement].lastChild)) {
            kek_ast_SelfAddAstChild(&parser->tree,statement,kek_ast_SelfParseGenericDelimited(parser));
            continue;
        }
        kek_ast_SelfAddAstChild(&parser->tree,statement,kek_ast_SelfParseTokenNode(parser));
    }
    kek_ast_SelfFinishLocationFromChildren(&parser->tree,statement);
    return (statement);
}
usize kek_ast_SelfParseList(struct SelfParser* parser,SelfAstKind listKind,u64 closePunctuation) {
    usize list=kek_ast_SelfCreateAstNode(parser,listKind,kek_ast_TokenLocation(parser->tokens[parser->position]));
    kek_ast_SelfParseChildrenInto(parser,list,closePunctuation);
    if (closePunctuation==11&&parser->position<parser->count&&kek_ast_IsClosingPunctuation(&parser->tokens[parser->position])) {
        kek_ast_SelfReportParseError(parser,&parser->tokens[parser->position],"unexpected closing delimiter");
        parser->position+=1;
    }
    kek_ast_SelfFinishLocationFromChildren(&parser->tree,list);
    return (list);
}
Status kek_ast_SelfParseTokens(struct Token* tokens,usize count,struct String source,i64 fileIndex,struct Allocator allocator,struct SelfDiagnosticBag* diagnostics,struct SelfAstTree* out) {
    struct SelfParser parser={0};
    parser.tokens=tokens;
    parser.count=count;
    parser.position=0;
    parser.source=source;
    parser.fileIndex=fileIndex;
    parser.diagnostics=diagnostics;
    parser.errorCount=0;
    Status status=kek_ast_SelfAstTreeInit(&parser.tree,allocator);
    if (status!=Status_Ok) {
        return (status);
    }
    usize root=kek_ast_SelfParseList(&parser,SelfAstKind_File,11);
    parser.tree.root=root;
    parser.tree.nodes[root].location=kek_diagnostics_SelfSourceLocationNew(1,1,0,source.len);
    out->nodes=parser.tree.nodes;
    out->len=parser.tree.len;
    out->cap=parser.tree.cap;
    out->root=parser.tree.root;
    out->allocator=parser.tree.allocator;
    return (Status_Ok);
}
Status kek_ast_SelfWriteAstNodeDump(struct SelfAstTree* tree,usize nodeIndex,usize depth,struct StringBuilder* out) {
    struct SelfAstNode* node=&tree->nodes[nodeIndex];
    Status status=StringBuilder_WriteCString(out,"node|");
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_diagnostics_SelfWriteU64Field(out,((u64)(depth)),1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_diagnostics_SelfWriteU64Field(out,((u64)(node->kind)),1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_diagnostics_SelfWriteU64Field(out,((u64)(node->location.line)),1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_diagnostics_SelfWriteU64Field(out,((u64)(node->location.column)),1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_diagnostics_SelfWriteU64Field(out,((u64)(node->location.offset)),1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_diagnostics_SelfWriteU64Field(out,((u64)(node->location.length)),1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_diagnostics_SelfWriteU64Field(out,((u64)(node->childCount)),1);
    if (status!=Status_Ok) {
        return (status);
    }
    u64 tokenKind=0;
    u64 tokenSubkind=0;
    u64 tokenOffset=0;
    u64 tokenLength=0;
    if (node->kind==SelfAstKind_Token) {
        tokenKind=((u64)(node->token.kind));
        tokenSubkind=node->token.subkind;
        tokenOffset=((u64)(node->token.offset));
        tokenLength=((u64)(node->token.length));
    }
    status=kek_diagnostics_SelfWriteU64Field(out,tokenKind,1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_diagnostics_SelfWriteU64Field(out,tokenSubkind,1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_diagnostics_SelfWriteU64Field(out,tokenOffset,1);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_diagnostics_SelfWriteU64Field(out,tokenLength,0);
    if (status!=Status_Ok) {
        return (status);
    }
    status=StringBuilder_WriteByte(out,'\n');
    if (status!=Status_Ok) {
        return (status);
    }
    usize child=node->firstChild;
    while (child!=0) {
        status=kek_ast_SelfWriteAstNodeDump(tree,child,depth+1,out);
        if (status!=Status_Ok) {
            return (status);
        }
        child=tree->nodes[child].nextSibling;
    }
    return (Status_Ok);
}
Status kek_ast_SelfWriteAstBridgeDump(struct String source,struct StringBuilder* out) {
    struct Allocator allocator=out->allocator;
    struct Array__Token tokens={0};
    Status status=kek_tokenizer_TokenizeToArray(source,0,allocator,&tokens);
    if (status!=Status_Ok) {
        return (status);
    }
    struct SelfDiagnosticBag diagnostics={0};
    kek_diagnostics_SelfDiagnosticBagInit(&diagnostics,allocator);
    struct SelfAstTree tree={0};
    status=kek_ast_SelfParseTokens(tokens.data,tokens.len,source,0,allocator,&diagnostics,&tree);
    if (status==Status_Ok) {
        status=kek_ast_SelfWriteAstNodeDump(&tree,tree.root,0,out);
    }
    if (status==Status_Ok) {
        status=kek_diagnostics_SelfWriteDiagnosticDump(&diagnostics,out);
    }
    kek_ast_SelfAstTreeDestroy(&tree);
    kek_diagnostics_SelfDiagnosticBagDestroy(&diagnostics);
    kek_tokenizer_TokenArrayDestroy(&tokens);
    return (status);
}
bool kek_compiler_SelfCIsOk(Status status) {
    return (status==Status_Ok);
}
struct String kek_compiler_SelfCTokenText(struct SelfCProgram* program,usize fileIndex,usize tokenIndex) {
    struct SelfCTokenFile* file=&program->tokenFiles[fileIndex];
    struct Token token=file->tokens[tokenIndex];
    struct String sourceText=file->sourceText;
    return (String_Slice(&sourceText,token.offset,token.length));
}
bool kek_compiler_SelfCTokenEquals(struct SelfCProgram* program,usize fileIndex,usize tokenIndex,str text) {
    struct String value=kek_compiler_SelfCTokenText(program,fileIndex,tokenIndex);
    return (String_EqualsCString(&value,text));
}
bool kek_compiler_SelfCStringEquals(struct String value,str text) {
    return (String_EqualsCString(&value,text));
}
bool kek_compiler_SelfCIsTokenKind(struct SelfCProgram* program,usize fileIndex,usize tokenIndex,TokenKind kind) {
    return (program->tokenFiles[fileIndex].tokens[tokenIndex].kind==kind);
}
bool kek_compiler_SelfCIsPunctuation(struct SelfCProgram* program,usize fileIndex,usize tokenIndex,PunctuationKind kind) {
    struct Token token=program->tokenFiles[fileIndex].tokens[tokenIndex];
    return (token.kind==TokenKind_Punctuation&&token.subkind==((u64)(kind)));
}
bool kek_compiler_SelfCIsOperator(struct SelfCProgram* program,usize fileIndex,usize tokenIndex,OperatorKind kind) {
    struct Token token=program->tokenFiles[fileIndex].tokens[tokenIndex];
    return (token.kind==TokenKind_Operator&&token.subkind==((u64)(kind)));
}
bool kek_compiler_SelfCIsKeyword(struct SelfCProgram* program,usize fileIndex,usize tokenIndex,KeywordKind kind) {
    struct Token token=program->tokenFiles[fileIndex].tokens[tokenIndex];
    return (token.kind==TokenKind_Keyword&&token.subkind==((u64)(kind)));
}
bool kek_compiler_SelfCIsIdentifierText(struct SelfCProgram* program,usize fileIndex,usize tokenIndex,str text) {
    struct Token token=program->tokenFiles[fileIndex].tokens[tokenIndex];
    if (!(token.kind==TokenKind_Identifier||token.kind==TokenKind_Keyword)) {
        return (0);
    }
    return (kek_compiler_SelfCTokenEquals(program,fileIndex,tokenIndex,text));
}
bool kek_compiler_SelfCIsEof(struct SelfCProgram* program,usize fileIndex,usize tokenIndex) {
    return (program->tokenFiles[fileIndex].tokens[tokenIndex].kind==TokenKind_Eof);
}
usize kek_compiler_SelfCFileTokenCount(struct SelfCProgram* program,usize fileIndex) {
    return (program->tokenFiles[fileIndex].tokenLen);
}
Status kek_compiler_SelfCWrite(struct StringBuilder* out,str text) {
    return (StringBuilder_WriteCString(out,text));
}
Status kek_compiler_SelfCWriteString(struct StringBuilder* out,struct String text) {
    return (StringBuilder_WriteString(out,text));
}
Status kek_compiler_SelfCWriteToken(struct SelfCProgram* program,struct StringBuilder* out,usize fileIndex,usize tokenIndex) {
    return (StringBuilder_WriteString(out,kek_compiler_SelfCTokenText(program,fileIndex,tokenIndex)));
}
Status kek_compiler_SelfCCloneCString(struct SelfCProgram* program,str text,struct OwnedString* out) {
    return (std_string_CloneCString(text,program->allocator,out));
}
Status kek_compiler_SelfCCloneString(struct SelfCProgram* program,struct String text,struct OwnedString* out) {
    return (std_string_CloneString(text,program->allocator,out));
}
struct String kek_compiler_SelfCOwnedView(struct OwnedString* value) {
    return (std_string_OwnedStringView(value));
}
bool kek_compiler_SelfCOwnedEqualsCString(struct OwnedString* value,str text) {
    struct String view=std_string_OwnedStringView(value);
    return (String_EqualsCString(&view,text));
}
bool kek_compiler_SelfCOwnedEquals(struct OwnedString* left,struct OwnedString* right) {
    struct String leftView=std_string_OwnedStringView(left);
    struct String rightView=std_string_OwnedStringView(right);
    return (String_Equals(&leftView,rightView));
}
Status kek_compiler_SelfCDetachBuilder(struct StringBuilder* builder,struct OwnedString* out) {
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
Status kek_compiler_SelfCMakeOwnedEmpty(struct SelfCProgram* program,struct OwnedString* out) {
    return (kek_compiler_SelfCCloneCString(program,"",out));
}
Status kek_compiler_SelfCWriteOwned(struct StringBuilder* out,struct OwnedString* text) {
    return (StringBuilder_WriteString(out,std_string_OwnedStringView(text)));
}
usize kek_compiler_SelfCStringLastSlash(struct String path) {
    usize last=path.len;
    for (usize i=0;i<path.len;i++) {
        if (path.data[i]=='/') {
            last=i;
        }
    }
    return (last);
}
Status kek_compiler_SelfCPackageNameFromPath(struct String path,struct Allocator allocator,struct OwnedString* out) {
    usize lastSlash=kek_compiler_SelfCStringLastSlash(path);
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
Status kek_compiler_SelfCModuleNameFromPath(struct String path,struct Allocator allocator,struct OwnedString* out) {
    usize lastSlash=kek_compiler_SelfCStringLastSlash(path);
    if (lastSlash==path.len) {
        return (std_string_CloneCString("",allocator,out));
    }
    struct String name=String_Slice(&path,lastSlash+1,path.len-lastSlash-1);
    if (String_EndsWith(&name,std_string_StringFromCString(".kek"))) {
        return (std_string_CloneString(String_Slice(&name,0,name.len-4),allocator,out));
    }
    return (std_string_CloneString(name,allocator,out));
}
Status kek_compiler_SelfCInitTokenFileSlot(struct Allocator allocator,struct SelfCTokenFile* tokenFile,usize fileIndex) {
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
Status kek_compiler_SelfCSetTokenFileNames(struct Allocator allocator,struct SelfCTokenFile* tokenFile,bool isRoot) {
    std_string_DestroyOwnedString(&tokenFile->packageName);
    std_string_DestroyOwnedString(&tokenFile->moduleName);
    if (isRoot) {
        Status status=std_string_CloneCString("",allocator,&tokenFile->packageName);
        if (status==Status_Ok) {
            status=std_string_CloneCString("",allocator,&tokenFile->moduleName);
        }
        return (status);
    }
    Status status=kek_compiler_SelfCPackageNameFromPath(tokenFile->path,allocator,&tokenFile->packageName);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCModuleNameFromPath(tokenFile->path,allocator,&tokenFile->moduleName);
    }
    return (status);
}
Status kek_compiler_SelfCTokenizeSourceFile(struct Allocator allocator,struct SelfSourceFile* sourceFile,struct SelfCTokenFile* tokenFile,usize fileIndex,bool isRoot,struct SelfDiagnosticBag* diagnostics) {
    tokenFile->sourceText=std_string_OwnedStringView(&sourceFile->content);
    tokenFile->path=std_string_OwnedStringView(&sourceFile->path);
    tokenFile->fileIndex=fileIndex;
    Status status=kek_compiler_SelfCSetTokenFileNames(allocator,tokenFile,isRoot);
    if (status!=Status_Ok) {
        return (status);
    }
    struct Array__Token tokens={0};
    status=kek_tokenizer_TokenizeToArray(tokenFile->sourceText,0,allocator,&tokens);
    if (status!=Status_Ok) {
        kek_tokenizer_TokenArrayDestroy(&tokens);
        return (status);
    }
    tokenFile->tokens=tokens.data;
    tokenFile->tokenLen=tokens.len;
    tokenFile->tokenCap=tokens.cap;
    struct SelfAstTree tree={0};
    status=kek_ast_SelfParseTokens(tokenFile->tokens,tokenFile->tokenLen,tokenFile->sourceText,((i64)(fileIndex)),allocator,diagnostics,&tree);
    if (status==Status_Ok) {
        kek_ast_SelfAstTreeDestroy(&tree);
    }
    return (status);
}
void kek_compiler_SelfCTokenizeJobRun(struct SelfCTokenizeJob* job) {
    job->status=kek_compiler_SelfCTokenizeSourceFile(job->allocator,job->sourceFile,job->tokenFile,job->fileIndex,job->isRoot,&job->diagnostics);
}
ptr kek_compiler_SelfCTokenizeFileThreadEntry(ptr arg) {
    struct SelfCTokenizeJob* job=((struct SelfCTokenizeJob*)(arg));
    kek_compiler_SelfCTokenizeJobRun(job);
    return (0);
}
Status kek_compiler_SelfCMergeDiagnostics(struct SelfDiagnosticBag* target,struct SelfDiagnosticBag* source) {
    for (usize i=0;i<source->count;i++) {
        struct SelfDiagnostic* item=&source->items[i];
        Status status=kek_diagnostics_SelfDiagnosticAdd(target,item->severity,item->phase,item->fileIndex,item->location,std_string_OwnedStringView(&item->message));
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
Status kek_compiler_SelfCProgramInit(struct SelfCProgram* program,struct Allocator allocator) {
    program->allocator=allocator;
    kek_diagnostics_SelfDiagnosticBagInit(&program->diagnostics,allocator);
    kek_source_SelfFileTableInit(&program->files,allocator);
    program->tokenFileCount=0;
    program->declCount=0;
    program->paramCount=0;
    program->fieldCount=0;
    program->typeUseCount=0;
    program->funcUseCount=0;
    return (Status_Ok);
}
Status kek_compiler_SelfCProgramDestroy(struct SelfCProgram* program) {
    for (usize i=0;i<program->tokenFileCount;i++) {
        struct Array__Token tokens={0};
        tokens.data=program->tokenFiles[i].tokens;
        tokens.len=program->tokenFiles[i].tokenLen;
        tokens.cap=program->tokenFiles[i].tokenCap;
        tokens.allocator=program->allocator;
        kek_tokenizer_TokenArrayDestroy(&tokens);
        std_string_DestroyOwnedString(&program->tokenFiles[i].packageName);
        std_string_DestroyOwnedString(&program->tokenFiles[i].moduleName);
    }
    for (usize i=0;i<program->declCount;i++) {
        std_string_DestroyOwnedString(&program->decls[i].packageName);
        std_string_DestroyOwnedString(&program->decls[i].moduleName);
    }
    for (usize i=0;i<program->typeUseCount;i++) {
        std_string_DestroyOwnedString(&program->typeUses[i].key);
        std_string_DestroyOwnedString(&program->typeUses[i].cName);
        std_string_DestroyOwnedString(&program->typeUses[i].baseName);
        std_string_DestroyOwnedString(&program->typeUses[i].arg0);
        std_string_DestroyOwnedString(&program->typeUses[i].arg1);
        std_string_DestroyOwnedString(&program->typeUses[i].arg2);
    }
    for (usize i=0;i<program->funcUseCount;i++) {
        std_string_DestroyOwnedString(&program->funcUses[i].key);
        std_string_DestroyOwnedString(&program->funcUses[i].cName);
        std_string_DestroyOwnedString(&program->funcUses[i].arg0);
        std_string_DestroyOwnedString(&program->funcUses[i].arg1);
        std_string_DestroyOwnedString(&program->funcUses[i].arg2);
    }
    kek_source_SelfFileTableDestroy(&program->files);
    kek_diagnostics_SelfDiagnosticBagDestroy(&program->diagnostics);
    return (Status_Ok);
}
Status kek_compiler_SelfCAddDiagnostic(struct SelfCProgram* program,str message) {
    return (kek_diagnostics_SelfDiagnosticAddCString(&program->diagnostics,SelfDiagnosticSeverity_Error,SelfDiagnosticPhase_Semantic,-1,kek_diagnostics_SelfSourceLocationNew(0,0,0,0),message));
}
Status kek_compiler_SelfCLoadAndTokenize(struct SelfCProgram* program,str entryPath) {
    Status status=kek_source_SelfLoadCompilationSources(entryPath,program->allocator,&program->files,&program->diagnostics);
    if (status!=Status_Ok) {
        return (status);
    }
    if (program->files.count>(sizeof(program->tokenFiles)/sizeof((program->tokenFiles)[0]))) {
        return (kek_compiler_SelfCAddDiagnostic(program,"too many source files"));
    }
    for (usize i=0;i<program->files.count;i++) {
        status=kek_compiler_SelfCInitTokenFileSlot(program->allocator,&program->tokenFiles[i],i);
        if (status!=Status_Ok) {
            program->tokenFileCount=i;
            return (status);
        }
    }
    program->tokenFileCount=program->files.count;
    struct SelfCTokenizeJob jobs[96];
    for (usize i=0;i<program->files.count;i++) {
        struct SelfCTokenizeJob* job=&jobs[i];
        job->allocator=program->allocator;
        job->sourceFile=&program->files.files[i];
        job->tokenFile=&program->tokenFiles[i];
        kek_diagnostics_SelfDiagnosticBagInit(&job->diagnostics,program->allocator);
        job->status=Status_Ok;
        job->fileIndex=i;
        job->isRoot=i==0;
        job->threaded=0;
        Status startStatus=std_thread_ThreadHandleStart(&job->threadHandle,((ptr)(kek_compiler_SelfCTokenizeFileThreadEntry)),((ptr)(job)));
        if (startStatus==Status_Ok) {
            job->threaded=1;
        } else {
            kek_compiler_SelfCTokenizeJobRun(job);
        }
    }
    Status firstStatus=Status_Ok;
    for (usize i=0;i<program->files.count;i++) {
        struct SelfCTokenizeJob* job=&jobs[i];
        if (job->threaded) {
            Status joinStatus=std_thread_ThreadHandleJoin(&job->threadHandle);
            if (joinStatus!=Status_Ok&&job->status==Status_Ok) {
                job->status=joinStatus;
            }
        }
        Status mergeStatus=kek_compiler_SelfCMergeDiagnostics(&program->diagnostics,&job->diagnostics);
        if (firstStatus==Status_Ok&&mergeStatus!=Status_Ok) {
            firstStatus=mergeStatus;
        }
        if (firstStatus==Status_Ok&&job->status!=Status_Ok) {
            firstStatus=job->status;
        }
        kek_diagnostics_SelfDiagnosticBagDestroy(&job->diagnostics);
    }
    return (firstStatus);
}
int kek_compiler_SelfCompilerFail(str text) {
    struct File stderr=std_file_Stderr();
    std_format_WriteStringToFile(&stderr,std_string_StringFromCString(text));
    return (1);
}
int kek_compiler_SelfCompilerPrintHelp(void) {
    struct File stdout=std_file_Stdout();
    std_format_WriteStringToFile(&stdout,std_string_StringFromCString("kek build <input.kek> -o <output.c>\n"));
    std_format_WriteStringToFile(&stdout,std_string_StringFromCString("kek --help\n"));
    std_format_WriteStringToFile(&stdout,std_string_StringFromCString("kek --version\n"));
    return (0);
}
Status kek_compiler_SelfCWriteDiagnostics(struct SelfCProgram* program) {
    struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
    Status status=kek_diagnostics_SelfWriteDiagnosticDump(&program->diagnostics,&builder);
    if (status==Status_Ok) {
        struct File stderr=std_file_Stderr();
        struct String view=StringBuilder_View(&builder);
        struct Result__usize write=File_Write(&stderr,String_Bytes(&view));
        status=write.status;
    }
    StringBuilder_Destroy(&builder);
    return (status);
}
bool kek_compiler_SelfCIsOpenDelimiter(struct SelfCProgram* program,usize fileIndex,usize index) {
    return (kek_compiler_SelfCIsPunctuation(program,fileIndex,index,PunctuationKind_LeftParen)||kek_compiler_SelfCIsPunctuation(program,fileIndex,index,PunctuationKind_LeftBrace)||kek_compiler_SelfCIsPunctuation(program,fileIndex,index,PunctuationKind_LeftBracket)||kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_Less));
}
bool kek_compiler_SelfCDelimiterMatches(struct SelfCProgram* program,usize fileIndex,usize openIndex,usize closeIndex) {
    if (kek_compiler_SelfCIsPunctuation(program,fileIndex,openIndex,PunctuationKind_LeftParen)) {
        return (kek_compiler_SelfCIsPunctuation(program,fileIndex,closeIndex,PunctuationKind_RightParen));
    }
    if (kek_compiler_SelfCIsPunctuation(program,fileIndex,openIndex,PunctuationKind_LeftBrace)) {
        return (kek_compiler_SelfCIsPunctuation(program,fileIndex,closeIndex,PunctuationKind_RightBrace));
    }
    if (kek_compiler_SelfCIsPunctuation(program,fileIndex,openIndex,PunctuationKind_LeftBracket)) {
        return (kek_compiler_SelfCIsPunctuation(program,fileIndex,closeIndex,PunctuationKind_RightBracket));
    }
    if (kek_compiler_SelfCIsOperator(program,fileIndex,openIndex,OperatorKind_Less)) {
        return (kek_compiler_SelfCIsOperator(program,fileIndex,closeIndex,OperatorKind_Greater));
    }
    return (0);
}
usize kek_compiler_SelfCFindMatching(struct SelfCProgram* program,usize fileIndex,usize openIndex) {
    usize count=kek_compiler_SelfCFileTokenCount(program,fileIndex);
    usize depth=0;
    bool isParen=kek_compiler_SelfCIsPunctuation(program,fileIndex,openIndex,PunctuationKind_LeftParen);
    bool isBrace=kek_compiler_SelfCIsPunctuation(program,fileIndex,openIndex,PunctuationKind_LeftBrace);
    bool isBracket=kek_compiler_SelfCIsPunctuation(program,fileIndex,openIndex,PunctuationKind_LeftBracket);
    bool isGeneric=kek_compiler_SelfCIsOperator(program,fileIndex,openIndex,OperatorKind_Less);
    for (usize i=openIndex;i<count;i++) {
        if ((isParen&&kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftParen))||(isBrace&&kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftBrace))||(isBracket&&kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftBracket))||(isGeneric&&kek_compiler_SelfCIsOperator(program,fileIndex,i,OperatorKind_Less))) {
            depth+=1;
            continue;
        }
        if ((isParen&&kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_RightParen))||(isBrace&&kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_RightBrace))||(isBracket&&kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_RightBracket))||(isGeneric&&kek_compiler_SelfCIsOperator(program,fileIndex,i,OperatorKind_Greater))) {
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
usize kek_compiler_SelfCSkipDelimited(struct SelfCProgram* program,usize fileIndex,usize index) {
    usize match=kek_compiler_SelfCFindMatching(program,fileIndex,index);
    if (match>=kek_compiler_SelfCFileTokenCount(program,fileIndex)) {
        return (index+1);
    }
    return (match+1);
}
usize kek_compiler_SelfCSkipAttributes(struct SelfCProgram* program,usize fileIndex,usize index) {
    while (index<kek_compiler_SelfCFileTokenCount(program,fileIndex)&&kek_compiler_SelfCIsPunctuation(program,fileIndex,index,PunctuationKind_LeftBracket)) {
        index=kek_compiler_SelfCSkipDelimited(program,fileIndex,index);
    }
    return (index);
}
usize kek_compiler_SelfCFindTopLevelColon(struct SelfCProgram* program,usize fileIndex,usize start,usize end) {
    usize parenDepth=0;
    usize braceDepth=0;
    usize bracketDepth=0;
    usize genericDepth=0;
    for (usize i=start;i<end;i++) {
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftParen)) {
            parenDepth+=1;
            continue;
        }
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_RightParen)) {
            if (parenDepth>0) {
                parenDepth-=1;
            }
            continue;
        }
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftBrace)) {
            braceDepth+=1;
            continue;
        }
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_RightBrace)) {
            if (braceDepth>0) {
                braceDepth-=1;
            }
            continue;
        }
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftBracket)) {
            bracketDepth+=1;
            continue;
        }
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_RightBracket)) {
            if (bracketDepth>0) {
                bracketDepth-=1;
            }
            continue;
        }
        if (kek_compiler_SelfCIsOperator(program,fileIndex,i,OperatorKind_Less)) {
            genericDepth+=1;
            continue;
        }
        if (kek_compiler_SelfCIsOperator(program,fileIndex,i,OperatorKind_Greater)) {
            if (genericDepth>0) {
                genericDepth-=1;
            }
            continue;
        }
        if (parenDepth==0&&braceDepth==0&&bracketDepth==0&&genericDepth==0&&kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_Colon)) {
            return (i);
        }
    }
    return (end);
}
usize kek_compiler_SelfCFindTokenAtDepthZero(struct SelfCProgram* program,usize fileIndex,usize start,usize end,PunctuationKind punctuation) {
    usize parenDepth=0;
    usize braceDepth=0;
    usize bracketDepth=0;
    usize genericDepth=0;
    for (usize i=start;i<end;i++) {
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftParen)) {
            parenDepth+=1;
            continue;
        }
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_RightParen)) {
            if (parenDepth>0) {
                parenDepth-=1;
            }
            continue;
        }
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftBrace)) {
            braceDepth+=1;
            continue;
        }
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_RightBrace)) {
            if (braceDepth>0) {
                braceDepth-=1;
            }
            continue;
        }
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftBracket)) {
            bracketDepth+=1;
            continue;
        }
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_RightBracket)) {
            if (bracketDepth>0) {
                bracketDepth-=1;
            }
            continue;
        }
        if (parenDepth==0&&braceDepth==0&&bracketDepth==0&&genericDepth==0&&kek_compiler_SelfCIsPunctuation(program,fileIndex,i,punctuation)) {
            return (i);
        }
    }
    return (end);
}
usize kek_compiler_SelfCFindOperatorScope(struct SelfCProgram* program,usize fileIndex,usize start,usize end) {
    usize genericDepth=0;
    for (usize i=start;i<end;i++) {
        if (kek_compiler_SelfCIsOperator(program,fileIndex,i,OperatorKind_Less)) {
            genericDepth+=1;
            continue;
        }
        if (kek_compiler_SelfCIsOperator(program,fileIndex,i,OperatorKind_Greater)) {
            if (genericDepth>0) {
                genericDepth-=1;
            }
            continue;
        }
        if (genericDepth==0&&kek_compiler_SelfCIsOperator(program,fileIndex,i,OperatorKind_Scope)) {
            return (i);
        }
    }
    return (end);
}
usize kek_compiler_SelfCFindNextGroup(struct SelfCProgram* program,usize fileIndex,usize start,usize end) {
    for (usize i=start;i<end;i++) {
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftParen)) {
            return (i);
        }
    }
    return (end);
}
usize kek_compiler_SelfCFindNextBlock(struct SelfCProgram* program,usize fileIndex,usize start,usize end) {
    for (usize i=start;i<end;i++) {
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftBrace)) {
            return (i);
        }
    }
    return (end);
}
u8 kek_compiler_SelfCOperatorCode(struct SelfCProgram* program,usize fileIndex,usize index) {
    struct Token token=program->tokenFiles[fileIndex].tokens[index];
    if (token.kind==TokenKind_Operator) {
        return (((u8)(token.subkind+1)));
    }
    return (0);
}
Status kek_compiler_SelfCAddDecl(struct SelfCProgram* program,struct SelfCDecl* decl) {
    if (program->declCount>=(sizeof(program->decls)/sizeof((program->decls)[0]))) {
        return (kek_compiler_SelfCAddDiagnostic(program,"too many declarations"));
    }
    program->decls[program->declCount]=decl[0];
    program->declCount+=1;
    return (Status_Ok);
}
Status kek_compiler_SelfCAddParam(struct SelfCProgram* program,struct SelfCParam* param) {
    if (program->paramCount>=(sizeof(program->params)/sizeof((program->params)[0]))) {
        return (kek_compiler_SelfCAddDiagnostic(program,"too many parameters"));
    }
    program->params[program->paramCount]=param[0];
    program->paramCount+=1;
    return (Status_Ok);
}
Status kek_compiler_SelfCAddField(struct SelfCProgram* program,struct SelfCField* field) {
    if (program->fieldCount>=(sizeof(program->fields)/sizeof((program->fields)[0]))) {
        return (kek_compiler_SelfCAddDiagnostic(program,"too many fields"));
    }
    program->fields[program->fieldCount]=field[0];
    program->fieldCount+=1;
    return (Status_Ok);
}
Status kek_compiler_SelfCParseParams(struct SelfCProgram* program,struct SelfCDecl* decl,usize fileIndex,usize start,usize end) {
    decl->firstParam=program->paramCount;
    decl->paramCount=0;
    usize itemStart=start;
    while (itemStart<end) {
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,itemStart,PunctuationKind_Comma)) {
            itemStart+=1;
            continue;
        }
        usize itemEnd=kek_compiler_SelfCFindTokenAtDepthZero(program,fileIndex,itemStart,end,PunctuationKind_Comma);
        usize colon=kek_compiler_SelfCFindTopLevelColon(program,fileIndex,itemStart,itemEnd);
        if (colon>=itemEnd) {
            break;
        }
        struct SelfCParam param={0};
        param.fileIndex=fileIndex;
        param.typeStart=itemStart;
        param.typeEnd=colon;
        param.nameIndex=colon+1;
        param.defaultStart=itemEnd;
        param.defaultEnd=itemEnd;
        param.hasDefault=0;
        for (usize i=param.nameIndex+1;i<itemEnd;i++) {
            if (kek_compiler_SelfCIsOperator(program,fileIndex,i,OperatorKind_Assign)) {
                param.defaultStart=i+1;
                param.defaultEnd=itemEnd;
                param.hasDefault=1;
                break;
            }
        }
        Status status=kek_compiler_SelfCAddParam(program,&param);
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
Status kek_compiler_SelfCParseFields(struct SelfCProgram* program,struct SelfCDecl* decl,usize fileIndex,usize start,usize end) {
    decl->firstField=program->fieldCount;
    decl->fieldCount=0;
    usize itemStart=start;
    while (itemStart<end) {
        itemStart=kek_compiler_SelfCSkipAttributes(program,fileIndex,itemStart);
        if (itemStart>=end) {
            break;
        }
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,itemStart,PunctuationKind_Semicolon)) {
            itemStart+=1;
            continue;
        }
        struct SelfCField field={0};
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
        if (kek_compiler_SelfCIsKeyword(program,fileIndex,itemStart,KeywordKind_Struct)) {
            usize colon=itemStart+1;
            if (colon<end&&kek_compiler_SelfCIsPunctuation(program,fileIndex,colon,PunctuationKind_Colon)) {
                usize nameIndex=colon+1;
                usize blockStart=kek_compiler_SelfCFindNextBlock(program,fileIndex,nameIndex+1,end);
                usize blockEnd=kek_compiler_SelfCFindMatching(program,fileIndex,blockStart);
                field.isNestedStruct=1;
                field.nameIndex=nameIndex;
                field.nestedBodyStart=blockStart+1;
                field.nestedBodyEnd=blockEnd;
                Status addNested=kek_compiler_SelfCAddField(program,&field);
                if (addNested!=Status_Ok) {
                    return (addNested);
                }
                decl->fieldCount+=1;
                itemStart=blockEnd+1;
                if (itemStart<end&&kek_compiler_SelfCIsPunctuation(program,fileIndex,itemStart,PunctuationKind_Semicolon)) {
                    itemStart+=1;
                }
                continue;
            }
        }
        usize itemEnd=itemStart;
        while (itemEnd<end&&!kek_compiler_SelfCIsPunctuation(program,fileIndex,itemEnd,PunctuationKind_Semicolon)) {
            if (kek_compiler_SelfCIsPunctuation(program,fileIndex,itemEnd,PunctuationKind_LeftBrace)||kek_compiler_SelfCIsPunctuation(program,fileIndex,itemEnd,PunctuationKind_LeftParen)||kek_compiler_SelfCIsPunctuation(program,fileIndex,itemEnd,PunctuationKind_LeftBracket)) {
                itemEnd=kek_compiler_SelfCSkipDelimited(program,fileIndex,itemEnd);
                continue;
            }
            itemEnd+=1;
        }
        usize colon=kek_compiler_SelfCFindTopLevelColon(program,fileIndex,itemStart,itemEnd);
        if (colon>=itemEnd) {
            itemStart=itemEnd+1;
            continue;
        }
        field.typeStart=itemStart;
        field.typeEnd=colon;
        field.nameIndex=colon+1;
        usize afterName=field.nameIndex+1;
        if (afterName<itemEnd&&kek_compiler_SelfCIsPunctuation(program,fileIndex,afterName,PunctuationKind_LeftBracket)) {
            field.isArray=1;
            field.arrayStart=afterName+1;
            field.arrayEnd=kek_compiler_SelfCFindMatching(program,fileIndex,afterName);
            afterName=field.arrayEnd+1;
        }
        for (usize i=afterName;i<itemEnd;i++) {
            if (kek_compiler_SelfCIsOperator(program,fileIndex,i,OperatorKind_Assign)) {
                field.hasDefault=1;
                field.defaultStart=i+1;
                field.defaultEnd=itemEnd;
                break;
            }
        }
        Status status=kek_compiler_SelfCAddField(program,&field);
        if (status!=Status_Ok) {
            return (status);
        }
        decl->fieldCount+=1;
        itemStart=itemEnd+1;
    }
    return (Status_Ok);
}
bool kek_compiler_SelfCDeclNameMatches(struct SelfCProgram* program,struct SelfCDecl* decl,struct String name) {
    if (decl->nameIndex>=kek_compiler_SelfCFileTokenCount(program,decl->fileIndex)) {
        return (0);
    }
    struct String declName=kek_compiler_SelfCTokenText(program,decl->fileIndex,decl->nameIndex);
    return (String_Equals(&declName,name));
}
struct SelfCDecl* kek_compiler_SelfCFindTypeDecl(struct SelfCProgram* program,struct String name) {
    for (usize i=0;i<program->declCount;i++) {
        struct SelfCDecl* decl=&program->decls[i];
        if ((decl->kind==SelfCDeclKind_Struct||decl->kind==SelfCDeclKind_Union||decl->kind==SelfCDeclKind_Enum||decl->kind==SelfCDeclKind_Alias)&&kek_compiler_SelfCDeclNameMatches(program,decl,name)) {
            return (decl);
        }
    }
    return (0);
}
Status kek_compiler_SelfCParseAliasDecl(struct SelfCProgram* program,usize fileIndex,usize start,usize* outEnd) {
    usize count=kek_compiler_SelfCFileTokenCount(program,fileIndex);
    usize end=start;
    while (end<count&&!kek_compiler_SelfCIsPunctuation(program,fileIndex,end,PunctuationKind_Semicolon)&&!kek_compiler_SelfCIsEof(program,fileIndex,end)) {
        end+=1;
    }
    struct SelfCDecl decl={0};
    decl.kind=SelfCDeclKind_Alias;
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
    decl.emitted=0;
    Status clone=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&program->tokenFiles[fileIndex].packageName),&decl.packageName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    clone=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&program->tokenFiles[fileIndex].moduleName),&decl.moduleName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    for (usize i=start+3;i<end;i++) {
        if (kek_compiler_SelfCIsOperator(program,fileIndex,i,OperatorKind_Assign)) {
            decl.returnStart=i+1;
            decl.returnEnd=end;
            break;
        }
    }
    Status status=kek_compiler_SelfCAddDecl(program,&decl);
    outEnd[0]=end+1;
    return (status);
}
Status kek_compiler_SelfCParseTypeDecl(struct SelfCProgram* program,usize fileIndex,usize start,SelfCDeclKind kind,usize* outEnd) {
    usize count=kek_compiler_SelfCFileTokenCount(program,fileIndex);
    usize colon=start+1;
    usize nameIndex=colon+1;
    if (kind==SelfCDeclKind_Enum) {
        if (colon<count&&kek_compiler_SelfCIsPunctuation(program,fileIndex,colon,PunctuationKind_Colon)) {
            usize secondColon=kek_compiler_SelfCFindTopLevelColon(program,fileIndex,colon+1,count);
            if (secondColon<count) {
                nameIndex=secondColon+1;
            }
        }
    }
    usize genericStart=count;
    usize genericEnd=count;
    if (nameIndex+1<count&&kek_compiler_SelfCIsOperator(program,fileIndex,nameIndex+1,OperatorKind_Less)) {
        genericStart=nameIndex+1;
        genericEnd=kek_compiler_SelfCFindMatching(program,fileIndex,genericStart);
    }
    usize blockStart=kek_compiler_SelfCFindNextBlock(program,fileIndex,nameIndex+1,count);
    usize blockEnd=kek_compiler_SelfCFindMatching(program,fileIndex,blockStart);
    struct SelfCDecl decl={0};
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
    decl.emitted=0;
    Status clone=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&program->tokenFiles[fileIndex].packageName),&decl.packageName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    clone=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&program->tokenFiles[fileIndex].moduleName),&decl.moduleName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    if (kind==SelfCDeclKind_Struct||kind==SelfCDeclKind_Union) {
        Status fields=kek_compiler_SelfCParseFields(program,&decl,fileIndex,blockStart+1,blockEnd);
        if (fields!=Status_Ok) {
            return (fields);
        }
    }
    Status status=kek_compiler_SelfCAddDecl(program,&decl);
    outEnd[0]=blockEnd+1;
    if (outEnd[0]<count&&kek_compiler_SelfCIsPunctuation(program,fileIndex,outEnd[0],PunctuationKind_Semicolon)) {
        outEnd[0]+=1;
    }
    return (status);
}
Status kek_compiler_SelfCParseExternDecl(struct SelfCProgram* program,usize fileIndex,usize start,usize* outEnd) {
    usize count=kek_compiler_SelfCFileTokenCount(program,fileIndex);
    usize blockStart=kek_compiler_SelfCFindNextBlock(program,fileIndex,start,count);
    usize blockEnd=kek_compiler_SelfCFindMatching(program,fileIndex,blockStart);
    struct SelfCDecl decl={0};
    decl.kind=SelfCDeclKind_ExternC;
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
    decl.emitted=0;
    Status clone=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&program->tokenFiles[fileIndex].packageName),&decl.packageName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    clone=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&program->tokenFiles[fileIndex].moduleName),&decl.moduleName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    Status status=kek_compiler_SelfCAddDecl(program,&decl);
    outEnd[0]=blockEnd+1;
    return (status);
}
Status kek_compiler_SelfCParseFunctionDecl(struct SelfCProgram* program,usize fileIndex,usize start,usize* outEnd) {
    usize count=kek_compiler_SelfCFileTokenCount(program,fileIndex);
    usize groupStart=kek_compiler_SelfCFindNextGroup(program,fileIndex,start,count);
    if (groupStart>=count) {
        outEnd[0]=start+1;
        return (Status_Ok);
    }
    usize groupEnd=kek_compiler_SelfCFindMatching(program,fileIndex,groupStart);
    usize colon=kek_compiler_SelfCFindTopLevelColon(program,fileIndex,start,groupStart);
    if (colon>=groupStart) {
        outEnd[0]=groupEnd+1;
        return (Status_Ok);
    }
    usize blockStart=groupEnd+1;
    bool hasBody=0;
    usize blockEnd=groupEnd;
    if (blockStart<count&&kek_compiler_SelfCIsPunctuation(program,fileIndex,blockStart,PunctuationKind_LeftBrace)) {
        hasBody=1;
        blockEnd=kek_compiler_SelfCFindMatching(program,fileIndex,blockStart);
    }
    struct SelfCDecl decl={0};
    decl.kind=SelfCDeclKind_Function;
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
    decl.emitted=0;
    Status clone=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&program->tokenFiles[fileIndex].packageName),&decl.packageName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    clone=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&program->tokenFiles[fileIndex].moduleName),&decl.moduleName);
    if (clone!=Status_Ok) {
        return (clone);
    }
    usize scope=kek_compiler_SelfCFindOperatorScope(program,fileIndex,colon+1,groupStart);
    if (scope<groupStart) {
        decl.hasReceiver=1;
        decl.receiverStart=colon+1;
        decl.receiverEnd=scope;
        decl.nameIndex=scope+1;
    }
    if (decl.nameIndex<groupStart&&kek_compiler_SelfCIsIdentifierText(program,fileIndex,decl.nameIndex,"operator")) {
        decl.isOperator=1;
        decl.operatorCode=kek_compiler_SelfCOperatorCode(program,fileIndex,decl.nameIndex+1);
    }
    usize genericProbe=decl.nameIndex+1;
    if (decl.isOperator) {
        genericProbe=decl.nameIndex+2;
    }
    if (genericProbe<groupStart&&kek_compiler_SelfCIsOperator(program,fileIndex,genericProbe,OperatorKind_Less)) {
        decl.genericStart=genericProbe;
        decl.genericEnd=kek_compiler_SelfCFindMatching(program,fileIndex,genericProbe);
        decl.isGeneric=1;
    }
    for (usize i=decl.receiverStart;i<decl.receiverEnd;i++) {
        if (kek_compiler_SelfCIsOperator(program,fileIndex,i,OperatorKind_Less)) {
            decl.isGeneric=1;
        }
    }
    Status params=kek_compiler_SelfCParseParams(program,&decl,fileIndex,groupStart+1,groupEnd);
    if (params!=Status_Ok) {
        return (params);
    }
    Status status=kek_compiler_SelfCAddDecl(program,&decl);
    outEnd[0]=blockEnd+1;
    if (outEnd[0]<count&&kek_compiler_SelfCIsPunctuation(program,fileIndex,outEnd[0],PunctuationKind_Semicolon)) {
        outEnd[0]+=1;
    }
    return (status);
}
Status kek_compiler_SelfCParseDeclarations(struct SelfCProgram* program) {
    for (usize fileIndex=0;fileIndex<program->tokenFileCount;fileIndex++) {
        usize index=0;
        usize count=kek_compiler_SelfCFileTokenCount(program,fileIndex);
        while (index<count&&!kek_compiler_SelfCIsEof(program,fileIndex,index)) {
            index=kek_compiler_SelfCSkipAttributes(program,fileIndex,index);
            if (index>=count||kek_compiler_SelfCIsEof(program,fileIndex,index)) {
                break;
            }
            usize next=index+1;
            Status status=Status_Ok;
            if (kek_compiler_SelfCIsPunctuation(program,fileIndex,index,PunctuationKind_Hash)) {
                usize groupStart=kek_compiler_SelfCFindNextGroup(program,fileIndex,index,count);
                if (groupStart<count) {
                    next=kek_compiler_SelfCSkipDelimited(program,fileIndex,groupStart);
                    if (next<count&&kek_compiler_SelfCIsPunctuation(program,fileIndex,next,PunctuationKind_Semicolon)) {
                        next+=1;
                    }
                }
            } else {
                if (kek_compiler_SelfCIsKeyword(program,fileIndex,index,KeywordKind_Extern)) {
                    status=kek_compiler_SelfCParseExternDecl(program,fileIndex,index,&next);
                } else {
                    if (kek_compiler_SelfCIsKeyword(program,fileIndex,index,KeywordKind_Alias)||kek_compiler_SelfCIsIdentifierText(program,fileIndex,index,"alias")) {
                        status=kek_compiler_SelfCParseAliasDecl(program,fileIndex,index,&next);
                    } else {
                        if (kek_compiler_SelfCIsKeyword(program,fileIndex,index,KeywordKind_Struct)||kek_compiler_SelfCIsIdentifierText(program,fileIndex,index,"struct")) {
                            status=kek_compiler_SelfCParseTypeDecl(program,fileIndex,index,SelfCDeclKind_Struct,&next);
                        } else {
                            if (kek_compiler_SelfCIsKeyword(program,fileIndex,index,KeywordKind_Union)||kek_compiler_SelfCIsIdentifierText(program,fileIndex,index,"union")) {
                                status=kek_compiler_SelfCParseTypeDecl(program,fileIndex,index,SelfCDeclKind_Union,&next);
                            } else {
                                if (kek_compiler_SelfCIsKeyword(program,fileIndex,index,KeywordKind_Enum)||kek_compiler_SelfCIsIdentifierText(program,fileIndex,index,"enum")) {
                                    status=kek_compiler_SelfCParseTypeDecl(program,fileIndex,index,SelfCDeclKind_Enum,&next);
                                } else {
                                    status=kek_compiler_SelfCParseFunctionDecl(program,fileIndex,index,&next);
                                }
                            }
                        }
                    }
                }
            }
            if (status!=Status_Ok) {
                return (status);
            }
            if (next<=index) {
                next=index+1;
            }
            index=next;
        }
    }
    return (Status_Ok);
}
Status kek_compiler_SelfCWriteTokenRangeRaw(struct SelfCProgram* program,struct StringBuilder* out,usize fileIndex,usize start,usize end) {
    for (usize i=start;i<end;i++) {
        Status status=kek_compiler_SelfCWriteToken(program,out,fileIndex,i);
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
bool kek_compiler_SelfCIsBuiltinTypeName(struct String name) {
    return (String_EqualsCString(&name,"void")||String_EqualsCString(&name,"bool")||String_EqualsCString(&name,"int")||String_EqualsCString(&name,"u8")||String_EqualsCString(&name,"u16")||String_EqualsCString(&name,"u32")||String_EqualsCString(&name,"u64")||String_EqualsCString(&name,"i8")||String_EqualsCString(&name,"i16")||String_EqualsCString(&name,"i32")||String_EqualsCString(&name,"i64")||String_EqualsCString(&name,"f32")||String_EqualsCString(&name,"f64")||String_EqualsCString(&name,"ptr")||String_EqualsCString(&name,"str")||String_EqualsCString(&name,"byte")||String_EqualsCString(&name,"usize")||String_EqualsCString(&name,"isize")||String_EqualsCString(&name,"RawHandle"));
}
Status kek_compiler_SelfCWriteSanitized(struct StringBuilder* out,struct String text) {
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
Status kek_compiler_SelfCWriteOperatorName(struct StringBuilder* out,u8 operatorCode) {
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
bool kek_compiler_SelfCDeclPackageIsRoot(struct SelfCDecl* decl) {
    struct String packageName=std_string_OwnedStringView(&decl->packageName);
    struct String moduleName=std_string_OwnedStringView(&decl->moduleName);
    return (packageName.len==0&&moduleName.len==0);
}
Status kek_compiler_SelfCWriteDeclCName(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl) {
    if (decl->hasReceiver) {
        Status status=kek_compiler_SelfCWriteTypeSuffixFromRange(program,out,decl->fileIndex,decl->receiverStart,decl->receiverEnd);
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
                status=kek_compiler_SelfCWriteOperatorName(out,decl->operatorCode);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"_");
            }
            if (status==Status_Ok) {
                status=std_format_FormatU64ToBuilder(out,((u64)(decl->paramCount)),10);
            }
            return (status);
        }
        return (kek_compiler_SelfCWriteToken(program,out,decl->fileIndex,decl->nameIndex));
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
    Status nameStatus=kek_compiler_SelfCWriteToken(program,out,decl->fileIndex,decl->nameIndex);
    if (nameStatus!=Status_Ok) {
        return (nameStatus);
    }
    if (decl->isGeneric&&decl->genericStart<kek_compiler_SelfCFileTokenCount(program,decl->fileIndex)) {
        return (Status_Ok);
    }
    return (Status_Ok);
}
Status kek_compiler_SelfCWriteTypeSuffixFromRange(struct SelfCProgram* program,struct StringBuilder* out,usize fileIndex,usize start,usize end) {
    if (start>=end) {
        return (Status_Ok);
    }
    if (start+1<end&&kek_compiler_SelfCIsOperator(program,fileIndex,start+1,OperatorKind_Less)) {
        Status status=kek_compiler_SelfCWriteSanitized(out,kek_compiler_SelfCTokenText(program,fileIndex,start));
        if (status!=Status_Ok) {
            return (status);
        }
        usize genericEnd=kek_compiler_SelfCFindMatching(program,fileIndex,start+1);
        usize argStart=start+2;
        while (argStart<genericEnd) {
            usize argEnd=kek_compiler_SelfCFindTokenAtDepthZero(program,fileIndex,argStart,genericEnd,PunctuationKind_Comma);
            status=StringBuilder_WriteCString(out,"__");
            if (status!=Status_Ok) {
                return (status);
            }
            status=kek_compiler_SelfCWriteTypeSuffixFromRange(program,out,fileIndex,argStart,argEnd);
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
    return (kek_compiler_SelfCWriteSanitized(out,kek_compiler_SelfCTokenText(program,fileIndex,start)));
}
Status kek_compiler_SelfCTypeInfoDestroy(struct SelfCTypeInfo* info) {
    std_string_DestroyOwnedString(&info->key);
    std_string_DestroyOwnedString(&info->cType);
    std_string_DestroyOwnedString(&info->baseName);
    std_string_DestroyOwnedString(&info->arg0);
    std_string_DestroyOwnedString(&info->arg1);
    std_string_DestroyOwnedString(&info->arg2);
    return (Status_Ok);
}
Status kek_compiler_SelfCTypeInfoInitEmpty(struct SelfCProgram* program,struct SelfCTypeInfo* info) {
    Status status=kek_compiler_SelfCMakeOwnedEmpty(program,&info->key);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeOwnedEmpty(program,&info->cType);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeOwnedEmpty(program,&info->baseName);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeOwnedEmpty(program,&info->arg0);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeOwnedEmpty(program,&info->arg1);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeOwnedEmpty(program,&info->arg2);
    }
    info->argCount=0;
    info->isPointer=0;
    return (status);
}
Status kek_compiler_SelfCReplaceOwned(struct OwnedString* target,struct OwnedString value) {
    std_string_DestroyOwnedString(target);
    target->data=value.data;
    target->len=value.len;
    target->cap=value.cap;
    target->allocator=value.allocator;
    return (Status_Ok);
}
Status kek_compiler_SelfCBuildTypeKey(struct SelfCProgram* program,usize fileIndex,usize start,usize end,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
    Status status=kek_compiler_SelfCWriteTypeSuffixFromRange(program,&builder,fileIndex,start,end);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCDetachBuilder(&builder,out);
    }
    StringBuilder_Destroy(&builder);
    return (status);
}
Status kek_compiler_SelfCWriteCTypeFromKey(struct SelfCProgram* program,struct StringBuilder* out,struct String key) {
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
    if (kek_compiler_SelfCIsBuiltinTypeName(key)) {
        return (StringBuilder_WriteString(out,key));
    }
    struct Result__usize ptrMarker=String_FindByte(&key,'*');
    if (ptrMarker.status==Status_Ok) {
        return (StringBuilder_WriteString(out,key));
    }
    struct SelfCDecl* decl=kek_compiler_SelfCFindTypeDecl(program,key);
    if (decl!=0) {
        if (decl->kind==SelfCDeclKind_Struct) {
            Status status=StringBuilder_WriteCString(out,"struct ");
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,key);
            }
            return (status);
        }
        return (StringBuilder_WriteString(out,key));
    }
    for (usize i=0;i<program->typeUseCount;i++) {
        struct String useKey=std_string_OwnedStringView(&program->typeUses[i].key);
        if (String_Equals(&useKey,key)) {
            struct SelfCDecl* baseDecl=kek_compiler_SelfCFindTypeDecl(program,std_string_OwnedStringView(&program->typeUses[i].baseName));
            if (baseDecl!=0&&baseDecl->kind==SelfCDeclKind_Struct) {
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
Status kek_compiler_SelfCMakeCTypeFromKey(struct SelfCProgram* program,struct String key,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
    Status status=kek_compiler_SelfCWriteCTypeFromKey(program,&builder,key);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCDetachBuilder(&builder,out);
    }
    StringBuilder_Destroy(&builder);
    return (status);
}
Status kek_compiler_SelfCTypeInfoFromKey(struct SelfCProgram* program,struct String key,struct SelfCTypeInfo* info) {
    Status status=kek_compiler_SelfCTypeInfoInitEmpty(program,info);
    if (status!=Status_Ok) {
        return (status);
    }
    struct OwnedString keyOwned={0};
    status=kek_compiler_SelfCCloneString(program,key,&keyOwned);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCReplaceOwned(&info->key,keyOwned);
    }
    struct OwnedString cOwned={0};
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeCTypeFromKey(program,key,&cOwned);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCReplaceOwned(&info->cType,cOwned);
    }
    struct Result__usize ptrMarker=String_FindByte(&key,'*');
    info->isPointer=ptrMarker.status==Status_Ok;
    return (status);
}
Status kek_compiler_SelfCRenderTypeInfo(struct SelfCProgram* program,usize fileIndex,usize start,usize end,struct SelfCTypeInfo* info) {
    Status status=kek_compiler_SelfCTypeInfoInitEmpty(program,info);
    if (status!=Status_Ok) {
        return (status);
    }
    if (start>=end) {
        return (Status_Ok);
    }
    struct String first=kek_compiler_SelfCTokenText(program,fileIndex,start);
    if (String_EqualsCString(&first,"ptr")&&start+1<end&&kek_compiler_SelfCIsOperator(program,fileIndex,start+1,OperatorKind_Less)) {
        usize genericEnd=kek_compiler_SelfCFindMatching(program,fileIndex,start+1);
        struct SelfCTypeInfo inner={0};
        status=kek_compiler_SelfCRenderTypeInfo(program,fileIndex,start+2,genericEnd,&inner);
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
            status=kek_compiler_SelfCDetachBuilder(&key,&keyOwned);
        }
        StringBuilder_Destroy(&key);
        if (status==Status_Ok) {
            status=kek_compiler_SelfCReplaceOwned(&info->key,keyOwned);
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
            status=kek_compiler_SelfCDetachBuilder(&cType,&cOwned);
        }
        StringBuilder_Destroy(&cType);
        if (status==Status_Ok) {
            status=kek_compiler_SelfCReplaceOwned(&info->cType,cOwned);
        }
        info->isPointer=1;
        kek_compiler_SelfCTypeInfoDestroy(&inner);
        return (status);
    }
    status=kek_compiler_SelfCBuildTypeKey(program,fileIndex,start,end,&info->key);
    if (status!=Status_Ok) {
        return (status);
    }
    if (start+1<end&&kek_compiler_SelfCIsOperator(program,fileIndex,start+1,OperatorKind_Less)) {
        status=kek_compiler_SelfCCloneString(program,first,&info->baseName);
        if (status!=Status_Ok) {
            return (status);
        }
        usize genericEnd=kek_compiler_SelfCFindMatching(program,fileIndex,start+1);
        usize argStart=start+2;
        while (argStart<genericEnd) {
            usize argEnd=kek_compiler_SelfCFindTokenAtDepthZero(program,fileIndex,argStart,genericEnd,PunctuationKind_Comma);
            struct SelfCTypeInfo argInfo={0};
            status=kek_compiler_SelfCRenderTypeInfo(program,fileIndex,argStart,argEnd,&argInfo);
            if (status!=Status_Ok) {
                return (status);
            }
            if (info->argCount==0) {
                struct OwnedString cloneArg={0};
                status=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&argInfo.key),&cloneArg);
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCReplaceOwned(&info->arg0,cloneArg);
                }
            } else {
                if (info->argCount==1) {
                    struct OwnedString cloneArg1={0};
                    status=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&argInfo.key),&cloneArg1);
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCReplaceOwned(&info->arg1,cloneArg1);
                    }
                } else {
                    if (info->argCount==2) {
                        struct OwnedString cloneArg2={0};
                        status=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&argInfo.key),&cloneArg2);
                        if (status==Status_Ok) {
                            status=kek_compiler_SelfCReplaceOwned(&info->arg2,cloneArg2);
                        }
                    }
                }
            }
            info->argCount+=1;
            kek_compiler_SelfCTypeInfoDestroy(&argInfo);
            if (status!=Status_Ok) {
                return (status);
            }
            if (argEnd>=genericEnd) {
                break;
            }
            argStart=argEnd+1;
        }
    }
    status=kek_compiler_SelfCMakeCTypeFromKey(program,std_string_OwnedStringView(&info->key),&info->cType);
    return (status);
}
bool kek_compiler_SelfCTypeUseExists(struct SelfCProgram* program,struct String key) {
    for (usize i=0;i<program->typeUseCount;i++) {
        struct String useKey=std_string_OwnedStringView(&program->typeUses[i].key);
        if (String_Equals(&useKey,key)) {
            return (1);
        }
    }
    return (0);
}
Status kek_compiler_SelfCAddTypeUse(struct SelfCProgram* program,struct SelfCTypeInfo* info) {
    if (info->argCount==0) {
        return (Status_Ok);
    }
    struct String key=std_string_OwnedStringView(&info->key);
    if (kek_compiler_SelfCTypeUseExists(program,key)) {
        return (Status_Ok);
    }
    if (program->typeUseCount>=(sizeof(program->typeUses)/sizeof((program->typeUses)[0]))) {
        return (kek_compiler_SelfCAddDiagnostic(program,"too many generic type uses"));
    }
    struct SelfCTypeUse* use=&program->typeUses[program->typeUseCount];
    Status status=kek_compiler_SelfCCloneString(program,key,&use->key);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&info->cType),&use->cName);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&info->baseName),&use->baseName);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&info->arg0),&use->arg0);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&info->arg1),&use->arg1);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&info->arg2),&use->arg2);
    }
    use->argCount=info->argCount;
    use->emitted=0;
    if (status==Status_Ok) {
        program->typeUseCount+=1;
    }
    return (status);
}
Status kek_compiler_SelfCAddTypeUseFromBaseArg(struct SelfCProgram* program,str baseName,struct String arg0) {
    struct StringBuilder keyBuilder=std_string_StringBuilderNew(program->allocator);
    Status status=StringBuilder_WriteCString(&keyBuilder,baseName);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(&keyBuilder,"__");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(&keyBuilder,arg0);
    }
    struct String key=StringBuilder_View(&keyBuilder);
    if (status==Status_Ok&&kek_compiler_SelfCTypeUseExists(program,key)) {
        StringBuilder_Destroy(&keyBuilder);
        return (Status_Ok);
    }
    if (status==Status_Ok&&program->typeUseCount>=(sizeof(program->typeUses)/sizeof((program->typeUses)[0]))) {
        StringBuilder_Destroy(&keyBuilder);
        return (kek_compiler_SelfCAddDiagnostic(program,"too many generic type uses"));
    }
    struct SelfCTypeUse* use=&program->typeUses[program->typeUseCount];
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,key,&use->key);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,key,&use->cName);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,std_string_StringFromCString(baseName),&use->baseName);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,arg0,&use->arg0);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeOwnedEmpty(program,&use->arg1);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeOwnedEmpty(program,&use->arg2);
    }
    use->argCount=1;
    use->emitted=0;
    if (status==Status_Ok) {
        program->typeUseCount+=1;
    }
    StringBuilder_Destroy(&keyBuilder);
    return (status);
}
bool kek_compiler_SelfCConcreteTypeKey(struct SelfCProgram* program,struct String key) {
    if (key.len==0) {
        return (0);
    }
    if (kek_compiler_SelfCIsBuiltinTypeName(key)) {
        return (1);
    }
    if (String_ContainsByte(&key,'*')) {
        return (1);
    }
    if (kek_compiler_SelfCFindTypeDecl(program,key)!=0) {
        return (1);
    }
    for (usize i=0;i<program->typeUseCount;i++) {
        struct String useKey=std_string_OwnedStringView(&program->typeUses[i].key);
        if (String_Equals(&useKey,key)) {
            return (1);
        }
    }
    return (0);
}
bool kek_compiler_SelfCTypeInfoConcrete(struct SelfCProgram* program,struct SelfCTypeInfo* info) {
    if (info->argCount==0) {
        return (kek_compiler_SelfCConcreteTypeKey(program,std_string_OwnedStringView(&info->key)));
    }
    if (info->argCount>=1&&!kek_compiler_SelfCConcreteTypeKey(program,std_string_OwnedStringView(&info->arg0))) {
        return (0);
    }
    if (info->argCount>=2&&!kek_compiler_SelfCConcreteTypeKey(program,std_string_OwnedStringView(&info->arg1))) {
        return (0);
    }
    if (info->argCount>=3&&!kek_compiler_SelfCConcreteTypeKey(program,std_string_OwnedStringView(&info->arg2))) {
        return (0);
    }
    return (1);
}
Status kek_compiler_SelfCCollectTypeUsesInRange(struct SelfCProgram* program,usize fileIndex,usize start,usize end) {
    usize i=start;
    while (i<end) {
        if (i+1<end&&kek_compiler_SelfCIsOperator(program,fileIndex,i+1,OperatorKind_Less)) {
            struct String name=kek_compiler_SelfCTokenText(program,fileIndex,i);
            struct SelfCDecl* decl=kek_compiler_SelfCFindTypeDecl(program,name);
            if (decl!=0&&decl->isGeneric) {
                usize genericEnd=kek_compiler_SelfCFindMatching(program,fileIndex,i+1);
                struct SelfCTypeInfo info={0};
                Status status=kek_compiler_SelfCRenderTypeInfo(program,fileIndex,i,genericEnd+1,&info);
                if (status!=Status_Ok) {
                    return (status);
                }
                if (kek_compiler_SelfCTypeInfoConcrete(program,&info)) {
                    status=kek_compiler_SelfCAddTypeUse(program,&info);
                }
                kek_compiler_SelfCTypeInfoDestroy(&info);
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
bool kek_compiler_SelfCDeclScopeMatches(struct SelfCDecl* decl,struct String scopeName) {
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
struct SelfCDecl* kek_compiler_SelfCFindFunctionDeclByName(struct SelfCProgram* program,struct String name,struct String scopeName) {
    for (usize i=0;i<program->declCount;i++) {
        struct SelfCDecl* decl=&program->decls[i];
        if (decl->kind!=SelfCDeclKind_Function) {
            continue;
        }
        if (decl->hasReceiver) {
            continue;
        }
        if (!kek_compiler_SelfCDeclNameMatches(program,decl,name)) {
            continue;
        }
        if (kek_compiler_SelfCDeclScopeMatches(decl,scopeName)) {
            return (decl);
        }
    }
    return (0);
}
bool kek_compiler_SelfCFuncUseExists(struct SelfCProgram* program,usize declIndex,struct String key) {
    for (usize i=0;i<program->funcUseCount;i++) {
        struct String useKey=std_string_OwnedStringView(&program->funcUses[i].key);
        if (program->funcUses[i].declIndex==declIndex&&String_Equals(&useKey,key)) {
            return (1);
        }
    }
    return (0);
}
usize kek_compiler_SelfCDeclIndex(struct SelfCProgram* program,struct SelfCDecl* decl) {
    for (usize i=0;i<program->declCount;i++) {
        if (&program->decls[i]==decl) {
            return (i);
        }
    }
    return ((sizeof(program->decls)/sizeof((program->decls)[0])));
}
Status kek_compiler_SelfCBuildGenericFuncCName(struct SelfCProgram* program,struct SelfCDecl* decl,struct SelfCTypeInfo* arg0,struct SelfCTypeInfo* arg1,struct SelfCTypeInfo* arg2,usize argCount,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
    Status status=Status_Ok;
    if (decl->hasReceiver&&argCount>=1) {
        status=kek_compiler_SelfCWriteSanitized(&builder,kek_compiler_SelfCTokenText(program,decl->fileIndex,decl->receiverStart));
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(&builder,"__");
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&arg0->key));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(&builder,'_');
        }
        if (decl->isOperator) {
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(&builder,"operator_");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteOperatorName(&builder,decl->operatorCode);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(&builder,"_");
            }
            if (status==Status_Ok) {
                status=std_format_FormatU64ToBuilder(&builder,((u64)(decl->paramCount)),10);
            }
        } else {
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteToken(program,&builder,decl->fileIndex,decl->nameIndex);
            }
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCDetachBuilder(&builder,out);
        }
        StringBuilder_Destroy(&builder);
        return (status);
    }
    status=kek_compiler_SelfCWriteDeclCName(program,&builder,decl);
    if (status==Status_Ok&&argCount>=1) {
        status=StringBuilder_WriteCString(&builder,"__");
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&arg0->key));
        }
    }
    if (status==Status_Ok&&argCount>=2) {
        status=StringBuilder_WriteCString(&builder,"__");
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&arg1->key));
        }
    }
    if (status==Status_Ok&&argCount>=3) {
        status=StringBuilder_WriteCString(&builder,"__");
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&arg2->key));
        }
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCDetachBuilder(&builder,out);
    }
    StringBuilder_Destroy(&builder);
    return (status);
}
Status kek_compiler_SelfCAddFuncUse(struct SelfCProgram* program,struct SelfCDecl* decl,struct SelfCTypeInfo* arg0,struct SelfCTypeInfo* arg1,struct SelfCTypeInfo* arg2,usize argCount) {
    if (argCount==0) {
        return (Status_Ok);
    }
    if (argCount>=1&&!kek_compiler_SelfCTypeInfoConcrete(program,arg0)) {
        return (Status_Ok);
    }
    if (argCount>=2&&!kek_compiler_SelfCTypeInfoConcrete(program,arg1)) {
        return (Status_Ok);
    }
    if (argCount>=3&&!kek_compiler_SelfCTypeInfoConcrete(program,arg2)) {
        return (Status_Ok);
    }
    struct OwnedString cName={0};
    Status status=kek_compiler_SelfCBuildGenericFuncCName(program,decl,arg0,arg1,arg2,argCount,&cName);
    if (status!=Status_Ok) {
        return (status);
    }
    usize declIndex=kek_compiler_SelfCDeclIndex(program,decl);
    struct String key=std_string_OwnedStringView(&cName);
    if (kek_compiler_SelfCFuncUseExists(program,declIndex,key)) {
        std_string_DestroyOwnedString(&cName);
        return (Status_Ok);
    }
    if (program->funcUseCount>=(sizeof(program->funcUses)/sizeof((program->funcUses)[0]))) {
        std_string_DestroyOwnedString(&cName);
        return (kek_compiler_SelfCAddDiagnostic(program,"too many generic function uses"));
    }
    struct SelfCFuncUse* use=&program->funcUses[program->funcUseCount];
    use->declIndex=declIndex;
    status=kek_compiler_SelfCCloneString(program,key,&use->key);
    if (status==Status_Ok) {
        use->cName=cName;
    } else {
        std_string_DestroyOwnedString(&cName);
    }
    if (status==Status_Ok&&argCount>=1) {
        status=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&arg0->key),&use->arg0);
    } else {
        if (status==Status_Ok) {
            status=kek_compiler_SelfCMakeOwnedEmpty(program,&use->arg0);
        }
    }
    if (status==Status_Ok&&argCount>=2) {
        status=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&arg1->key),&use->arg1);
    } else {
        if (status==Status_Ok) {
            status=kek_compiler_SelfCMakeOwnedEmpty(program,&use->arg1);
        }
    }
    if (status==Status_Ok&&argCount>=3) {
        status=kek_compiler_SelfCCloneString(program,std_string_OwnedStringView(&arg2->key),&use->arg2);
    } else {
        if (status==Status_Ok) {
            status=kek_compiler_SelfCMakeOwnedEmpty(program,&use->arg2);
        }
    }
    use->argCount=argCount;
    use->emitted=0;
    if (status==Status_Ok) {
        program->funcUseCount+=1;
    }
    return (status);
}
Status kek_compiler_SelfCCollectGenericFunctionUsesInRange(struct SelfCProgram* program,usize fileIndex,usize start,usize end) {
    usize i=start;
    while (i<end) {
        if (i+2<end&&kek_compiler_SelfCIsOperator(program,fileIndex,i+1,OperatorKind_Less)) {
            usize genericEnd=kek_compiler_SelfCFindMatching(program,fileIndex,i+1);
            if (genericEnd+1<end&&kek_compiler_SelfCIsPunctuation(program,fileIndex,genericEnd+1,PunctuationKind_LeftParen)) {
                struct String name=kek_compiler_SelfCTokenText(program,fileIndex,i);
                struct String scopeName=std_string_StringFromCString("");
                if (i>=2&&kek_compiler_SelfCIsOperator(program,fileIndex,i-1,OperatorKind_Scope)) {
                    scopeName=kek_compiler_SelfCTokenText(program,fileIndex,i-2);
                }
                struct SelfCDecl* decl=kek_compiler_SelfCFindFunctionDeclByName(program,name,scopeName);
                if (decl!=0&&decl->isGeneric) {
                    struct SelfCTypeInfo arg0={0};
                    struct SelfCTypeInfo arg1={0};
                    struct SelfCTypeInfo arg2={0};
                    kek_compiler_SelfCTypeInfoInitEmpty(program,&arg0);
                    kek_compiler_SelfCTypeInfoInitEmpty(program,&arg1);
                    kek_compiler_SelfCTypeInfoInitEmpty(program,&arg2);
                    usize argCount=0;
                    usize argStart=i+2;
                    Status status=Status_Ok;
                    while (argStart<genericEnd) {
                        usize argEnd=kek_compiler_SelfCFindTokenAtDepthZero(program,fileIndex,argStart,genericEnd,PunctuationKind_Comma);
                        if (argCount==0) {
                            status=kek_compiler_SelfCRenderTypeInfo(program,fileIndex,argStart,argEnd,&arg0);
                        } else {
                            if (argCount==1) {
                                status=kek_compiler_SelfCRenderTypeInfo(program,fileIndex,argStart,argEnd,&arg1);
                            } else {
                                if (argCount==2) {
                                    status=kek_compiler_SelfCRenderTypeInfo(program,fileIndex,argStart,argEnd,&arg2);
                                }
                            }
                        }
                        argCount+=1;
                        if (status!=Status_Ok||argEnd>=genericEnd) {
                            break;
                        }
                        argStart=argEnd+1;
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCAddFuncUse(program,decl,&arg0,&arg1,&arg2,argCount);
                    }
                    kek_compiler_SelfCTypeInfoDestroy(&arg0);
                    kek_compiler_SelfCTypeInfoDestroy(&arg1);
                    kek_compiler_SelfCTypeInfoDestroy(&arg2);
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
struct SelfCDecl* kek_compiler_SelfCFindGenericMethodDecl(struct SelfCProgram* program,struct String receiverBase,str methodName) {
    for (usize i=0;i<program->declCount;i++) {
        struct SelfCDecl* decl=&program->decls[i];
        if (decl->kind!=SelfCDeclKind_Function||!decl->hasReceiver||!decl->isGeneric||decl->isOperator) {
            continue;
        }
        struct String base=kek_compiler_SelfCTokenText(program,decl->fileIndex,decl->receiverStart);
        if (!String_Equals(&base,receiverBase)) {
            continue;
        }
        struct String name=kek_compiler_SelfCTokenText(program,decl->fileIndex,decl->nameIndex);
        if (String_EqualsCString(&name,methodName)) {
            return (decl);
        }
    }
    return (0);
}
Status kek_compiler_SelfCAddGenericMethodUseByName(struct SelfCProgram* program,struct SelfCTypeUse* typeUse,str methodName) {
    struct String baseName=std_string_OwnedStringView(&typeUse->baseName);
    struct SelfCDecl* decl=kek_compiler_SelfCFindGenericMethodDecl(program,baseName,methodName);
    if (decl==0) {
        return (Status_Ok);
    }
    struct SelfCTypeInfo arg0={0};
    struct SelfCTypeInfo arg1={0};
    struct SelfCTypeInfo arg2={0};
    bool arg0Ready=0;
    bool arg1Ready=0;
    bool arg2Ready=0;
    Status status=kek_compiler_SelfCTypeInfoInitEmpty(program,&arg1);
    if (status==Status_Ok) {
        arg1Ready=1;
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCTypeInfoInitEmpty(program,&arg2);
    }
    if (status==Status_Ok) {
        arg2Ready=1;
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCTypeInfoFromKey(program,std_string_OwnedStringView(&typeUse->arg0),&arg0);
    }
    if (status==Status_Ok) {
        arg0Ready=1;
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCAddFuncUse(program,decl,&arg0,&arg1,&arg2,1);
    }
    if (arg0Ready) {
        kek_compiler_SelfCTypeInfoDestroy(&arg0);
    }
    if (arg1Ready) {
        kek_compiler_SelfCTypeInfoDestroy(&arg1);
    }
    if (arg2Ready) {
        kek_compiler_SelfCTypeInfoDestroy(&arg2);
    }
    return (status);
}
Status kek_compiler_SelfCCollectGenericCollectionMethods(struct SelfCProgram* program) {
    usize typeUseLimit=program->typeUseCount;
    for (usize i=0;i<typeUseLimit;i++) {
        struct SelfCTypeUse* use=&program->typeUses[i];
        struct String base=std_string_OwnedStringView(&use->baseName);
        if (use->argCount<1) {
            continue;
        }
        Status status=Status_Ok;
        if (String_EqualsCString(&base,"Array")) {
            status=kek_compiler_SelfCAddTypeUseFromBaseArg(program,"Result",std_string_OwnedStringView(&use->arg0));
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddTypeUseFromBaseArg(program,"Slice",std_string_OwnedStringView(&use->arg0));
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddTypeUseFromBaseArg(program,"Span",std_string_OwnedStringView(&use->arg0));
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"Destroy");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"Clear");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"Reserve");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"AppendSlice");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"Push");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"PushZeroed");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"Pop");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"Get");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"Set");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"GetPtr");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"LastPtr");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"Span");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"Slice");
            }
        } else {
            if (String_EqualsCString(&base,"LinkedList")) {
                status=kek_compiler_SelfCAddTypeUseFromBaseArg(program,"ListNode",std_string_OwnedStringView(&use->arg0));
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCAddTypeUseFromBaseArg(program,"Result",std_string_OwnedStringView(&use->arg0));
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"PushBack");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"PushFront");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"PopBack");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"Destroy");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCAddGenericMethodUseByName(program,use,"PopFront");
                }
            }
        }
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
Status kek_compiler_SelfCCollectGenericFunctionUses(struct SelfCProgram* program) {
    for (usize i=0;i<program->declCount;i++) {
        struct SelfCDecl* decl=&program->decls[i];
        Status status=kek_compiler_SelfCCollectGenericFunctionUsesInRange(program,decl->fileIndex,decl->start,decl->end);
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (kek_compiler_SelfCCollectGenericCollectionMethods(program));
}
Status kek_compiler_SelfCCollectTypeUses(struct SelfCProgram* program) {
    for (usize i=0;i<program->declCount;i++) {
        struct SelfCDecl* decl=&program->decls[i];
        Status status=kek_compiler_SelfCCollectTypeUsesInRange(program,decl->fileIndex,decl->start,decl->end);
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
Status kek_compiler_SelfCWritePrelude(struct StringBuilder* out) {
    Status status=StringBuilder_WriteCString(out,"#include <assert.h>");
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"#include <stdint.h>");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"#include <stddef.h>");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"#include <stdbool.h>");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef uint8_t u8;");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef uint16_t u16;");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef uint32_t u32;");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef uint64_t u64;");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef int8_t i8;");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef int16_t i16;");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef int32_t i32;");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef int64_t i64;");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef float f32;");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef double f64;");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef void* ptr;");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"typedef const char* str;");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,10);
    }
    return (status);
}
Status kek_compiler_SelfCWriteDeclarator(struct SelfCProgram* program,struct StringBuilder* out,usize fileIndex,usize typeStart,usize typeEnd,usize nameIndex,bool isArray,usize arrayStart,usize arrayEnd) {
    struct SelfCTypeInfo info={0};
    Status status=kek_compiler_SelfCRenderTypeInfo(program,fileIndex,typeStart,typeEnd,&info);
    if (status!=Status_Ok) {
        return (status);
    }
    status=StringBuilder_WriteString(out,std_string_OwnedStringView(&info.cType));
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,' ');
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteToken(program,out,fileIndex,nameIndex);
    }
    if (status==Status_Ok&&isArray) {
        status=StringBuilder_WriteByte(out,'[');
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteTokenRangeRaw(program,out,fileIndex,arrayStart,arrayEnd);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,']');
        }
    }
    kek_compiler_SelfCTypeInfoDestroy(&info);
    return (status);
}
Status kek_compiler_SelfCWriteFields(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl) {
    for (usize i=0;i<decl->fieldCount;i++) {
        struct SelfCField* field=&program->fields[decl->firstField+i];
        Status status=StringBuilder_WriteCString(out,"    ");
        if (status!=Status_Ok) {
            return (status);
        }
        if (field->isNestedStruct) {
            status=StringBuilder_WriteCString(out,"struct {\n");
            if (status!=Status_Ok) {
                return (status);
            }
            struct SelfCDecl nested={0};
            nested.kind=SelfCDeclKind_Struct;
            nested.fileIndex=field->fileIndex;
            nested.bodyStart=field->nestedBodyStart;
            nested.bodyEnd=field->nestedBodyEnd;
            nested.firstField=program->fieldCount;
            nested.fieldCount=0;
            nested.nameIndex=field->nameIndex;
            nested.isGeneric=0;
            Status parseNested=kek_compiler_SelfCParseFields(program,&nested,field->fileIndex,field->nestedBodyStart,field->nestedBodyEnd);
            if (parseNested!=Status_Ok) {
                return (parseNested);
            }
            status=kek_compiler_SelfCWriteFields(program,out,&nested);
            if (status!=Status_Ok) {
                return (status);
            }
            status=StringBuilder_WriteCString(out,"    } ");
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteToken(program,out,field->fileIndex,field->nameIndex);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,";\n");
            }
            return (status);
        }
        status=kek_compiler_SelfCWriteDeclarator(program,out,field->fileIndex,field->typeStart,field->typeEnd,field->nameIndex,field->isArray,field->arrayStart,field->arrayEnd);
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,";\n");
        }
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
bool kek_compiler_SelfCGenericParamEquals(struct SelfCProgram* program,struct SelfCDecl* decl,usize paramIndex,struct String name) {
    if (!decl->isGeneric) {
        return (0);
    }
    if (decl->genericStart<kek_compiler_SelfCFileTokenCount(program,decl->fileIndex)) {
        usize index=decl->genericStart+1;
        usize current=0;
        while (index<decl->genericEnd) {
            if (kek_compiler_SelfCIsPunctuation(program,decl->fileIndex,index,PunctuationKind_Comma)) {
                index+=1;
                continue;
            }
            if (current==paramIndex) {
                struct String paramName=kek_compiler_SelfCTokenText(program,decl->fileIndex,index);
                return (String_Equals(&paramName,name));
            }
            current+=1;
            while (index<decl->genericEnd&&!kek_compiler_SelfCIsPunctuation(program,decl->fileIndex,index,PunctuationKind_Comma)) {
                index+=1;
            }
        }
    }
    if (decl->hasReceiver&&decl->receiverStart+1<decl->receiverEnd&&kek_compiler_SelfCIsOperator(program,decl->fileIndex,decl->receiverStart+1,OperatorKind_Less)) {
        usize genericEnd=kek_compiler_SelfCFindMatching(program,decl->fileIndex,decl->receiverStart+1);
        usize index=decl->receiverStart+2;
        usize current=0;
        while (index<genericEnd) {
            if (kek_compiler_SelfCIsPunctuation(program,decl->fileIndex,index,PunctuationKind_Comma)) {
                index+=1;
                continue;
            }
            if (current==paramIndex) {
                struct String paramName=kek_compiler_SelfCTokenText(program,decl->fileIndex,index);
                return (String_Equals(&paramName,name));
            }
            current+=1;
            while (index<genericEnd&&!kek_compiler_SelfCIsPunctuation(program,decl->fileIndex,index,PunctuationKind_Comma)) {
                index+=1;
            }
        }
    }
    return (0);
}
Status kek_compiler_SelfCWriteTypeSuffixSubst(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCTypeUse* use,usize fileIndex,usize start,usize end) {
    if (start>=end) {
        return (Status_Ok);
    }
    struct String first=kek_compiler_SelfCTokenText(program,fileIndex,start);
    if (end==start+1) {
        if (kek_compiler_SelfCGenericParamEquals(program,decl,0,first)) {
            return (StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg0)));
        }
        if (kek_compiler_SelfCGenericParamEquals(program,decl,1,first)) {
            return (StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg1)));
        }
        if (kek_compiler_SelfCGenericParamEquals(program,decl,2,first)) {
            return (StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg2)));
        }
        return (kek_compiler_SelfCWriteSanitized(out,first));
    }
    if (start+1<end&&kek_compiler_SelfCIsOperator(program,fileIndex,start+1,OperatorKind_Less)) {
        Status status=kek_compiler_SelfCWriteSanitized(out,first);
        usize genericEnd=kek_compiler_SelfCFindMatching(program,fileIndex,start+1);
        usize argStart=start+2;
        while (argStart<genericEnd) {
            usize argEnd=kek_compiler_SelfCFindTokenAtDepthZero(program,fileIndex,argStart,genericEnd,PunctuationKind_Comma);
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"__");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteTypeSuffixSubst(program,out,decl,use,fileIndex,argStart,argEnd);
            }
            if (status!=Status_Ok||argEnd>=genericEnd) {
                break;
            }
            argStart=argEnd+1;
        }
        return (status);
    }
    return (kek_compiler_SelfCWriteTypeSuffixFromRange(program,out,fileIndex,start,end));
}
Status kek_compiler_SelfCMakeSubstTypeKey(struct SelfCProgram* program,struct SelfCDecl* decl,struct SelfCTypeUse* use,usize fileIndex,usize start,usize end,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
    Status status=kek_compiler_SelfCWriteTypeSuffixSubst(program,&builder,decl,use,fileIndex,start,end);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCDetachBuilder(&builder,out);
    }
    StringBuilder_Destroy(&builder);
    return (status);
}
Status kek_compiler_SelfCWriteDeclaratorSubst(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCTypeUse* use,usize fileIndex,usize typeStart,usize typeEnd,usize nameIndex,bool isArray,usize arrayStart,usize arrayEnd) {
    struct String first=kek_compiler_SelfCTokenText(program,fileIndex,typeStart);
    struct OwnedString key={0};
    Status status=Status_Ok;
    if (String_EqualsCString(&first,"ptr")&&typeStart+1<typeEnd&&kek_compiler_SelfCIsOperator(program,fileIndex,typeStart+1,OperatorKind_Less)) {
        usize genericEnd=kek_compiler_SelfCFindMatching(program,fileIndex,typeStart+1);
        struct OwnedString innerKey={0};
        status=kek_compiler_SelfCMakeSubstTypeKey(program,decl,use,fileIndex,typeStart+2,genericEnd,&innerKey);
        struct StringBuilder ct=std_string_StringBuilderNew(program->allocator);
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteCTypeFromKey(program,&ct,std_string_OwnedStringView(&innerKey));
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
        status=kek_compiler_SelfCMakeSubstTypeKey(program,decl,use,fileIndex,typeStart,typeEnd,&key);
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteCTypeFromKey(program,out,std_string_OwnedStringView(&key));
        }
        if (status==Status_Ok) {
            std_string_DestroyOwnedString(&key);
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,' ');
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteToken(program,out,fileIndex,nameIndex);
    }
    if (status==Status_Ok&&isArray) {
        status=StringBuilder_WriteByte(out,'[');
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteTokenRangeRaw(program,out,fileIndex,arrayStart,arrayEnd);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,']');
        }
    }
    return (status);
}
Status kek_compiler_SelfCWriteFieldsSubst(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCTypeUse* use) {
    for (usize i=0;i<decl->fieldCount;i++) {
        struct SelfCField* field=&program->fields[decl->firstField+i];
        Status status=StringBuilder_WriteCString(out,"    ");
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteDeclaratorSubst(program,out,decl,use,field->fileIndex,field->typeStart,field->typeEnd,field->nameIndex,field->isArray,field->arrayStart,field->arrayEnd);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,";\n");
        }
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
Status kek_compiler_SelfCWriteStructDecl(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct String specializedName) {
    Status status=StringBuilder_WriteCString(out,"struct ");
    bool hasPacked=0;
    bool hasAligned=0;
    usize alignedValue=0;
    if (decl->start>0) {
        usize scan=decl->start;
        while (scan>0) {
            scan-=1;
            if (kek_compiler_SelfCIsPunctuation(program,decl->fileIndex,scan,PunctuationKind_LeftBracket)) {
                for (usize i=scan;i<decl->start;i++) {
                    if (kek_compiler_SelfCIsIdentifierText(program,decl->fileIndex,i,"packed")) {
                        hasPacked=1;
                    }
                    if (kek_compiler_SelfCIsIdentifierText(program,decl->fileIndex,i,"aligned")) {
                        hasAligned=1;
                        if (i+2<decl->start&&kek_compiler_SelfCIsPunctuation(program,decl->fileIndex,i+1,PunctuationKind_LeftParen)) {
                            alignedValue=i+2;
                        }
                    }
                }
                break;
            }
            if (kek_compiler_SelfCIsPunctuation(program,decl->fileIndex,scan,PunctuationKind_Semicolon)||kek_compiler_SelfCIsPunctuation(program,decl->fileIndex,scan,PunctuationKind_RightBrace)) {
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
                status=kek_compiler_SelfCWriteToken(program,out,decl->fileIndex,alignedValue);
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
        status=kek_compiler_SelfCWriteToken(program,out,decl->fileIndex,decl->nameIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," {\n");
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteFields(program,out,decl);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"};\n");
    }
    return (status);
}
Status kek_compiler_SelfCWriteUnionDecl(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl) {
    Status status=StringBuilder_WriteCString(out,"typedef union ");
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteToken(program,out,decl->fileIndex,decl->nameIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," {\n");
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteFields(program,out,decl);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"} ");
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteToken(program,out,decl->fileIndex,decl->nameIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    return (status);
}
Status kek_compiler_SelfCWriteEnumDecl(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl) {
    Status status=StringBuilder_WriteCString(out,"typedef enum ");
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteToken(program,out,decl->fileIndex,decl->nameIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," {\n");
    }
    usize i=decl->bodyStart;
    while (i<decl->bodyEnd) {
        if (kek_compiler_SelfCIsPunctuation(program,decl->fileIndex,i,PunctuationKind_Comma)) {
            i+=1;
            continue;
        }
        if (!kek_compiler_SelfCIsTokenKind(program,decl->fileIndex,i,TokenKind_Identifier)) {
            i+=1;
            continue;
        }
        status=StringBuilder_WriteCString(out,"    ");
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteToken(program,out,decl->fileIndex,decl->nameIndex);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,'_');
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteToken(program,out,decl->fileIndex,i);
        }
        i+=1;
        if (i<decl->bodyEnd&&kek_compiler_SelfCIsOperator(program,decl->fileIndex,i,OperatorKind_Assign)) {
            status=StringBuilder_WriteByte(out,'=');
            i+=1;
            while (i<decl->bodyEnd&&!kek_compiler_SelfCIsPunctuation(program,decl->fileIndex,i,PunctuationKind_Comma)) {
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteToken(program,out,decl->fileIndex,i);
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
        status=kek_compiler_SelfCWriteToken(program,out,decl->fileIndex,decl->nameIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    return (status);
}
Status kek_compiler_SelfCWriteAliasDecl(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl) {
    struct SelfCTypeInfo info={0};
    Status status=kek_compiler_SelfCRenderTypeInfo(program,decl->fileIndex,decl->returnStart,decl->returnEnd,&info);
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
        status=kek_compiler_SelfCWriteToken(program,out,decl->fileIndex,decl->nameIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    kek_compiler_SelfCTypeInfoDestroy(&info);
    return (status);
}
Status kek_compiler_SelfCWriteExternDecl(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl) {
    struct SelfCTokenFile* file=&program->tokenFiles[decl->fileIndex];
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
Status kek_compiler_SelfCWriteTypeDeclarations(struct SelfCProgram* program,struct StringBuilder* out) {
    for (usize i=0;i<program->declCount;i++) {
        struct SelfCDecl* decl=&program->decls[i];
        Status status=Status_Ok;
        if (decl->kind==SelfCDeclKind_ExternC) {
            status=kek_compiler_SelfCWriteExternDecl(program,out,decl);
        } else {
            if (decl->kind==SelfCDeclKind_Alias) {
                status=kek_compiler_SelfCWriteAliasDecl(program,out,decl);
            } else {
                if (decl->kind==SelfCDeclKind_Struct&&!decl->isGeneric) {
                    status=kek_compiler_SelfCWriteStructDecl(program,out,decl,std_string_StringFromCString(""));
                } else {
                    if (decl->kind==SelfCDeclKind_Union) {
                        status=kek_compiler_SelfCWriteUnionDecl(program,out,decl);
                    } else {
                        if (decl->kind==SelfCDeclKind_Enum) {
                            status=kek_compiler_SelfCWriteEnumDecl(program,out,decl);
                        }
                    }
                }
            }
        }
        if (status!=Status_Ok) {
            return (status);
        }
    }
    for (usize i=0;i<program->typeUseCount;i++) {
        struct SelfCTypeUse* use=&program->typeUses[i];
        struct SelfCDecl* decl=kek_compiler_SelfCFindTypeDecl(program,std_string_OwnedStringView(&use->baseName));
        if (decl!=0&&decl->kind==SelfCDeclKind_Struct) {
            Status status=StringBuilder_WriteCString(out,"struct ");
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->key));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out," {\n");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteFieldsSubst(program,out,decl,use);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"};\n");
            }
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    return (Status_Ok);
}
Status kek_compiler_SelfCEnvInit(struct SelfCProgram* program,struct SelfCEnv* env) {
    env->localCount=0;
    env->hasThis=0;
    env->deferCounter=0;
    env->eachCounter=0;
    Status status=kek_compiler_SelfCMakeOwnedEmpty(program,&env->returnTypeKey);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeOwnedEmpty(program,&env->returnCType);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeOwnedEmpty(program,&env->thisTypeKey);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeOwnedEmpty(program,&env->thisCType);
    }
    return (status);
}
Status kek_compiler_SelfCEnvDestroy(struct SelfCEnv* env) {
    for (usize i=0;i<env->localCount;i++) {
        std_string_DestroyOwnedString(&env->locals[i].name);
        std_string_DestroyOwnedString(&env->locals[i].typeKey);
        std_string_DestroyOwnedString(&env->locals[i].cType);
        std_string_DestroyOwnedString(&env->locals[i].arrayLen);
    }
    std_string_DestroyOwnedString(&env->returnTypeKey);
    std_string_DestroyOwnedString(&env->returnCType);
    std_string_DestroyOwnedString(&env->thisTypeKey);
    std_string_DestroyOwnedString(&env->thisCType);
    return (Status_Ok);
}
struct SelfCLocal* kek_compiler_SelfCEnvFind(struct SelfCEnv* env,struct String name) {
    for (usize i=env->localCount;i>0;i--) {
        struct SelfCLocal* local=&env->locals[i-1];
        struct String localName=std_string_OwnedStringView(&local->name);
        if (String_Equals(&localName,name)) {
            return (local);
        }
    }
    return (0);
}
Status kek_compiler_SelfCEnvAdd(struct SelfCProgram* program,struct SelfCEnv* env,struct String name,struct String typeKey,struct String cType,bool isArray,struct String arrayLen,bool isPointer) {
    if (env->localCount>=(sizeof(env->locals)/sizeof((env->locals)[0]))) {
        return (kek_compiler_SelfCAddDiagnostic(program,"too many locals"));
    }
    struct SelfCLocal* local=&env->locals[env->localCount];
    Status status=kek_compiler_SelfCCloneString(program,name,&local->name);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,typeKey,&local->typeKey);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,cType,&local->cType);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,arrayLen,&local->arrayLen);
    }
    local->isArray=isArray;
    local->isPointer=isPointer;
    if (status==Status_Ok) {
        env->localCount+=1;
    }
    return (status);
}
Status kek_compiler_SelfCExprInitEmpty(struct SelfCProgram* program,struct SelfCExpr* expr) {
    Status status=kek_compiler_SelfCMakeOwnedEmpty(program,&expr->text);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeOwnedEmpty(program,&expr->typeKey);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeOwnedEmpty(program,&expr->cType);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCMakeOwnedEmpty(program,&expr->arrayLen);
    }
    expr->isArray=0;
    expr->isLvalue=0;
    expr->isPointer=0;
    return (status);
}
Status kek_compiler_SelfCExprDestroy(struct SelfCExpr* expr) {
    std_string_DestroyOwnedString(&expr->text);
    std_string_DestroyOwnedString(&expr->typeKey);
    std_string_DestroyOwnedString(&expr->cType);
    std_string_DestroyOwnedString(&expr->arrayLen);
    return (Status_Ok);
}
Status kek_compiler_SelfCExprSetText(struct SelfCProgram* program,struct SelfCExpr* expr,struct String text) {
    struct OwnedString owned={0};
    Status status=kek_compiler_SelfCCloneString(program,text,&owned);
    if (status!=Status_Ok) {
        return (status);
    }
    return (kek_compiler_SelfCReplaceOwned(&expr->text,owned));
}
Status kek_compiler_SelfCExprSetCString(struct SelfCProgram* program,struct SelfCExpr* expr,str text) {
    struct OwnedString owned={0};
    Status status=kek_compiler_SelfCCloneCString(program,text,&owned);
    if (status!=Status_Ok) {
        return (status);
    }
    return (kek_compiler_SelfCReplaceOwned(&expr->text,owned));
}
Status kek_compiler_SelfCExprSetType(struct SelfCProgram* program,struct SelfCExpr* expr,struct String key,struct String cType,bool isPointer) {
    struct OwnedString keyOwned={0};
    Status status=kek_compiler_SelfCCloneString(program,key,&keyOwned);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCReplaceOwned(&expr->typeKey,keyOwned);
    }
    struct OwnedString cOwned={0};
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,cType,&cOwned);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCReplaceOwned(&expr->cType,cOwned);
    }
    expr->isPointer=isPointer;
    return (status);
}
Status kek_compiler_SelfCExprFromBuilder(struct SelfCExpr* expr,struct StringBuilder* builder) {
    struct OwnedString owned={0};
    Status status=kek_compiler_SelfCDetachBuilder(builder,&owned);
    if (status!=Status_Ok) {
        return (status);
    }
    return (kek_compiler_SelfCReplaceOwned(&expr->text,owned));
}
u8 kek_compiler_SelfCOperatorPrecedence(struct SelfCProgram* program,usize fileIndex,usize index) {
    if (kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_Assign)||kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_PlusAssign)||kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_MinusAssign)) {
        return (1);
    }
    if (kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_LogicalOr)) {
        return (2);
    }
    if (kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_LogicalAnd)) {
        return (3);
    }
    if (kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_Equal)||kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_NotEqual)||kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_Less)||kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_LessEqual)||kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_Greater)||kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_GreaterEqual)) {
        return (4);
    }
    if (kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_Plus)||kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_Minus)) {
        return (5);
    }
    if (kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_Multiply)||kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_Divide)||kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_Modulo)) {
        return (6);
    }
    return (0);
}
bool kek_compiler_SelfCOperatorRightAssociative(struct SelfCProgram* program,usize fileIndex,usize index) {
    return (kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_Assign)||kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_PlusAssign)||kek_compiler_SelfCIsOperator(program,fileIndex,index,OperatorKind_MinusAssign));
}
Status kek_compiler_SelfCWriteNumberToken(struct SelfCProgram* program,struct StringBuilder* out,usize fileIndex,usize index) {
    struct String text=kek_compiler_SelfCTokenText(program,fileIndex,index);
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
struct SelfCDecl* kek_compiler_SelfCFindFieldDecl(struct SelfCProgram* program,struct String typeKey) {
    for (usize i=0;i<program->declCount;i++) {
        struct SelfCDecl* decl=&program->decls[i];
        if ((decl->kind==SelfCDeclKind_Struct||decl->kind==SelfCDeclKind_Union)&&kek_compiler_SelfCDeclNameMatches(program,decl,typeKey)) {
            return (decl);
        }
    }
    for (usize i=0;i<program->typeUseCount;i++) {
        struct String useKey=std_string_OwnedStringView(&program->typeUses[i].key);
        if (String_Equals(&useKey,typeKey)) {
            return (kek_compiler_SelfCFindTypeDecl(program,std_string_OwnedStringView(&program->typeUses[i].baseName)));
        }
    }
    return (0);
}
Status kek_compiler_SelfCFieldType(struct SelfCProgram* program,struct String typeKey,struct String fieldName,struct SelfCTypeInfo* outInfo) {
    kek_compiler_SelfCTypeInfoInitEmpty(program,outInfo);
    struct SelfCDecl* decl=kek_compiler_SelfCFindFieldDecl(program,typeKey);
    if (decl==0) {
        return (Status_Ok);
    }
    for (usize i=0;i<decl->fieldCount;i++) {
        struct SelfCField* field=&program->fields[decl->firstField+i];
        struct String fieldToken=kek_compiler_SelfCTokenText(program,field->fileIndex,field->nameIndex);
        if (String_Equals(&fieldToken,fieldName)) {
            kek_compiler_SelfCTypeInfoDestroy(outInfo);
            return (kek_compiler_SelfCRenderTypeInfo(program,field->fileIndex,field->typeStart,field->typeEnd,outInfo));
        }
    }
    return (Status_Ok);
}
struct SelfCDecl* kek_compiler_SelfCFindMethod(struct SelfCProgram* program,struct String receiverType,struct String name,bool isOperator,u8 operatorCode,usize argCount) {
    for (usize i=0;i<program->declCount;i++) {
        struct SelfCDecl* decl=&program->decls[i];
        if (decl->kind!=SelfCDeclKind_Function||!decl->hasReceiver) {
            continue;
        }
        struct StringBuilder receiver=std_string_StringBuilderNew(program->allocator);
        Status status=kek_compiler_SelfCWriteTypeSuffixFromRange(program,&receiver,decl->fileIndex,decl->receiverStart,decl->receiverEnd);
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
            struct String methodName=kek_compiler_SelfCTokenText(program,decl->fileIndex,decl->nameIndex);
            if (!decl->isOperator&&String_Equals(&methodName,name)) {
                return (decl);
            }
        }
    }
    return (0);
}
Status kek_compiler_SelfCParserInit(struct SelfCProgram* program,struct SelfCEnv* env,usize fileIndex,usize start,usize end,struct String expectedKey,struct String expectedCType,bool expectedIsArray,struct SelfCExprParser* parser) {
    parser->program=program;
    parser->env=env;
    parser->fileIndex=fileIndex;
    parser->pos=start;
    parser->end=end;
    parser->expectedIsArray=expectedIsArray;
    Status status=kek_compiler_SelfCCloneString(program,expectedKey,&parser->expectedTypeKey);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCloneString(program,expectedCType,&parser->expectedCType);
    }
    return (status);
}
Status kek_compiler_SelfCParserDestroy(struct SelfCExprParser* parser) {
    std_string_DestroyOwnedString(&parser->expectedTypeKey);
    std_string_DestroyOwnedString(&parser->expectedCType);
    return (Status_Ok);
}
Status kek_compiler_SelfCCompileExpressionRange(struct SelfCProgram* program,struct SelfCEnv* env,usize fileIndex,usize start,usize end,struct String expectedKey,struct String expectedCType,bool expectedIsArray,struct SelfCExpr* out) {
    struct SelfCExprParser parser={0};
    Status status=kek_compiler_SelfCParserInit(program,env,fileIndex,start,end,expectedKey,expectedCType,expectedIsArray,&parser);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_compiler_SelfCCompileExpression(&parser,1,out);
    kek_compiler_SelfCParserDestroy(&parser);
    return (status);
}
Status kek_compiler_SelfCWriteInitializerList(struct SelfCExprParser* parser,usize start,usize end,struct StringBuilder* out) {
    Status status=StringBuilder_WriteByte(out,'{');
    usize itemStart=start;
    while (itemStart<end) {
        if (kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,itemStart,PunctuationKind_Comma)) {
            itemStart+=1;
            continue;
        }
        usize itemEnd=kek_compiler_SelfCFindTokenAtDepthZero(parser->program,parser->fileIndex,itemStart,end,PunctuationKind_Comma);
        if (itemStart+1<itemEnd&&kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,itemStart+1,OperatorKind_Assign)&&kek_compiler_SelfCIsTokenKind(parser->program,parser->fileIndex,itemStart,TokenKind_Identifier)) {
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(out,'.');
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteToken(parser->program,out,parser->fileIndex,itemStart);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(out,'=');
            }
            struct SelfCExpr value={0};
            if (status==Status_Ok) {
                status=kek_compiler_SelfCCompileExpressionRange(parser->program,parser->env,parser->fileIndex,itemStart+2,itemEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&value);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&value.text));
            }
            if (status==Status_Ok) {
                kek_compiler_SelfCExprDestroy(&value);
            }
        } else {
            struct SelfCExpr value={0};
            if (status==Status_Ok) {
                status=kek_compiler_SelfCCompileExpressionRange(parser->program,parser->env,parser->fileIndex,itemStart,itemEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&value);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&value.text));
            }
            if (status==Status_Ok) {
                kek_compiler_SelfCExprDestroy(&value);
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
Status kek_compiler_SelfCExprFromLiteralBlock(struct SelfCExprParser* parser,usize blockStart,usize blockEnd,struct SelfCExpr* out) {
    Status status=kek_compiler_SelfCExprInitEmpty(parser->program,out);
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
        status=kek_compiler_SelfCWriteInitializerList(parser,blockStart+1,blockEnd,&builder);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCExprFromBuilder(out,&builder);
    }
    StringBuilder_Destroy(&builder);
    if (status==Status_Ok&&expectedKey.len>0) {
        status=kek_compiler_SelfCExprSetType(parser->program,out,expectedKey,expectedCType,0);
    }
    return (status);
}
Status kek_compiler_SelfCCompilePrimary(struct SelfCExprParser* parser,struct SelfCExpr* out) {
    Status status=kek_compiler_SelfCExprInitEmpty(parser->program,out);
    if (status!=Status_Ok) {
        return (status);
    }
    if (parser->pos>=parser->end) {
        return (Status_Ok);
    }
    usize index=parser->pos;
    struct Token token=parser->program[0].tokenFiles[parser->fileIndex].tokens[index];
    if (token.kind==TokenKind_Number) {
        struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
        status=kek_compiler_SelfCWriteNumberToken(parser->program,&builder,parser->fileIndex,index);
        if (status==Status_Ok) {
            status=kek_compiler_SelfCExprFromBuilder(out,&builder);
        }
        StringBuilder_Destroy(&builder);
        if (status==Status_Ok) {
            status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_StringFromCString("int"),std_string_StringFromCString("int"),0);
        }
        parser->pos+=1;
        return (status);
    }
    if (token.kind==TokenKind_String||token.kind==TokenKind_Char) {
        status=kek_compiler_SelfCExprSetText(parser->program,out,kek_compiler_SelfCTokenText(parser->program,parser->fileIndex,index));
        if (status==Status_Ok) {
            if (token.kind==TokenKind_String) {
                status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_StringFromCString("str"),std_string_StringFromCString("str"),1);
            } else {
                status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_StringFromCString("int"),std_string_StringFromCString("int"),0);
            }
        }
        parser->pos+=1;
        return (status);
    }
    if (kek_compiler_SelfCIsKeyword(parser->program,parser->fileIndex,index,KeywordKind_True)||kek_compiler_SelfCIsKeyword(parser->program,parser->fileIndex,index,KeywordKind_False)) {
        if (kek_compiler_SelfCIsKeyword(parser->program,parser->fileIndex,index,KeywordKind_True)) {
            status=kek_compiler_SelfCExprSetCString(parser->program,out,"1");
        } else {
            status=kek_compiler_SelfCExprSetCString(parser->program,out,"0");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_StringFromCString("bool"),std_string_StringFromCString("bool"),0);
        }
        parser->pos+=1;
        return (status);
    }
    if (kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,index,PunctuationKind_LeftParen)) {
        usize match=kek_compiler_SelfCFindMatching(parser->program,parser->fileIndex,index);
        struct SelfCExpr inner={0};
        status=kek_compiler_SelfCCompileExpressionRange(parser->program,parser->env,parser->fileIndex,index+1,match,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&inner);
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
            status=kek_compiler_SelfCExprFromBuilder(out,&builder);
        }
        StringBuilder_Destroy(&builder);
        if (status==Status_Ok) {
            status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_OwnedStringView(&inner.typeKey),std_string_OwnedStringView(&inner.cType),inner.isPointer);
            out->isArray=inner.isArray;
            out->isLvalue=inner.isLvalue;
            out->isPointer=inner.isPointer;
        }
        kek_compiler_SelfCExprDestroy(&inner);
        parser->pos=match+1;
        return (status);
    }
    if (kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,index,PunctuationKind_LeftBrace)) {
        usize match=kek_compiler_SelfCFindMatching(parser->program,parser->fileIndex,index);
        status=kek_compiler_SelfCExprFromLiteralBlock(parser,index,match,out);
        parser->pos=match+1;
        return (status);
    }
    if (kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,index,OperatorKind_Scope)) {
        if (index+1<parser->end) {
            status=kek_compiler_SelfCExprSetText(parser->program,out,kek_compiler_SelfCTokenText(parser->program,parser->fileIndex,index+1));
            parser->pos=index+2;
            return (status);
        }
    }
    if (token.kind==TokenKind_Identifier||token.kind==TokenKind_Keyword) {
        if (index+2<parser->end&&kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,index+1,PunctuationKind_Colon)&&kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,index+2,PunctuationKind_LeftBrace)) {
            usize blockEnd=kek_compiler_SelfCFindMatching(parser->program,parser->fileIndex,index+2);
            struct SelfCTypeInfo typeInfo={0};
            status=kek_compiler_SelfCRenderTypeInfo(parser->program,parser->fileIndex,index,index+1,&typeInfo);
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
                status=kek_compiler_SelfCWriteInitializerList(parser,index+3,blockEnd,&builder);
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCExprFromBuilder(out,&builder);
            }
            StringBuilder_Destroy(&builder);
            if (status==Status_Ok) {
                status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_OwnedStringView(&typeInfo.key),std_string_OwnedStringView(&typeInfo.cType),typeInfo.isPointer);
            }
            kek_compiler_SelfCTypeInfoDestroy(&typeInfo);
            parser->pos=blockEnd+1;
            return (status);
        }
        if (kek_compiler_SelfCIsIdentifierText(parser->program,parser->fileIndex,index,"cast")&&index+1<parser->end&&kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,index+1,OperatorKind_Less)) {
            usize genericEnd=kek_compiler_SelfCFindMatching(parser->program,parser->fileIndex,index+1);
            usize groupStart=genericEnd+1;
            usize groupEnd=kek_compiler_SelfCFindMatching(parser->program,parser->fileIndex,groupStart);
            struct SelfCTypeInfo typeInfo={0};
            status=kek_compiler_SelfCRenderTypeInfo(parser->program,parser->fileIndex,index+2,genericEnd,&typeInfo);
            struct SelfCExpr value={0};
            if (status==Status_Ok) {
                status=kek_compiler_SelfCCompileExpressionRange(parser->program,parser->env,parser->fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&value);
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
                status=kek_compiler_SelfCExprFromBuilder(out,&builder);
            }
            StringBuilder_Destroy(&builder);
            if (status==Status_Ok) {
                status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_OwnedStringView(&typeInfo.key),std_string_OwnedStringView(&typeInfo.cType),typeInfo.isPointer);
            }
            kek_compiler_SelfCExprDestroy(&value);
            kek_compiler_SelfCTypeInfoDestroy(&typeInfo);
            parser->pos=groupEnd+1;
            return (status);
        }
        if ((kek_compiler_SelfCIsIdentifierText(parser->program,parser->fileIndex,index,"sizeof")||kek_compiler_SelfCIsIdentifierText(parser->program,parser->fileIndex,index,"alignof"))&&index+1<parser->end&&kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,index+1,PunctuationKind_LeftParen)) {
            usize groupEnd=kek_compiler_SelfCFindMatching(parser->program,parser->fileIndex,index+1);
            struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
            bool isAlign=kek_compiler_SelfCIsIdentifierText(parser->program,parser->fileIndex,index,"alignof");
            if (isAlign) {
                status=StringBuilder_WriteCString(&builder,"_Alignof(");
            } else {
                status=StringBuilder_WriteCString(&builder,"sizeof(");
            }
            struct SelfCTypeInfo typeInfo={0};
            if (status==Status_Ok) {
                status=kek_compiler_SelfCRenderTypeInfo(parser->program,parser->fileIndex,index+2,groupEnd,&typeInfo);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&typeInfo.cType));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(&builder,')');
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCExprFromBuilder(out,&builder);
            }
            StringBuilder_Destroy(&builder);
            kek_compiler_SelfCTypeInfoDestroy(&typeInfo);
            if (status==Status_Ok) {
                status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_StringFromCString("usize"),std_string_StringFromCString("usize"),0);
            }
            parser->pos=groupEnd+1;
            return (status);
        }
        if (kek_compiler_SelfCIsIdentifierText(parser->program,parser->fileIndex,index,"offsetof")&&index+1<parser->end&&kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,index+1,PunctuationKind_LeftParen)) {
            usize groupEnd=kek_compiler_SelfCFindMatching(parser->program,parser->fileIndex,index+1);
            usize comma=kek_compiler_SelfCFindTokenAtDepthZero(parser->program,parser->fileIndex,index+2,groupEnd,PunctuationKind_Comma);
            struct SelfCTypeInfo typeInfo={0};
            status=kek_compiler_SelfCRenderTypeInfo(parser->program,parser->fileIndex,index+2,comma,&typeInfo);
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
                status=kek_compiler_SelfCWriteTokenRangeRaw(parser->program,&builder,parser->fileIndex,comma+1,groupEnd);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(&builder,')');
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCExprFromBuilder(out,&builder);
            }
            StringBuilder_Destroy(&builder);
            kek_compiler_SelfCTypeInfoDestroy(&typeInfo);
            if (status==Status_Ok) {
                status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_StringFromCString("usize"),std_string_StringFromCString("usize"),0);
            }
            parser->pos=groupEnd+1;
            return (status);
        }
        struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
        if (index+2<parser->end&&kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,index+1,OperatorKind_Scope)) {
            struct String scopeName=kek_compiler_SelfCTokenText(parser->program,parser->fileIndex,index);
            struct String name=kek_compiler_SelfCTokenText(parser->program,parser->fileIndex,index+2);
            struct SelfCDecl* scopedDecl=kek_compiler_SelfCFindFunctionDeclByName(parser->program,name,scopeName);
            if (scopedDecl!=0) {
                status=kek_compiler_SelfCWriteDeclCName(parser->program,&builder,scopedDecl);
            } else {
                status=kek_compiler_SelfCWriteToken(parser->program,&builder,parser->fileIndex,index);
                if (status==Status_Ok) {
                    status=StringBuilder_WriteByte(&builder,'_');
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteToken(parser->program,&builder,parser->fileIndex,index+2);
                }
            }
            parser->pos=index+3;
            if (parser->pos<parser->end&&kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_Less)) {
                usize gEnd=kek_compiler_SelfCFindMatching(parser->program,parser->fileIndex,parser->pos);
                if (gEnd+1<parser->end&&kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,gEnd+1,PunctuationKind_LeftParen)) {
                    usize argStart=parser->pos+1;
                    while (argStart<gEnd) {
                        usize argEnd=kek_compiler_SelfCFindTokenAtDepthZero(parser->program,parser->fileIndex,argStart,gEnd,PunctuationKind_Comma);
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteCString(&builder,"__");
                        }
                        if (status==Status_Ok) {
                            status=kek_compiler_SelfCWriteTypeSuffixFromRange(parser->program,&builder,parser->fileIndex,argStart,argEnd);
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
            status=kek_compiler_SelfCWriteToken(parser->program,&builder,parser->fileIndex,index);
            parser->pos=index+1;
            if (parser->pos<parser->end&&kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_Less)) {
                usize gEnd=kek_compiler_SelfCFindMatching(parser->program,parser->fileIndex,parser->pos);
                if (gEnd+1<parser->end&&kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,gEnd+1,PunctuationKind_LeftParen)) {
                    usize argStart=parser->pos+1;
                    while (argStart<gEnd) {
                        usize argEnd=kek_compiler_SelfCFindTokenAtDepthZero(parser->program,parser->fileIndex,argStart,gEnd,PunctuationKind_Comma);
                        if (status==Status_Ok) {
                            status=StringBuilder_WriteCString(&builder,"__");
                        }
                        if (status==Status_Ok) {
                            status=kek_compiler_SelfCWriteTypeSuffixFromRange(parser->program,&builder,parser->fileIndex,argStart,argEnd);
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
            status=kek_compiler_SelfCExprFromBuilder(out,&builder);
        }
        StringBuilder_Destroy(&builder);
        struct SelfCLocal* local=kek_compiler_SelfCEnvFind(parser->env,kek_compiler_SelfCTokenText(parser->program,parser->fileIndex,index));
        if (status==Status_Ok&&local!=0) {
            status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_OwnedStringView(&local->typeKey),std_string_OwnedStringView(&local->cType),local->isPointer);
            out->isArray=local->isArray;
            out->isPointer=local->isPointer;
            out->isLvalue=1;
            struct OwnedString arrLen={0};
            if (status==Status_Ok) {
                status=kek_compiler_SelfCCloneString(parser->program,std_string_OwnedStringView(&local->arrayLen),&arrLen);
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCReplaceOwned(&out->arrayLen,arrLen);
            }
        } else {
            if (status==Status_Ok) {
                struct SelfCDecl* functionDecl=kek_compiler_SelfCFindFunctionDeclByName(parser->program,kek_compiler_SelfCTokenText(parser->program,parser->fileIndex,index),std_string_StringFromCString(""));
                if (functionDecl!=0) {
                    struct StringBuilder functionName=std_string_StringBuilderNew(parser->program[0].allocator);
                    status=kek_compiler_SelfCWriteDeclCName(parser->program,&functionName,functionDecl);
                    struct String currentName=std_string_OwnedStringView(&out->text);
                    struct Result__usize suffixStart=String_FindByte(&currentName,'_');
                    if (status==Status_Ok&&suffixStart.status==Status_Ok&&suffixStart.value+1<currentName.len&&currentName.data[suffixStart.value+1]=='_') {
                        status=StringBuilder_WriteString(&functionName,String_Slice(&currentName,suffixStart.value,currentName.len-suffixStart.value));
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCExprFromBuilder(out,&functionName);
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
Status kek_compiler_SelfCWriteCallArgs(struct SelfCExprParser* parser,usize start,usize end,struct StringBuilder* out) {
    usize itemStart=start;
    bool first=1;
    Status status=Status_Ok;
    while (itemStart<end) {
        if (kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,itemStart,PunctuationKind_Comma)) {
            itemStart+=1;
            continue;
        }
        usize itemEnd=kek_compiler_SelfCFindTokenAtDepthZero(parser->program,parser->fileIndex,itemStart,end,PunctuationKind_Comma);
        usize exprStart=itemStart;
        usize assign=kek_compiler_SelfCFindTokenAtDepthZero(parser->program,parser->fileIndex,itemStart,itemEnd,PunctuationKind_Colon);
        if (assign<itemEnd) {
            exprStart=assign+1;
        } else {
            for (usize i=itemStart;i<itemEnd;i++) {
                if (kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,i,OperatorKind_Assign)) {
                    exprStart=i+1;
                    break;
                }
            }
        }
        struct SelfCExpr arg={0};
        if (status==Status_Ok) {
            status=kek_compiler_SelfCCompileExpressionRange(parser->program,parser->env,parser->fileIndex,exprStart,itemEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&arg);
        }
        if (status==Status_Ok&&!first) {
            status=StringBuilder_WriteByte(out,',');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&arg.text));
        }
        if (status==Status_Ok) {
            kek_compiler_SelfCExprDestroy(&arg);
        }
        first=0;
        if (status!=Status_Ok||itemEnd>=end) {
            break;
        }
        itemStart=itemEnd+1;
    }
    return (status);
}
usize kek_compiler_SelfCCountCallArgs(struct SelfCProgram* program,usize fileIndex,usize start,usize end) {
    if (start>=end) {
        return (0);
    }
    usize count=1;
    usize i=start;
    while (i<end) {
        usize comma=kek_compiler_SelfCFindTokenAtDepthZero(program,fileIndex,i,end,PunctuationKind_Comma);
        if (comma>=end) {
            break;
        }
        count+=1;
        i=comma+1;
    }
    return (count);
}
Status kek_compiler_SelfCApplyPostfix(struct SelfCExprParser* parser,struct SelfCExpr* expr) {
    while (parser->pos<parser->end) {
        if (kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,parser->pos,PunctuationKind_LeftParen)) {
            usize groupStart=parser->pos;
            usize groupEnd=kek_compiler_SelfCFindMatching(parser->program,parser->fileIndex,groupStart);
            struct String funcText=std_string_OwnedStringView(&expr->text);
            if (String_EqualsCString(&funcText,"len")) {
                struct SelfCExpr arg={0};
                Status status=kek_compiler_SelfCCompileExpressionRange(parser->program,parser->env,parser->fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&arg);
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
                    status=kek_compiler_SelfCExprFromBuilder(expr,&lenBuilder);
                }
                StringBuilder_Destroy(&lenBuilder);
                kek_compiler_SelfCExprDestroy(&arg);
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCExprSetType(parser->program,expr,std_string_StringFromCString("usize"),std_string_StringFromCString("usize"),0);
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
                status=kek_compiler_SelfCWriteCallArgs(parser,groupStart+1,groupEnd,&builder);
            }
            usize argCount=kek_compiler_SelfCCountCallArgs(parser->program,parser->fileIndex,groupStart+1,groupEnd);
            if (status==Status_Ok&&String_EqualsCString(&funcText,"add")&&argCount==1) {
                status=StringBuilder_WriteCString(&builder,",0");
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(&builder,')');
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCExprFromBuilder(expr,&builder);
            }
            StringBuilder_Destroy(&builder);
            expr->isLvalue=0;
            parser->pos=groupEnd+1;
            if (status!=Status_Ok) {
                return (status);
            }
            continue;
        }
        if (kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,parser->pos,PunctuationKind_LeftBracket)) {
            usize indexStart=parser->pos;
            usize indexEnd=kek_compiler_SelfCFindMatching(parser->program,parser->fileIndex,indexStart);
            struct SelfCExpr indexExpr={0};
            Status status=kek_compiler_SelfCCompileExpressionRange(parser->program,parser->env,parser->fileIndex,indexStart+1,indexEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&indexExpr);
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
                status=kek_compiler_SelfCExprFromBuilder(expr,&builder);
            }
            StringBuilder_Destroy(&builder);
            kek_compiler_SelfCExprDestroy(&indexExpr);
            expr->isLvalue=1;
            expr->isArray=0;
            struct String indexedKey=std_string_OwnedStringView(&expr->typeKey);
            if (indexedKey.len>0&&indexedKey.data[indexedKey.len-1]=='*') {
                struct OwnedString newKey={0};
                status=kek_compiler_SelfCCloneString(parser->program,String_Slice(&indexedKey,0,indexedKey.len-1),&newKey);
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCReplaceOwned(&expr->typeKey,newKey);
                }
                struct OwnedString newCType={0};
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCMakeCTypeFromKey(parser->program,std_string_OwnedStringView(&expr->typeKey),&newCType);
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCReplaceOwned(&expr->cType,newCType);
                }
                expr->isPointer=0;
            }
            parser->pos=indexEnd+1;
            if (status!=Status_Ok) {
                return (status);
            }
            continue;
        }
        if (kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,parser->pos,PunctuationKind_Dot)) {
            usize nameIndex=parser->pos+1;
            struct String fieldName=kek_compiler_SelfCTokenText(parser->program,parser->fileIndex,nameIndex);
            if (nameIndex+1<parser->end&&kek_compiler_SelfCIsPunctuation(parser->program,parser->fileIndex,nameIndex+1,PunctuationKind_LeftParen)) {
                usize groupStart=nameIndex+1;
                usize groupEnd=kek_compiler_SelfCFindMatching(parser->program,parser->fileIndex,groupStart);
                struct String receiverKey=std_string_OwnedStringView(&expr->typeKey);
                struct String methodReceiverKey=receiverKey;
                if (expr->isPointer&&receiverKey.len>0&&receiverKey.data[receiverKey.len-1]=='*') {
                    methodReceiverKey=String_Slice(&receiverKey,0,receiverKey.len-1);
                }
                struct SelfCDecl* method=kek_compiler_SelfCFindMethod(parser->program,methodReceiverKey,fieldName,0,0,0);
                struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
                Status status=Status_Ok;
                if (method!=0) {
                    status=kek_compiler_SelfCWriteDeclCName(parser->program,&builder,method);
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
                        status=kek_compiler_SelfCWriteCallArgs(parser,groupStart+1,groupEnd,&builder);
                    }
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteByte(&builder,')');
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCExprFromBuilder(expr,&builder);
                }
                StringBuilder_Destroy(&builder);
                expr->isLvalue=0;
                if (method!=0) {
                    struct SelfCTypeInfo returnInfo={0};
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCRenderTypeInfo(parser->program,method->fileIndex,method->returnStart,method->returnEnd,&returnInfo);
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCExprSetType(parser->program,expr,std_string_OwnedStringView(&returnInfo.key),std_string_OwnedStringView(&returnInfo.cType),returnInfo.isPointer);
                    }
                    if (status==Status_Ok) {
                        kek_compiler_SelfCTypeInfoDestroy(&returnInfo);
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
                status=kek_compiler_SelfCExprFromBuilder(expr,&builder);
            }
            StringBuilder_Destroy(&builder);
            struct SelfCTypeInfo fieldType={0};
            if (status==Status_Ok) {
                status=kek_compiler_SelfCFieldType(parser->program,std_string_OwnedStringView(&expr->typeKey),fieldName,&fieldType);
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCExprSetType(parser->program,expr,std_string_OwnedStringView(&fieldType.key),std_string_OwnedStringView(&fieldType.cType),fieldType.isPointer);
            }
            if (status==Status_Ok) {
                kek_compiler_SelfCTypeInfoDestroy(&fieldType);
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
Status kek_compiler_SelfCCompileUnary(struct SelfCExprParser* parser,struct SelfCExpr* out) {
    if (parser->pos<parser->end&&(kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_Minus)||kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_LogicalNot)||kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_BitwiseNot)||kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_BitwiseAnd)||kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,parser->pos,OperatorKind_Multiply))) {
        usize op=parser->pos;
        parser->pos+=1;
        struct SelfCExpr inner={0};
        Status status=kek_compiler_SelfCCompileUnary(parser,&inner);
        struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteToken(parser->program,&builder,parser->fileIndex,op);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&inner.text));
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCExprInitEmpty(parser->program,out);
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCExprFromBuilder(out,&builder);
        }
        StringBuilder_Destroy(&builder);
        if (status==Status_Ok) {
            struct SelfCDecl* unaryMethod=0;
            if (kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,op,OperatorKind_Minus)) {
                unaryMethod=kek_compiler_SelfCFindMethod(parser->program,std_string_OwnedStringView(&inner.typeKey),std_string_StringFromCString(""),1,kek_compiler_SelfCOperatorCode(parser->program,parser->fileIndex,op),0);
            }
            if (unaryMethod!=0) {
                struct StringBuilder call=std_string_StringBuilderNew(parser->program[0].allocator);
                status=kek_compiler_SelfCWriteDeclCName(parser->program,&call,unaryMethod);
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
                    status=kek_compiler_SelfCExprFromBuilder(out,&call);
                }
                StringBuilder_Destroy(&call);
            } else {
                if (kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,op,OperatorKind_BitwiseAnd)) {
                    struct StringBuilder key=std_string_StringBuilderNew(parser->program[0].allocator);
                    status=StringBuilder_WriteString(&key,std_string_OwnedStringView(&inner.typeKey));
                    if (status==Status_Ok) {
                        status=StringBuilder_WriteByte(&key,'*');
                    }
                    struct OwnedString keyOwned={0};
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCDetachBuilder(&key,&keyOwned);
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
                            status=kek_compiler_SelfCDetachBuilder(&ct,&cOwned);
                        }
                        StringBuilder_Destroy(&ct);
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_OwnedStringView(&keyOwned),std_string_OwnedStringView(&cOwned),1);
                        std_string_DestroyOwnedString(&keyOwned);
                        std_string_DestroyOwnedString(&cOwned);
                    }
                } else {
                    status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_OwnedStringView(&inner.typeKey),std_string_OwnedStringView(&inner.cType),inner.isPointer);
                }
            }
        }
        kek_compiler_SelfCExprDestroy(&inner);
        return (status);
    }
    Status status=kek_compiler_SelfCCompilePrimary(parser,out);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCApplyPostfix(parser,out);
    }
    return (status);
}
Status kek_compiler_SelfCCompileBinaryOperation(struct SelfCExprParser* parser,struct SelfCExpr* left,usize operatorIndex,struct SelfCExpr* right,struct SelfCExpr* out) {
    Status status=kek_compiler_SelfCExprInitEmpty(parser->program,out);
    if (status!=Status_Ok) {
        return (status);
    }
    u8 opCode=kek_compiler_SelfCOperatorCode(parser->program,parser->fileIndex,operatorIndex);
    struct SelfCDecl* method=0;
    usize operatorArgCount=1;
    if (kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_PlusAssign)||kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_MinusAssign)) {
        operatorArgCount=1;
    }
    if (opCode!=0&&std_string_OwnedStringView(&left->typeKey).len>0) {
        method=kek_compiler_SelfCFindMethod(parser->program,std_string_OwnedStringView(&left->typeKey),std_string_StringFromCString(""),1,opCode,operatorArgCount);
    }
    if (method!=0) {
        struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
        status=kek_compiler_SelfCWriteDeclCName(parser->program,&builder,method);
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
            status=kek_compiler_SelfCExprFromBuilder(out,&builder);
        }
        StringBuilder_Destroy(&builder);
        if (status==Status_Ok) {
            struct SelfCTypeInfo returnInfo={0};
            status=kek_compiler_SelfCRenderTypeInfo(parser->program,method->fileIndex,method->returnStart,method->returnEnd,&returnInfo);
            if (status==Status_Ok) {
                status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_OwnedStringView(&returnInfo.key),std_string_OwnedStringView(&returnInfo.cType),returnInfo.isPointer);
            }
            if (status==Status_Ok) {
                kek_compiler_SelfCTypeInfoDestroy(&returnInfo);
            }
        }
        return (status);
    }
    struct StringBuilder builder=std_string_StringBuilderNew(parser->program[0].allocator);
    status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&left->text));
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteToken(parser->program,&builder,parser->fileIndex,operatorIndex);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(&builder,std_string_OwnedStringView(&right->text));
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCExprFromBuilder(out,&builder);
    }
    StringBuilder_Destroy(&builder);
    if (status==Status_Ok) {
        if (kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_Equal)||kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_NotEqual)||kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_Less)||kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_LessEqual)||kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_Greater)||kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_GreaterEqual)||kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_LogicalAnd)||kek_compiler_SelfCIsOperator(parser->program,parser->fileIndex,operatorIndex,OperatorKind_LogicalOr)) {
            status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_StringFromCString("bool"),std_string_StringFromCString("bool"),0);
        } else {
            status=kek_compiler_SelfCExprSetType(parser->program,out,std_string_OwnedStringView(&left->typeKey),std_string_OwnedStringView(&left->cType),left->isPointer);
        }
    }
    return (status);
}
Status kek_compiler_SelfCCompileExpression(struct SelfCExprParser* parser,u8 minPrecedence,struct SelfCExpr* out) {
    struct SelfCExpr left={0};
    Status status=kek_compiler_SelfCCompileUnary(parser,&left);
    if (status!=Status_Ok) {
        return (status);
    }
    while (parser->pos<parser->end) {
        u8 precedence=kek_compiler_SelfCOperatorPrecedence(parser->program,parser->fileIndex,parser->pos);
        if (precedence==0||precedence<minPrecedence) {
            break;
        }
        usize op=parser->pos;
        parser->pos+=1;
        u8 nextMin=precedence+1;
        if (kek_compiler_SelfCOperatorRightAssociative(parser->program,parser->fileIndex,op)) {
            nextMin=precedence;
        }
        struct SelfCExpr right={0};
        status=kek_compiler_SelfCCompileExpression(parser,nextMin,&right);
        if (status!=Status_Ok) {
            kek_compiler_SelfCExprDestroy(&left);
            return (status);
        }
        struct SelfCExpr combined={0};
        status=kek_compiler_SelfCCompileBinaryOperation(parser,&left,op,&right,&combined);
        kek_compiler_SelfCExprDestroy(&left);
        kek_compiler_SelfCExprDestroy(&right);
        if (status!=Status_Ok) {
            return (status);
        }
        left=combined;
    }
    out[0]=left;
    return (Status_Ok);
}
Status kek_compiler_SelfCWriteIndent(struct StringBuilder* out,usize indent) {
    for (usize i=0;i<indent;i++) {
        Status status=StringBuilder_WriteCString(out,"    ");
        if (status!=Status_Ok) {
            return (status);
        }
    }
    return (Status_Ok);
}
usize kek_compiler_SelfCFindStatementSemicolon(struct SelfCProgram* program,usize fileIndex,usize start,usize end) {
    usize i=start;
    while (i<end) {
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftParen)||kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftBrace)||kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_LeftBracket)) {
            i=kek_compiler_SelfCSkipDelimited(program,fileIndex,i);
            continue;
        }
        if (kek_compiler_SelfCIsPunctuation(program,fileIndex,i,PunctuationKind_Semicolon)) {
            return (i);
        }
        i+=1;
    }
    return (end);
}
Status kek_compiler_SelfCArrayLenString(struct SelfCProgram* program,usize fileIndex,usize start,usize end,struct OwnedString* out) {
    struct StringBuilder builder=std_string_StringBuilderNew(program->allocator);
    Status status=kek_compiler_SelfCWriteTokenRangeRaw(program,&builder,fileIndex,start,end);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCDetachBuilder(&builder,out);
    }
    StringBuilder_Destroy(&builder);
    return (status);
}
bool kek_compiler_SelfCFieldDefaultIsZero(struct SelfCProgram* program,struct SelfCField* field) {
    if (!field->hasDefault) {
        return (1);
    }
    if (field->defaultEnd!=field->defaultStart+1) {
        return (0);
    }
    if (kek_compiler_SelfCIsTokenKind(program,field->fileIndex,field->defaultStart,TokenKind_Number)&&kek_compiler_SelfCTokenEquals(program,field->fileIndex,field->defaultStart,"0")) {
        return (1);
    }
    if (kek_compiler_SelfCIsIdentifierText(program,field->fileIndex,field->defaultStart,"false")) {
        return (1);
    }
    return (0);
}
Status kek_compiler_SelfCWriteDefaultInitializer(struct SelfCProgram* program,struct StringBuilder* out,struct String typeKey) {
    struct SelfCDecl* decl=kek_compiler_SelfCFindFieldDecl(program,typeKey);
    if (decl!=0&&decl->kind==SelfCDeclKind_Union) {
        return (StringBuilder_WriteCString(out,"{0}"));
    }
    if (decl==0||decl->kind!=SelfCDeclKind_Struct) {
        return (StringBuilder_WriteCString(out,"0"));
    }
    bool hasNonZeroDefault=0;
    for (usize i=0;i<decl->fieldCount;i++) {
        struct SelfCField* field=&program->fields[decl->firstField+i];
        if (field->hasDefault&&!kek_compiler_SelfCFieldDefaultIsZero(program,field)) {
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
        struct SelfCField* field=&program->fields[decl->firstField+i];
        if (!field->hasDefault||kek_compiler_SelfCFieldDefaultIsZero(program,field)) {
            continue;
        }
        if (!first&&status==Status_Ok) {
            status=StringBuilder_WriteByte(out,',');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,'.');
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteToken(program,out,field->fileIndex,field->nameIndex);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,'=');
        }
        struct SelfCExpr defaultExpr={0};
        struct SelfCEnv emptyEnv={0};
        kek_compiler_SelfCEnvInit(program,&emptyEnv);
        if (status==Status_Ok) {
            status=kek_compiler_SelfCCompileExpressionRange(program,&emptyEnv,field->fileIndex,field->defaultStart,field->defaultEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&defaultExpr);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&defaultExpr.text));
        }
        if (status==Status_Ok) {
            kek_compiler_SelfCExprDestroy(&defaultExpr);
        }
        kek_compiler_SelfCEnvDestroy(&emptyEnv);
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
Status kek_compiler_SelfCWriteVarDecl(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize semicolon,usize indent) {
    usize colon=kek_compiler_SelfCFindTopLevelColon(program,fileIndex,start,semicolon);
    if (colon>=semicolon) {
        return (Status_Invalid);
    }
    usize nameIndex=colon+1;
    usize afterName=nameIndex+1;
    bool isArray=0;
    usize arrayStart=semicolon;
    usize arrayEnd=semicolon;
    if (afterName<semicolon&&kek_compiler_SelfCIsPunctuation(program,fileIndex,afterName,PunctuationKind_LeftBracket)) {
        isArray=1;
        arrayStart=afterName+1;
        arrayEnd=kek_compiler_SelfCFindMatching(program,fileIndex,afterName);
        afterName=arrayEnd+1;
        while (afterName<semicolon&&kek_compiler_SelfCIsPunctuation(program,fileIndex,afterName,PunctuationKind_LeftBracket)) {
            afterName=kek_compiler_SelfCFindMatching(program,fileIndex,afterName)+1;
        }
    }
    usize arraySuffixStart=nameIndex+1;
    usize arraySuffixEnd=afterName;
    usize initStart=semicolon;
    bool hasInit=0;
    for (usize i=afterName;i<semicolon;i++) {
        if (kek_compiler_SelfCIsOperator(program,fileIndex,i,OperatorKind_Assign)) {
            initStart=i+1;
            hasInit=1;
            break;
        }
    }
    struct SelfCTypeInfo typeInfo={0};
    Status status=kek_compiler_SelfCRenderTypeInfo(program,fileIndex,start,colon,&typeInfo);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_compiler_SelfCWriteIndent(out,indent);
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&typeInfo.cType));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,' ');
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteToken(program,out,fileIndex,nameIndex);
    }
    struct OwnedString arrayLen={0};
    if (isArray) {
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteTokenRangeRaw(program,out,fileIndex,arraySuffixStart,arraySuffixEnd);
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCArrayLenString(program,fileIndex,arrayStart,arrayEnd,&arrayLen);
        }
    } else {
        if (status==Status_Ok) {
            status=kek_compiler_SelfCMakeOwnedEmpty(program,&arrayLen);
        }
    }
    if (hasInit) {
        struct SelfCExpr init={0};
        if (status==Status_Ok) {
            status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,initStart,semicolon,std_string_OwnedStringView(&typeInfo.key),std_string_OwnedStringView(&typeInfo.cType),isArray,&init);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,'=');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&init.text));
        }
        if (status==Status_Ok) {
            kek_compiler_SelfCExprDestroy(&init);
        }
    } else {
        if (!isArray) {
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(out,'=');
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteDefaultInitializer(program,out,std_string_OwnedStringView(&typeInfo.key));
            }
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCEnvAdd(program,env,kek_compiler_SelfCTokenText(program,fileIndex,nameIndex),std_string_OwnedStringView(&typeInfo.key),std_string_OwnedStringView(&typeInfo.cType),isArray,std_string_OwnedStringView(&arrayLen),typeInfo.isPointer);
    }
    std_string_DestroyOwnedString(&arrayLen);
    kek_compiler_SelfCTypeInfoDestroy(&typeInfo);
    return (status);
}
Status kek_compiler_SelfCWriteExprStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize semicolon,usize indent) {
    if (start>=semicolon) {
        return (Status_Ok);
    }
    if (start+2==semicolon&&kek_compiler_SelfCIsOperator(program,fileIndex,start+1,OperatorKind_Plus)&&kek_compiler_SelfCIsOperator(program,fileIndex,start+2,OperatorKind_Plus)) {
    }
    struct SelfCExpr expr={0};
    Status status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,start,semicolon,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&expr);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&expr.text));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    kek_compiler_SelfCExprDestroy(&expr);
    return (status);
}
Status kek_compiler_SelfCWriteIfStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    usize groupStart=kek_compiler_SelfCFindNextGroup(program,fileIndex,start,kek_compiler_SelfCFileTokenCount(program,fileIndex));
    usize groupEnd=kek_compiler_SelfCFindMatching(program,fileIndex,groupStart);
    usize blockStart=groupEnd+1;
    usize blockEnd=blockStart;
    struct SelfCExpr cond={0};
    Status status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&cond);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteIndent(out,indent);
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
    kek_compiler_SelfCExprDestroy(&cond);
    if (blockStart<kek_compiler_SelfCFileTokenCount(program,fileIndex)&&kek_compiler_SelfCIsPunctuation(program,fileIndex,blockStart,PunctuationKind_LeftBrace)) {
        blockEnd=kek_compiler_SelfCFindMatching(program,fileIndex,blockStart);
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"{\n");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteBlock(program,out,env,fileIndex,blockStart+1,blockEnd,indent+1);
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"}");
        }
        outNext[0]=blockEnd+1;
    } else {
        usize semicolon=kek_compiler_SelfCFindStatementSemicolon(program,fileIndex,blockStart,kek_compiler_SelfCFileTokenCount(program,fileIndex));
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"{\n");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteExprStatement(program,out,env,fileIndex,blockStart,semicolon,indent+1);
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"}");
        }
        outNext[0]=semicolon+1;
    }
    if (outNext[0]<kek_compiler_SelfCFileTokenCount(program,fileIndex)&&kek_compiler_SelfCIsKeyword(program,fileIndex,outNext[0],KeywordKind_Else)) {
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out," else ");
        }
        usize elseStart=outNext[0]+1;
        if (elseStart<kek_compiler_SelfCFileTokenCount(program,fileIndex)&&kek_compiler_SelfCIsPunctuation(program,fileIndex,elseStart,PunctuationKind_LeftBrace)) {
            usize elseEnd=kek_compiler_SelfCFindMatching(program,fileIndex,elseStart);
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"{\n");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteBlock(program,out,env,fileIndex,elseStart+1,elseEnd,indent+1);
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteIndent(out,indent);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"}");
            }
            outNext[0]=elseEnd+1;
        } else {
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"{\n");
            }
            usize semicolon=kek_compiler_SelfCFindStatementSemicolon(program,fileIndex,elseStart,kek_compiler_SelfCFileTokenCount(program,fileIndex));
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteExprStatement(program,out,env,fileIndex,elseStart,semicolon,indent+1);
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteIndent(out,indent);
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
Status kek_compiler_SelfCWriteWhileStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    usize groupStart=kek_compiler_SelfCFindNextGroup(program,fileIndex,start,kek_compiler_SelfCFileTokenCount(program,fileIndex));
    usize groupEnd=kek_compiler_SelfCFindMatching(program,fileIndex,groupStart);
    usize blockStart=groupEnd+1;
    usize blockEnd=kek_compiler_SelfCFindMatching(program,fileIndex,blockStart);
    struct SelfCExpr cond={0};
    Status status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&cond);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"while (");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&cond.text));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,") {\n");
    }
    kek_compiler_SelfCExprDestroy(&cond);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteBlock(program,out,env,fileIndex,blockStart+1,blockEnd,indent+1);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"}\n");
    }
    outNext[0]=blockEnd+1;
    return (status);
}
Status kek_compiler_SelfCWriteDoStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    usize blockStart=start+1;
    usize blockEnd=kek_compiler_SelfCFindMatching(program,fileIndex,blockStart);
    usize whileIndex=blockEnd+1;
    usize groupStart=kek_compiler_SelfCFindNextGroup(program,fileIndex,whileIndex,kek_compiler_SelfCFileTokenCount(program,fileIndex));
    usize groupEnd=kek_compiler_SelfCFindMatching(program,fileIndex,groupStart);
    struct SelfCExpr cond={0};
    Status status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&cond);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"do {\n");
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteBlock(program,out,env,fileIndex,blockStart+1,blockEnd,indent+1);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"} while (");
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&cond.text));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,");\n");
    }
    kek_compiler_SelfCExprDestroy(&cond);
    outNext[0]=groupEnd+2;
    return (status);
}
Status kek_compiler_SelfCWriteForStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    usize groupStart=kek_compiler_SelfCFindNextGroup(program,fileIndex,start,kek_compiler_SelfCFileTokenCount(program,fileIndex));
    usize groupEnd=kek_compiler_SelfCFindMatching(program,fileIndex,groupStart);
    usize firstSemi=kek_compiler_SelfCFindTokenAtDepthZero(program,fileIndex,groupStart+1,groupEnd,PunctuationKind_Semicolon);
    usize secondSemi=kek_compiler_SelfCFindTokenAtDepthZero(program,fileIndex,firstSemi+1,groupEnd,PunctuationKind_Semicolon);
    usize blockStart=groupEnd+1;
    usize blockEnd=kek_compiler_SelfCFindMatching(program,fileIndex,blockStart);
    Status status=kek_compiler_SelfCWriteIndent(out,indent);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"for (");
    }
    usize colon=kek_compiler_SelfCFindTopLevelColon(program,fileIndex,groupStart+1,firstSemi);
    if (colon<firstSemi) {
        struct SelfCTypeInfo typeInfo={0};
        if (status==Status_Ok) {
            status=kek_compiler_SelfCRenderTypeInfo(program,fileIndex,groupStart+1,colon,&typeInfo);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&typeInfo.cType));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,' ');
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteToken(program,out,fileIndex,colon+1);
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCEnvAdd(program,env,kek_compiler_SelfCTokenText(program,fileIndex,colon+1),std_string_OwnedStringView(&typeInfo.key),std_string_OwnedStringView(&typeInfo.cType),0,std_string_StringFromCString(""),typeInfo.isPointer);
        }
        usize eq=colon+2;
        while (eq<firstSemi&&!kek_compiler_SelfCIsOperator(program,fileIndex,eq,OperatorKind_Assign)) {
            eq+=1;
        }
        if (eq<firstSemi) {
            struct SelfCExpr init={0};
            if (status==Status_Ok) {
                status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,eq+1,firstSemi,std_string_OwnedStringView(&typeInfo.key),std_string_OwnedStringView(&typeInfo.cType),0,&init);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(out,'=');
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&init.text));
            }
            if (status==Status_Ok) {
                kek_compiler_SelfCExprDestroy(&init);
            }
        }
        kek_compiler_SelfCTypeInfoDestroy(&typeInfo);
    } else {
        struct SelfCExpr initExpr={0};
        if (status==Status_Ok) {
            status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,groupStart+1,firstSemi,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&initExpr);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&initExpr.text));
        }
        if (status==Status_Ok) {
            kek_compiler_SelfCExprDestroy(&initExpr);
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,';');
    }
    struct SelfCExpr cond={0};
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,firstSemi+1,secondSemi,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&cond);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&cond.text));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,';');
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteTokenRangeRaw(program,out,fileIndex,secondSemi+1,groupEnd);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,") {\n");
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteBlock(program,out,env,fileIndex,blockStart+1,blockEnd,indent+1);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"}\n");
    }
    kek_compiler_SelfCExprDestroy(&cond);
    outNext[0]=blockEnd+1;
    return (status);
}
Status kek_compiler_SelfCWriteEachStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    usize genericStart=start+1;
    usize genericEnd=kek_compiler_SelfCFindMatching(program,fileIndex,genericStart);
    usize groupStart=genericEnd+1;
    usize groupEnd=kek_compiler_SelfCFindMatching(program,fileIndex,groupStart);
    usize blockStart=groupEnd+1;
    usize blockEnd=kek_compiler_SelfCFindMatching(program,fileIndex,blockStart);
    usize comma=kek_compiler_SelfCFindTokenAtDepthZero(program,fileIndex,genericStart+1,genericEnd,PunctuationKind_Comma);
    usize valueTypeStart=genericStart+1;
    usize valueTypeEnd=genericEnd;
    usize valueNameIndex=genericStart+1;
    bool hasIndex=0;
    usize indexTypeStart=genericStart+1;
    usize indexTypeEnd=genericStart+1;
    usize indexNameIndex=genericStart+1;
    if (comma<genericEnd) {
        hasIndex=1;
        usize indexColon=kek_compiler_SelfCFindTopLevelColon(program,fileIndex,genericStart+1,comma);
        indexTypeStart=genericStart+1;
        indexTypeEnd=indexColon;
        indexNameIndex=indexColon+1;
        usize valueColon=kek_compiler_SelfCFindTopLevelColon(program,fileIndex,comma+1,genericEnd);
        valueTypeStart=comma+1;
        valueTypeEnd=valueColon;
        valueNameIndex=valueColon+1;
    } else {
        usize valueColon=kek_compiler_SelfCFindTopLevelColon(program,fileIndex,genericStart+1,genericEnd);
        valueTypeStart=genericStart+1;
        valueTypeEnd=valueColon;
        valueNameIndex=valueColon+1;
    }
    struct SelfCTypeInfo valueType={0};
    Status status=kek_compiler_SelfCRenderTypeInfo(program,fileIndex,valueTypeStart,valueTypeEnd,&valueType);
    if (status!=Status_Ok) {
        return (status);
    }
    if (groupStart+2<groupEnd&&kek_compiler_SelfCIsIdentifierText(program,fileIndex,groupStart+1,"range")) {
        usize rangeGroup=groupStart+4;
        while (rangeGroup<groupEnd&&!kek_compiler_SelfCIsPunctuation(program,fileIndex,rangeGroup,PunctuationKind_LeftParen)) {
            rangeGroup+=1;
        }
        usize rangeEnd=kek_compiler_SelfCFindMatching(program,fileIndex,rangeGroup);
        usize firstComma=kek_compiler_SelfCFindTokenAtDepthZero(program,fileIndex,rangeGroup+1,rangeEnd,PunctuationKind_Comma);
        usize secondComma=kek_compiler_SelfCFindTokenAtDepthZero(program,fileIndex,firstComma+1,rangeEnd,PunctuationKind_Comma);
        struct SelfCExpr beginExpr={0};
        struct SelfCExpr endExpr={0};
        struct SelfCExpr stepExpr={0};
        status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,rangeGroup+1,firstComma,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&beginExpr);
        usize rangeValueEnd=rangeEnd;
        if (secondComma<rangeEnd) {
            rangeValueEnd=secondComma;
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,firstComma+1,rangeValueEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&endExpr);
        }
        if (secondComma<rangeEnd) {
            if (status==Status_Ok) {
                status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,secondComma+1,rangeEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&stepExpr);
            }
        } else {
            if (status==Status_Ok) {
                kek_compiler_SelfCExprInitEmpty(program,&stepExpr);
                status=kek_compiler_SelfCExprSetCString(program,&stepExpr,"1");
            }
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteIndent(out,indent);
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
            status=kek_compiler_SelfCWriteToken(program,out,fileIndex,valueNameIndex);
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
            status=kek_compiler_SelfCWriteToken(program,out,fileIndex,valueNameIndex);
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
            status=kek_compiler_SelfCWriteToken(program,out,fileIndex,valueNameIndex);
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
            status=kek_compiler_SelfCEnvAdd(program,env,kek_compiler_SelfCTokenText(program,fileIndex,valueNameIndex),std_string_OwnedStringView(&valueType.key),std_string_OwnedStringView(&valueType.cType),0,std_string_StringFromCString(""),valueType.isPointer);
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteBlock(program,out,env,fileIndex,blockStart+1,blockEnd,indent+1);
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"}\n");
        }
        kek_compiler_SelfCExprDestroy(&beginExpr);
        kek_compiler_SelfCExprDestroy(&endExpr);
        kek_compiler_SelfCExprDestroy(&stepExpr);
    } else {
        struct SelfCExpr arrayExpr={0};
        status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&arrayExpr);
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"for (");
        }
        if (hasIndex) {
            struct SelfCTypeInfo indexType={0};
            if (status==Status_Ok) {
                status=kek_compiler_SelfCRenderTypeInfo(program,fileIndex,indexTypeStart,indexTypeEnd,&indexType);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&indexType.cType));
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteByte(out,' ');
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteToken(program,out,fileIndex,indexNameIndex);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"=0;");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteToken(program,out,fileIndex,indexNameIndex);
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
                status=kek_compiler_SelfCWriteToken(program,out,fileIndex,indexNameIndex);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"++) {\n");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCEnvAdd(program,env,kek_compiler_SelfCTokenText(program,fileIndex,indexNameIndex),std_string_OwnedStringView(&indexType.key),std_string_OwnedStringView(&indexType.cType),0,std_string_StringFromCString(""),indexType.isPointer);
            }
            kek_compiler_SelfCTypeInfoDestroy(&indexType);
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
            status=kek_compiler_SelfCWriteIndent(out,indent+1);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&valueType.cType));
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,' ');
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteToken(program,out,fileIndex,valueNameIndex);
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
                status=kek_compiler_SelfCWriteToken(program,out,fileIndex,indexNameIndex);
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
            status=kek_compiler_SelfCEnvAdd(program,env,kek_compiler_SelfCTokenText(program,fileIndex,valueNameIndex),std_string_OwnedStringView(&valueType.key),std_string_OwnedStringView(&valueType.cType),0,std_string_StringFromCString(""),valueType.isPointer);
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteBlock(program,out,env,fileIndex,blockStart+1,blockEnd,indent+1);
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"}\n");
        }
        kek_compiler_SelfCExprDestroy(&arrayExpr);
    }
    kek_compiler_SelfCTypeInfoDestroy(&valueType);
    outNext[0]=blockEnd+1;
    return (status);
}
Status kek_compiler_SelfCWriteSwitchStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    usize groupStart=kek_compiler_SelfCFindNextGroup(program,fileIndex,start,kek_compiler_SelfCFileTokenCount(program,fileIndex));
    usize groupEnd=kek_compiler_SelfCFindMatching(program,fileIndex,groupStart);
    usize blockStart=groupEnd+1;
    usize blockEnd=kek_compiler_SelfCFindMatching(program,fileIndex,blockStart);
    struct SelfCExpr value={0};
    Status status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&value);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteIndent(out,indent);
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
    kek_compiler_SelfCExprDestroy(&value);
    usize i=blockStart+1;
    while (i<blockEnd&&status==Status_Ok) {
        if (kek_compiler_SelfCIsKeyword(program,fileIndex,i,KeywordKind_Case)) {
            usize caseGroup=i+1;
            usize caseGroupEnd=kek_compiler_SelfCFindMatching(program,fileIndex,caseGroup);
            struct SelfCExpr caseExpr={0};
            status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,caseGroup+1,caseGroupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&caseExpr);
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteIndent(out,indent+1);
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
            kek_compiler_SelfCExprDestroy(&caseExpr);
            usize caseBlock=caseGroupEnd+1;
            if (caseBlock<blockEnd&&kek_compiler_SelfCIsPunctuation(program,fileIndex,caseBlock,PunctuationKind_Colon)) {
                caseBlock+=1;
            }
            if (caseBlock<blockEnd&&kek_compiler_SelfCIsPunctuation(program,fileIndex,caseBlock,PunctuationKind_LeftBrace)) {
                usize caseBlockEnd=kek_compiler_SelfCFindMatching(program,fileIndex,caseBlock);
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"{\n");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteBlock(program,out,env,fileIndex,caseBlock+1,caseBlockEnd,indent+2);
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteIndent(out,indent+1);
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"}\n");
                }
                i=caseBlockEnd+1;
            } else {
                usize semicolon=kek_compiler_SelfCFindStatementSemicolon(program,fileIndex,caseBlock,blockEnd);
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"{\n");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteExprStatement(program,out,env,fileIndex,caseBlock,semicolon,indent+2);
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteIndent(out,indent+1);
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"}\n");
                }
                i=semicolon+1;
            }
            continue;
        }
        if (kek_compiler_SelfCIsKeyword(program,fileIndex,i,KeywordKind_Default)) {
            status=kek_compiler_SelfCWriteIndent(out,indent+1);
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"default: ");
            }
            usize bodyStart=i+1;
            if (bodyStart<blockEnd&&kek_compiler_SelfCIsPunctuation(program,fileIndex,bodyStart,PunctuationKind_Colon)) {
                bodyStart+=1;
            }
            if (bodyStart<blockEnd&&kek_compiler_SelfCIsPunctuation(program,fileIndex,bodyStart,PunctuationKind_LeftBrace)) {
                usize bodyEnd=kek_compiler_SelfCFindMatching(program,fileIndex,bodyStart);
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"{\n");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteBlock(program,out,env,fileIndex,bodyStart+1,bodyEnd,indent+2);
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteIndent(out,indent+1);
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"}\n");
                }
                i=bodyEnd+1;
            } else {
                usize semicolon=kek_compiler_SelfCFindStatementSemicolon(program,fileIndex,bodyStart,blockEnd);
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"{\n");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteExprStatement(program,out,env,fileIndex,bodyStart,semicolon,indent+2);
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteIndent(out,indent+1);
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
        status=kek_compiler_SelfCWriteIndent(out,indent);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"}\n");
    }
    outNext[0]=blockEnd+1;
    return (status);
}
Status kek_compiler_SelfCWriteReturnStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize semicolon,usize indent) {
    Status status=kek_compiler_SelfCWriteIndent(out,indent);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"return");
    }
    if (start+1<semicolon) {
        struct SelfCExpr value={0};
        if (status==Status_Ok) {
            status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,start+1,semicolon,std_string_OwnedStringView(&env->returnTypeKey),std_string_OwnedStringView(&env->returnCType),0,&value);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,' ');
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&value.text));
        }
        if (status==Status_Ok) {
            kek_compiler_SelfCExprDestroy(&value);
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    return (status);
}
Status kek_compiler_SelfCWriteSingleStatement(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize* outNext,usize indent) {
    if (kek_compiler_SelfCIsPunctuation(program,fileIndex,start,PunctuationKind_LeftBrace)) {
        usize blockEnd=kek_compiler_SelfCFindMatching(program,fileIndex,start);
        Status status=kek_compiler_SelfCWriteIndent(out,indent);
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"{\n");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteBlock(program,out,env,fileIndex,start+1,blockEnd,indent+1);
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteIndent(out,indent);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"}\n");
        }
        outNext[0]=blockEnd+1;
        return (status);
    }
    if (kek_compiler_SelfCIsKeyword(program,fileIndex,start,KeywordKind_If)) {
        return (kek_compiler_SelfCWriteIfStatement(program,out,env,fileIndex,start,outNext,indent));
    }
    if (kek_compiler_SelfCIsKeyword(program,fileIndex,start,KeywordKind_While)) {
        return (kek_compiler_SelfCWriteWhileStatement(program,out,env,fileIndex,start,outNext,indent));
    }
    if (kek_compiler_SelfCIsKeyword(program,fileIndex,start,KeywordKind_Do)) {
        return (kek_compiler_SelfCWriteDoStatement(program,out,env,fileIndex,start,outNext,indent));
    }
    if (kek_compiler_SelfCIsKeyword(program,fileIndex,start,KeywordKind_For)) {
        return (kek_compiler_SelfCWriteForStatement(program,out,env,fileIndex,start,outNext,indent));
    }
    if (kek_compiler_SelfCIsKeyword(program,fileIndex,start,KeywordKind_Each)) {
        return (kek_compiler_SelfCWriteEachStatement(program,out,env,fileIndex,start,outNext,indent));
    }
    if (kek_compiler_SelfCIsKeyword(program,fileIndex,start,KeywordKind_Switch)) {
        return (kek_compiler_SelfCWriteSwitchStatement(program,out,env,fileIndex,start,outNext,indent));
    }
    usize semicolon=kek_compiler_SelfCFindStatementSemicolon(program,fileIndex,start,kek_compiler_SelfCFileTokenCount(program,fileIndex));
    if (kek_compiler_SelfCIsKeyword(program,fileIndex,start,KeywordKind_Return)) {
        outNext[0]=semicolon+1;
        return (kek_compiler_SelfCWriteReturnStatement(program,out,env,fileIndex,start,semicolon,indent));
    }
    if (kek_compiler_SelfCIsKeyword(program,fileIndex,start,KeywordKind_Break)) {
        outNext[0]=semicolon+1;
        Status status=kek_compiler_SelfCWriteIndent(out,indent);
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"break;\n");
        }
        return (status);
    }
    if (kek_compiler_SelfCIsKeyword(program,fileIndex,start,KeywordKind_Continue)) {
        outNext[0]=semicolon+1;
        Status status=kek_compiler_SelfCWriteIndent(out,indent);
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"continue;\n");
        }
        return (status);
    }
    if (kek_compiler_SelfCIsKeyword(program,fileIndex,start,KeywordKind_Unreachable)) {
        outNext[0]=semicolon+1;
        Status status=kek_compiler_SelfCWriteIndent(out,indent);
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"__builtin_unreachable();\n");
        }
        return (status);
    }
    if (kek_compiler_SelfCIsKeyword(program,fileIndex,start,KeywordKind_Panic)) {
        outNext[0]=semicolon+1;
        usize groupStart=start+1;
        usize groupEnd=kek_compiler_SelfCFindMatching(program,fileIndex,groupStart);
        struct SelfCExpr message={0};
        Status status=kek_compiler_SelfCCompileExpressionRange(program,env,fileIndex,groupStart+1,groupEnd,std_string_StringFromCString(""),std_string_StringFromCString(""),0,&message);
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteIndent(out,indent);
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
        kek_compiler_SelfCExprDestroy(&message);
        return (status);
    }
    if (kek_compiler_SelfCIsKeyword(program,fileIndex,start,KeywordKind_Defer)) {
        outNext[0]=semicolon+1;
        return (Status_Ok);
    }
    usize colon=kek_compiler_SelfCFindTopLevelColon(program,fileIndex,start,semicolon);
    if (colon<semicolon) {
        outNext[0]=semicolon+1;
        return (kek_compiler_SelfCWriteVarDecl(program,out,env,fileIndex,start,semicolon,indent));
    }
    outNext[0]=semicolon+1;
    return (kek_compiler_SelfCWriteExprStatement(program,out,env,fileIndex,start,semicolon,indent));
}
Status kek_compiler_SelfCWriteDeferred(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize end,usize indent) {
    if (start>=end||!kek_compiler_SelfCIsKeyword(program,fileIndex,start,KeywordKind_Defer)) {
        return (Status_Ok);
    }
    usize body=start+1;
    if (body<end&&kek_compiler_SelfCIsPunctuation(program,fileIndex,body,PunctuationKind_LeftBrace)) {
        usize blockEnd=kek_compiler_SelfCFindMatching(program,fileIndex,body);
        return (kek_compiler_SelfCWriteBlock(program,out,env,fileIndex,body+1,blockEnd,indent));
    }
    usize semicolon=kek_compiler_SelfCFindStatementSemicolon(program,fileIndex,body,end);
    return (kek_compiler_SelfCWriteExprStatement(program,out,env,fileIndex,body,semicolon,indent));
}
Status kek_compiler_SelfCWriteBlock(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCEnv* env,usize fileIndex,usize start,usize end,usize indent) {
    usize deferStarts[64]={0};
    usize deferEnds[64]={0};
    usize deferCount=0;
    usize index=start;
    Status status=Status_Ok;
    while (index<end&&status==Status_Ok) {
        if (kek_compiler_SelfCIsKeyword(program,fileIndex,index,KeywordKind_Defer)) {
            usize deferEnd=index+1;
            if (deferEnd<end&&kek_compiler_SelfCIsPunctuation(program,fileIndex,deferEnd,PunctuationKind_LeftBrace)) {
                deferEnd=kek_compiler_SelfCFindMatching(program,fileIndex,deferEnd)+1;
            } else {
                deferEnd=kek_compiler_SelfCFindStatementSemicolon(program,fileIndex,deferEnd,end)+1;
            }
            if (deferCount<((void)(deferStarts),64)) {
                deferStarts[deferCount]=index;
                deferEnds[deferCount]=deferEnd;
                deferCount+=1;
            }
            index=deferEnd;
            continue;
        }
        usize next=index+1;
        status=kek_compiler_SelfCWriteSingleStatement(program,out,env,fileIndex,index,&next,indent);
        if (next<=index) {
            next=index+1;
        }
        index=next;
    }
    while (deferCount>0&&status==Status_Ok) {
        deferCount-=1;
        status=kek_compiler_SelfCWriteDeferred(program,out,env,fileIndex,deferStarts[deferCount],deferEnds[deferCount],indent);
    }
    return (status);
}
Status kek_compiler_SelfCFuncUseTempTypeUse(struct SelfCFuncUse* funcUse,struct SelfCTypeUse* out) {
    out->key=funcUse->key;
    out->cName=funcUse->cName;
    out->baseName=funcUse->key;
    out->arg0=funcUse->arg0;
    out->arg1=funcUse->arg1;
    out->arg2=funcUse->arg2;
    out->argCount=funcUse->argCount;
    out->emitted=0;
    return (Status_Ok);
}
Status kek_compiler_SelfCWriteSubstTypeForFunc(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCFuncUse* funcUse,usize fileIndex,usize start,usize end) {
    struct SelfCTypeUse temp={0};
    kek_compiler_SelfCFuncUseTempTypeUse(funcUse,&temp);
    struct String firstTypeToken=kek_compiler_SelfCTokenText(program,fileIndex,start);
    if (start<end&&String_EqualsCString(&firstTypeToken,"ptr")&&start+1<end&&kek_compiler_SelfCIsOperator(program,fileIndex,start+1,OperatorKind_Less)) {
        usize genericEnd=kek_compiler_SelfCFindMatching(program,fileIndex,start+1);
        struct OwnedString innerKey={0};
        Status innerStatus=kek_compiler_SelfCMakeSubstTypeKey(program,decl,&temp,fileIndex,start+2,genericEnd,&innerKey);
        if (innerStatus==Status_Ok) {
            innerStatus=kek_compiler_SelfCWriteCTypeFromKey(program,out,std_string_OwnedStringView(&innerKey));
        }
        if (innerStatus==Status_Ok) {
            innerStatus=StringBuilder_WriteByte(out,'*');
        }
        std_string_DestroyOwnedString(&innerKey);
        return (innerStatus);
    }
    struct OwnedString key={0};
    Status status=kek_compiler_SelfCMakeSubstTypeKey(program,decl,&temp,fileIndex,start,end,&key);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteCTypeFromKey(program,out,std_string_OwnedStringView(&key));
    }
    if (status==Status_Ok) {
        std_string_DestroyOwnedString(&key);
    }
    return (status);
}
Status kek_compiler_SelfCWriteFunctionSignature(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct String nameOverride,struct SelfCFuncUse* funcUse) {
    struct String functionName=kek_compiler_SelfCTokenText(program,decl->fileIndex,decl->nameIndex);
    bool isMain=!decl->hasReceiver&&String_EqualsCString(&functionName,"main")&&kek_compiler_SelfCDeclPackageIsRoot(decl);
    Status status=Status_Ok;
    if (isMain) {
        status=StringBuilder_WriteCString(out,"int");
    } else {
        if (funcUse!=0) {
            status=kek_compiler_SelfCWriteSubstTypeForFunc(program,out,decl,funcUse,decl->fileIndex,decl->returnStart,decl->returnEnd);
        } else {
            struct SelfCTypeInfo returnInfo={0};
            status=kek_compiler_SelfCRenderTypeInfo(program,decl->fileIndex,decl->returnStart,decl->returnEnd,&returnInfo);
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&returnInfo.cType));
            }
            if (status==Status_Ok) {
                kek_compiler_SelfCTypeInfoDestroy(&returnInfo);
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
            status=kek_compiler_SelfCWriteDeclCName(program,out,decl);
        }
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'(');
    }
    bool first=1;
    if (decl->hasReceiver) {
        if (funcUse!=0) {
            status=kek_compiler_SelfCWriteSubstTypeForFunc(program,out,decl,funcUse,decl->fileIndex,decl->receiverStart,decl->receiverEnd);
        } else {
            struct SelfCTypeInfo receiverInfo={0};
            if (status==Status_Ok) {
                status=kek_compiler_SelfCRenderTypeInfo(program,decl->fileIndex,decl->receiverStart,decl->receiverEnd,&receiverInfo);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&receiverInfo.cType));
            }
            if (status==Status_Ok) {
                kek_compiler_SelfCTypeInfoDestroy(&receiverInfo);
            }
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"* this");
        }
        first=0;
    }
    for (usize i=0;i<decl->paramCount;i++) {
        struct SelfCParam* param=&program->params[decl->firstParam+i];
        if (!first&&status==Status_Ok) {
            status=StringBuilder_WriteByte(out,',');
        }
        if (funcUse!=0) {
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteSubstTypeForFunc(program,out,decl,funcUse,param->fileIndex,param->typeStart,param->typeEnd);
            }
        } else {
            struct SelfCTypeInfo paramInfo={0};
            if (status==Status_Ok) {
                status=kek_compiler_SelfCRenderTypeInfo(program,param->fileIndex,param->typeStart,param->typeEnd,&paramInfo);
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&paramInfo.cType));
            }
            if (status==Status_Ok) {
                kek_compiler_SelfCTypeInfoDestroy(&paramInfo);
            }
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteByte(out,' ');
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteToken(program,out,param->fileIndex,param->nameIndex);
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
Status kek_compiler_SelfCWriteFunctionPrototype(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl) {
    Status status=kek_compiler_SelfCWriteFunctionSignature(program,out,decl,std_string_StringFromCString(""),0);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,";\n");
    }
    return (status);
}
Status kek_compiler_SelfCWriteFunctionBody(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl) {
    Status status=kek_compiler_SelfCWriteFunctionSignature(program,out,decl,std_string_StringFromCString(""),0);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," {\n");
    }
    struct SelfCEnv env={0};
    if (status==Status_Ok) {
        status=kek_compiler_SelfCEnvInit(program,&env);
    }
    struct SelfCTypeInfo returnInfo={0};
    if (status==Status_Ok) {
        status=kek_compiler_SelfCRenderTypeInfo(program,decl->fileIndex,decl->returnStart,decl->returnEnd,&returnInfo);
    }
    if (status==Status_Ok) {
        kek_compiler_SelfCReplaceOwned(&env.returnTypeKey,returnInfo.key);
        kek_compiler_SelfCReplaceOwned(&env.returnCType,returnInfo.cType);
        returnInfo.key.data=0;
        returnInfo.cType.data=0;
    }
    if (decl->hasReceiver&&status==Status_Ok) {
        struct SelfCTypeInfo receiverInfo={0};
        status=kek_compiler_SelfCRenderTypeInfo(program,decl->fileIndex,decl->receiverStart,decl->receiverEnd,&receiverInfo);
        if (status==Status_Ok) {
            env.hasThis=1;
            kek_compiler_SelfCReplaceOwned(&env.thisTypeKey,receiverInfo.key);
            kek_compiler_SelfCReplaceOwned(&env.thisCType,receiverInfo.cType);
            receiverInfo.key.data=0;
            receiverInfo.cType.data=0;
            status=kek_compiler_SelfCEnvAdd(program,&env,std_string_StringFromCString("this"),std_string_OwnedStringView(&env.thisTypeKey),std_string_OwnedStringView(&env.thisCType),0,std_string_StringFromCString(""),1);
        }
        kek_compiler_SelfCTypeInfoDestroy(&receiverInfo);
    }
    for (usize i=0;i<decl->paramCount&&status==Status_Ok;i++) {
        struct SelfCParam* param=&program->params[decl->firstParam+i];
        struct SelfCTypeInfo paramInfo={0};
        status=kek_compiler_SelfCRenderTypeInfo(program,param->fileIndex,param->typeStart,param->typeEnd,&paramInfo);
        if (status==Status_Ok) {
            status=kek_compiler_SelfCEnvAdd(program,&env,kek_compiler_SelfCTokenText(program,param->fileIndex,param->nameIndex),std_string_OwnedStringView(&paramInfo.key),std_string_OwnedStringView(&paramInfo.cType),0,std_string_StringFromCString(""),paramInfo.isPointer);
        }
        kek_compiler_SelfCTypeInfoDestroy(&paramInfo);
    }
    kek_compiler_SelfCTypeInfoDestroy(&returnInfo);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteBlock(program,out,&env,decl->fileIndex,decl->bodyStart,decl->bodyEnd,1);
    }
    kek_compiler_SelfCEnvDestroy(&env);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,"}\n");
    }
    return (status);
}
Status kek_compiler_SelfCWriteManualLine(struct StringBuilder* out,str text) {
    Status status=StringBuilder_WriteCString(out,text);
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'\n');
    }
    return (status);
}
Status kek_compiler_SelfCWriteArrayMethodRef(struct StringBuilder* out,struct SelfCFuncUse* use,str name) {
    Status status=StringBuilder_WriteCString(out,"Array__");
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg0));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'_');
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,name);
    }
    return (status);
}
Status kek_compiler_SelfCWriteLinkedListMethodRef(struct StringBuilder* out,struct SelfCFuncUse* use,str name) {
    Status status=StringBuilder_WriteCString(out,"LinkedList__");
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg0));
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'_');
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out,name);
    }
    return (status);
}
Status kek_compiler_SelfCWriteResultTypeRef(struct StringBuilder* out,struct SelfCFuncUse* use) {
    Status status=StringBuilder_WriteCString(out,"struct Result__");
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg0));
    }
    return (status);
}
Status kek_compiler_SelfCWriteListNodeTypeRef(struct StringBuilder* out,struct SelfCFuncUse* use) {
    Status status=StringBuilder_WriteCString(out,"struct ListNode__");
    if (status==Status_Ok) {
        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg0));
    }
    return (status);
}
Status kek_compiler_SelfCWriteManualArrayGenericBody(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCFuncUse* use,struct String argC) {
    struct String name=kek_compiler_SelfCTokenText(program,decl->fileIndex,decl->nameIndex);
    Status status=Status_Ok;
    if (String_EqualsCString(&name,"Destroy")) {
        status=kek_compiler_SelfCWriteManualLine(out,"    if(this->data!=0){");
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteManualLine(out,"        kek_std_free(this->data);");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteManualLine(out,"    }");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteManualLine(out,"    this->data=0;");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteManualLine(out,"    this->len=0;");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteManualLine(out,"    this->cap=0;");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteManualLine(out,"    return Status_Ok;");
        }
    } else {
        if (String_EqualsCString(&name,"Clear")) {
            status=kek_compiler_SelfCWriteManualLine(out,"    this->len=0;");
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    return Status_Ok;");
            }
        } else {
            if (String_EqualsCString(&name,"Reserve")) {
                status=kek_compiler_SelfCWriteManualLine(out,"    usize needed=this->len+additional;");
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    if(needed<=this->cap){return Status_Ok;}");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    usize newCap=this->cap;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    if(newCap==0){newCap=8;}");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    while(newCap<needed){newCap=newCap*2;}");
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
                    status=kek_compiler_SelfCWriteManualLine(out,"));");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    if(newData==0){return Status_NoMemory;}");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    this->data=newData;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    this->cap=newCap;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    return Status_Ok;");
                }
            } else {
                if (String_EqualsCString(&name,"AppendSlice")) {
                    status=StringBuilder_WriteCString(out,"    Status status=");
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCWriteArrayMethodRef(out,use,"Reserve");
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCWriteManualLine(out,"(this,items.len);");
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCWriteManualLine(out,"    if(status!=Status_Ok){return status;}");
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCWriteManualLine(out,"    for(usize i=0;i<items.len;i++){");
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCWriteManualLine(out,"        this->data[this->len+i]=items.data[i];");
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCWriteManualLine(out,"    }");
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCWriteManualLine(out,"    this->len+=items.len;");
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCWriteManualLine(out,"    return Status_Ok;");
                    }
                } else {
                    if (String_EqualsCString(&name,"Push")) {
                        status=StringBuilder_WriteCString(out,"    Status status=");
                        if (status==Status_Ok) {
                            status=kek_compiler_SelfCWriteArrayMethodRef(out,use,"Reserve");
                        }
                        if (status==Status_Ok) {
                            status=kek_compiler_SelfCWriteManualLine(out,"(this,1);");
                        }
                        if (status==Status_Ok) {
                            status=kek_compiler_SelfCWriteManualLine(out,"    if(status!=Status_Ok){return status;}");
                        }
                        if (status==Status_Ok) {
                            status=kek_compiler_SelfCWriteManualLine(out,"    this->data[this->len]=value;");
                        }
                        if (status==Status_Ok) {
                            status=kek_compiler_SelfCWriteManualLine(out,"    this->len+=1;");
                        }
                        if (status==Status_Ok) {
                            status=kek_compiler_SelfCWriteManualLine(out,"    return Status_Ok;");
                        }
                    } else {
                        if (String_EqualsCString(&name,"PushZeroed")) {
                            status=StringBuilder_WriteCString(out,"    Status status=");
                            if (status==Status_Ok) {
                                status=kek_compiler_SelfCWriteArrayMethodRef(out,use,"Reserve");
                            }
                            if (status==Status_Ok) {
                                status=kek_compiler_SelfCWriteManualLine(out,"(this,1);");
                            }
                            if (status==Status_Ok) {
                                status=kek_compiler_SelfCWriteManualLine(out,"    if(status!=Status_Ok){return status;}");
                            }
                            if (status==Status_Ok) {
                                status=StringBuilder_WriteCString(out,"    kek_std_mem_set((void*)(&this->data[this->len]),0,sizeof(");
                            }
                            if (status==Status_Ok) {
                                status=StringBuilder_WriteString(out,argC);
                            }
                            if (status==Status_Ok) {
                                status=kek_compiler_SelfCWriteManualLine(out,"));");
                            }
                            if (status==Status_Ok) {
                                status=kek_compiler_SelfCWriteManualLine(out,"    this->len+=1;");
                            }
                            if (status==Status_Ok) {
                                status=kek_compiler_SelfCWriteManualLine(out,"    return Status_Ok;");
                            }
                        } else {
                            if (String_EqualsCString(&name,"Pop")) {
                                status=StringBuilder_WriteCString(out,"    ");
                                if (status==Status_Ok) {
                                    status=kek_compiler_SelfCWriteResultTypeRef(out,use);
                                }
                                if (status==Status_Ok) {
                                    status=kek_compiler_SelfCWriteManualLine(out," result={0};");
                                }
                                if (status==Status_Ok) {
                                    status=kek_compiler_SelfCWriteManualLine(out,"    if(this->len==0){result.status=Status_End;return result;}");
                                }
                                if (status==Status_Ok) {
                                    status=kek_compiler_SelfCWriteManualLine(out,"    this->len-=1;");
                                }
                                if (status==Status_Ok) {
                                    status=kek_compiler_SelfCWriteManualLine(out,"    result.status=Status_Ok;");
                                }
                                if (status==Status_Ok) {
                                    status=kek_compiler_SelfCWriteManualLine(out,"    result.value=this->data[this->len];");
                                }
                                if (status==Status_Ok) {
                                    status=kek_compiler_SelfCWriteManualLine(out,"    return result;");
                                }
                            } else {
                                if (String_EqualsCString(&name,"Get")) {
                                    status=StringBuilder_WriteCString(out,"    ");
                                    if (status==Status_Ok) {
                                        status=kek_compiler_SelfCWriteResultTypeRef(out,use);
                                    }
                                    if (status==Status_Ok) {
                                        status=kek_compiler_SelfCWriteManualLine(out," result={0};");
                                    }
                                    if (status==Status_Ok) {
                                        status=kek_compiler_SelfCWriteManualLine(out,"    if(index>=this->len){result.status=Status_Invalid;return result;}");
                                    }
                                    if (status==Status_Ok) {
                                        status=kek_compiler_SelfCWriteManualLine(out,"    result.status=Status_Ok;");
                                    }
                                    if (status==Status_Ok) {
                                        status=kek_compiler_SelfCWriteManualLine(out,"    result.value=this->data[index];");
                                    }
                                    if (status==Status_Ok) {
                                        status=kek_compiler_SelfCWriteManualLine(out,"    return result;");
                                    }
                                } else {
                                    if (String_EqualsCString(&name,"Set")) {
                                        status=kek_compiler_SelfCWriteManualLine(out,"    if(index>=this->len){return Status_Invalid;}");
                                        if (status==Status_Ok) {
                                            status=kek_compiler_SelfCWriteManualLine(out,"    this->data[index]=value;");
                                        }
                                        if (status==Status_Ok) {
                                            status=kek_compiler_SelfCWriteManualLine(out,"    return Status_Ok;");
                                        }
                                    } else {
                                        if (String_EqualsCString(&name,"GetPtr")) {
                                            status=kek_compiler_SelfCWriteManualLine(out,"    if(index>=this->len){return 0;}");
                                            if (status==Status_Ok) {
                                                status=kek_compiler_SelfCWriteManualLine(out,"    return &this->data[index];");
                                            }
                                        } else {
                                            if (String_EqualsCString(&name,"LastPtr")) {
                                                status=kek_compiler_SelfCWriteManualLine(out,"    if(this->len==0){return 0;}");
                                                if (status==Status_Ok) {
                                                    status=kek_compiler_SelfCWriteManualLine(out,"    return &this->data[this->len-1];");
                                                }
                                            } else {
                                                if (String_EqualsCString(&name,"Span")) {
                                                    status=StringBuilder_WriteCString(out,"    struct Span__");
                                                    if (status==Status_Ok) {
                                                        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg0));
                                                    }
                                                    if (status==Status_Ok) {
                                                        status=kek_compiler_SelfCWriteManualLine(out," out={0};");
                                                    }
                                                    if (status==Status_Ok) {
                                                        status=kek_compiler_SelfCWriteManualLine(out,"    out.data=this->data;");
                                                    }
                                                    if (status==Status_Ok) {
                                                        status=kek_compiler_SelfCWriteManualLine(out,"    out.len=this->len;");
                                                    }
                                                    if (status==Status_Ok) {
                                                        status=kek_compiler_SelfCWriteManualLine(out,"    return out;");
                                                    }
                                                } else {
                                                    if (String_EqualsCString(&name,"Slice")) {
                                                        status=StringBuilder_WriteCString(out,"    struct Slice__");
                                                        if (status==Status_Ok) {
                                                            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg0));
                                                        }
                                                        if (status==Status_Ok) {
                                                            status=kek_compiler_SelfCWriteManualLine(out," out={0};");
                                                        }
                                                        if (status==Status_Ok) {
                                                            status=kek_compiler_SelfCWriteManualLine(out,"    out.data=this->data;");
                                                        }
                                                        if (status==Status_Ok) {
                                                            status=kek_compiler_SelfCWriteManualLine(out,"    out.len=this->len;");
                                                        }
                                                        if (status==Status_Ok) {
                                                            status=kek_compiler_SelfCWriteManualLine(out,"    return out;");
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
Status kek_compiler_SelfCWriteManualLinkedListGenericBody(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCFuncUse* use) {
    struct String name=kek_compiler_SelfCTokenText(program,decl->fileIndex,decl->nameIndex);
    Status status=Status_Ok;
    if (String_EqualsCString(&name,"PushBack")||String_EqualsCString(&name,"PushFront")) {
        status=StringBuilder_WriteCString(out,"    ");
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteListNodeTypeRef(out,use);
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"* node=(");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteListNodeTypeRef(out,use);
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteManualLine(out,"*)kek_std_alloc(");
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"        sizeof(");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteListNodeTypeRef(out,use);
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteManualLine(out,"));");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteManualLine(out,"    if(node==0){return Status_NoMemory;}");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteManualLine(out,"    node->value=value;");
        }
        if (String_EqualsCString(&name,"PushBack")) {
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    node->next=0;");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    node->prev=this->last;");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    if(this->last!=0){");
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"        ");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteListNodeTypeRef(out,use);
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"* last=this->last;");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"        last->next=node;");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    }else{");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"        this->first=node;");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    }");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    this->last=node;");
            }
        } else {
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    node->prev=0;");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    node->next=this->first;");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    if(this->first!=0){");
            }
            if (status==Status_Ok) {
                status=StringBuilder_WriteCString(out,"        ");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteListNodeTypeRef(out,use);
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"* first=this->first;");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"        first->prev=node;");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    }else{");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"        this->last=node;");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    }");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    this->first=node;");
            }
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteManualLine(out,"    this->len+=1;");
        }
        if (status==Status_Ok) {
            status=kek_compiler_SelfCWriteManualLine(out,"    return Status_Ok;");
        }
    } else {
        if (String_EqualsCString(&name,"PopBack")||String_EqualsCString(&name,"PopFront")) {
            status=StringBuilder_WriteCString(out,"    ");
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteResultTypeRef(out,use);
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out," result={0};");
            }
            if (String_EqualsCString(&name,"PopBack")) {
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    if(this->last==0){result.status=Status_End;return result;}");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"    ");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteListNodeTypeRef(out,use);
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"* node=this->last;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    result.value=node->value;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    this->last=node->prev;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    if(this->last!=0){");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"        ");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteListNodeTypeRef(out,use);
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"* last=this->last;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"        last->next=0;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    }else{");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"        this->first=0;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    }");
                }
            } else {
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    if(this->first==0){result.status=Status_End;return result;}");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"    ");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteListNodeTypeRef(out,use);
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"* node=this->first;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    result.value=node->value;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    this->first=node->next;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    if(this->first!=0){");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"        ");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteListNodeTypeRef(out,use);
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"* first=this->first;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"        first->prev=0;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    }else{");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"        this->last=0;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    }");
                }
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    this->len-=1;");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    kek_std_free(node);");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    result.status=Status_Ok;");
            }
            if (status==Status_Ok) {
                status=kek_compiler_SelfCWriteManualLine(out,"    return result;");
            }
        } else {
            if (String_EqualsCString(&name,"Destroy")) {
                status=kek_compiler_SelfCWriteManualLine(out,"    while(this->len>0){");
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"        ");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteResultTypeRef(out,use);
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out," ignored={0};");
                }
                if (status==Status_Ok) {
                    status=StringBuilder_WriteCString(out,"        ignored=");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteLinkedListMethodRef(out,use,"PopBack");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"(this);");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"        if(ignored.status!=Status_Ok){return ignored.status;}");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    }");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    return Status_Ok;");
                }
            }
        }
    }
    return (status);
}
Status kek_compiler_SelfCWriteManualGenericBody(struct SelfCProgram* program,struct StringBuilder* out,struct SelfCDecl* decl,struct SelfCFuncUse* use) {
    struct String name=kek_compiler_SelfCTokenText(program,decl->fileIndex,decl->nameIndex);
    struct OwnedString argC={0};
    Status status=kek_compiler_SelfCMakeCTypeFromKey(program,std_string_OwnedStringView(&use->arg0),&argC);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_compiler_SelfCWriteFunctionSignature(program,out,decl,std_string_OwnedStringView(&use->cName),use);
    if (status==Status_Ok) {
        status=StringBuilder_WriteCString(out," {\n");
    }
    bool handled=0;
    if (decl->hasReceiver&&status==Status_Ok) {
        struct String receiverBase=kek_compiler_SelfCTokenText(program,decl->fileIndex,decl->receiverStart);
        if (String_EqualsCString(&receiverBase,"Array")) {
            status=kek_compiler_SelfCWriteManualArrayGenericBody(program,out,decl,use,std_string_OwnedStringView(&argC));
            handled=1;
        } else {
            if (String_EqualsCString(&receiverBase,"LinkedList")) {
                status=kek_compiler_SelfCWriteManualLinkedListGenericBody(program,out,decl,use);
                handled=1;
            }
        }
    }
    if (!handled&&String_EqualsCString(&name,"FixedSlice")) {
        if (status==Status_Ok) {
            status=StringBuilder_WriteCString(out,"    struct Slice__");
        }
        if (status==Status_Ok) {
            status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg0));
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
                status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg0));
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
                    status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg0));
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out," array={0};");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    array.allocator=allocator;");
                }
                if (status==Status_Ok) {
                    status=kek_compiler_SelfCWriteManualLine(out,"    return array;");
                }
            } else {
                if (String_EqualsCString(&name,"LinkedListNew")) {
                    if (status==Status_Ok) {
                        status=StringBuilder_WriteCString(out,"    struct LinkedList__");
                    }
                    if (status==Status_Ok) {
                        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg0));
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCWriteManualLine(out," list={0};");
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCWriteManualLine(out,"    list.allocator=allocator;");
                    }
                    if (status==Status_Ok) {
                        status=kek_compiler_SelfCWriteManualLine(out,"    return list;");
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
                                                    status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg0));
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
                                                        status=StringBuilder_WriteString(out,std_string_OwnedStringView(&use->arg0));
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
Status kek_compiler_SelfCWriteFunctionDeclarations(struct SelfCProgram* program,struct StringBuilder* out) {
    for (usize i=0;i<program->declCount;i++) {
        struct SelfCDecl* decl=&program->decls[i];
        if (decl->kind==SelfCDeclKind_Function&&!decl->isGeneric) {
            Status status=kek_compiler_SelfCWriteFunctionPrototype(program,out,decl);
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    for (usize i=0;i<program->funcUseCount;i++) {
        struct SelfCFuncUse* use=&program->funcUses[i];
        if (use->declIndex<program->declCount) {
            Status status=kek_compiler_SelfCWriteFunctionSignature(program,out,&program->decls[use->declIndex],std_string_OwnedStringView(&use->cName),use);
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
Status kek_compiler_SelfCWriteFunctionDefinitions(struct SelfCProgram* program,struct StringBuilder* out) {
    for (usize i=0;i<program->funcUseCount;i++) {
        struct SelfCFuncUse* use=&program->funcUses[i];
        if (use->declIndex<program->declCount) {
            Status status=kek_compiler_SelfCWriteManualGenericBody(program,out,&program->decls[use->declIndex],use);
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    for (usize i=0;i<program->declCount;i++) {
        struct SelfCDecl* decl=&program->decls[i];
        if (decl->kind==SelfCDeclKind_Function&&!decl->isGeneric&&decl->hasBody) {
            Status status=kek_compiler_SelfCWriteFunctionBody(program,out,decl);
            if (status!=Status_Ok) {
                return (status);
            }
        }
    }
    return (Status_Ok);
}
Status kek_compiler_SelfCWriteProgram(struct SelfCProgram* program,struct StringBuilder* out) {
    Status status=kek_compiler_SelfCWritePrelude(out);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteTypeDeclarations(program,out);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'\n');
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteFunctionDeclarations(program,out);
    }
    if (status==Status_Ok) {
        status=StringBuilder_WriteByte(out,'\n');
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteFunctionDefinitions(program,out);
    }
    return (status);
}
Status kek_compiler_SelfCompileToCString(str inputPath,struct Allocator allocator,struct StringBuilder* out,struct SelfDiagnosticBag* diagnostics) {
    struct SelfCProgram program={0};
    Status status=kek_compiler_SelfCProgramInit(&program,allocator);
    if (status!=Status_Ok) {
        return (status);
    }
    status=kek_compiler_SelfCLoadAndTokenize(&program,inputPath);
    if (status==Status_Ok) {
        status=kek_compiler_SelfCParseDeclarations(&program);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCollectTypeUses(&program);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCCollectGenericFunctionUses(&program);
    }
    if (status==Status_Ok) {
        status=kek_compiler_SelfCWriteProgram(&program,out);
    }
    if (program.diagnostics.count>0) {
        for (usize i=0;i<program.diagnostics.count&&diagnostics->count<(sizeof(diagnostics->items)/sizeof((diagnostics->items)[0]));i++) {
            struct SelfDiagnostic* item=&program.diagnostics.items[i];
            kek_diagnostics_SelfDiagnosticAdd(diagnostics,item->severity,item->phase,item->fileIndex,item->location,std_string_OwnedStringView(&item->message));
        }
    }
    kek_compiler_SelfCProgramDestroy(&program);
    return (status);
}
Status kek_compiler_SelfCompileToC(str inputPath,str outputPath,struct Allocator allocator,struct SelfDiagnosticBag* diagnostics) {
    struct StringBuilder generated=std_string_StringBuilderNew(allocator);
    Status status=kek_compiler_SelfCompileToCString(inputPath,allocator,&generated,diagnostics);
    if (status==Status_Ok) {
        struct String view=StringBuilder_View(&generated);
        status=std_file_WriteFile(outputPath,view);
    }
    StringBuilder_Destroy(&generated);
    return (status);
}
Status kek_compiler_SelfCompileToCStringWithoutDiagnostics(str inputPath,struct Allocator allocator,struct StringBuilder* out) {
    struct SelfDiagnosticBag diagnostics={0};
    kek_diagnostics_SelfDiagnosticBagInit(&diagnostics,allocator);
    Status status=kek_compiler_SelfCompileToCString(inputPath,allocator,out,&diagnostics);
    kek_diagnostics_SelfDiagnosticBagDestroy(&diagnostics);
    return (status);
}
Status kek_compiler_SelfWriteCompileDiagnostics(str inputPath,struct Allocator allocator,struct StringBuilder* out) {
    struct StringBuilder generated=std_string_StringBuilderNew(allocator);
    struct SelfDiagnosticBag diagnostics={0};
    kek_diagnostics_SelfDiagnosticBagInit(&diagnostics,allocator);
    Status status=kek_compiler_SelfCompileToCString(inputPath,allocator,&generated,&diagnostics);
    Status writeStatus=Status_Ok;
    if (diagnostics.count>0) {
        writeStatus=kek_diagnostics_SelfWriteDiagnosticDump(&diagnostics,out);
    } else {
        if (status!=Status_Ok) {
            writeStatus=StringBuilder_WriteCString(out,"diag|2|4|-1|0|0|0|0|compile failed\n");
        }
    }
    StringBuilder_Destroy(&generated);
    kek_diagnostics_SelfDiagnosticBagDestroy(&diagnostics);
    if (writeStatus!=Status_Ok) {
        return (writeStatus);
    }
    return (Status_Ok);
}
Status kek_compiler_SelfCompilerRunBuild(str inputPath,str outputPath) {
    struct Allocator allocator=std_mem_DefaultAllocator();
    struct SelfDiagnosticBag diagnostics={0};
    kek_diagnostics_SelfDiagnosticBagInit(&diagnostics,allocator);
    Status status=kek_compiler_SelfCompileToC(inputPath,outputPath,allocator,&diagnostics);
    if (diagnostics.count>0) {
        struct StringBuilder dump=std_string_StringBuilderNew(allocator);
        kek_diagnostics_SelfWriteDiagnosticDump(&diagnostics,&dump);
        struct File stderr=std_file_Stderr();
        struct String view=StringBuilder_View(&dump);
        File_Write(&stderr,String_Bytes(&view));
        StringBuilder_Destroy(&dump);
    }
    kek_diagnostics_SelfDiagnosticBagDestroy(&diagnostics);
    return (status);
}
int kek_compiler_SelfCompilerMain(int argc,str* argv) {
    if (argc<2) {
        return (kek_compiler_SelfCompilerFail("missing command\n"));
    }
    struct String command=std_string_StringFromCString(argv[1]);
    if (String_EqualsCString(&command,"--help")) {
        return (kek_compiler_SelfCompilerPrintHelp());
    }
    if (String_EqualsCString(&command,"--version")) {
        struct File stdout=std_file_Stdout();
        std_format_WriteStringToFile(&stdout,std_string_StringFromCString("0.3.0-self\n"));
        return (0);
    }
    if (!String_EqualsCString(&command,"build")) {
        return (kek_compiler_SelfCompilerFail("unknown command\n"));
    }
    if (argc<3) {
        return (kek_compiler_SelfCompilerFail("missing input\n"));
    }
    str inputPath=argv[2];
    str outputPath="out.kek.c";
    usize i=3;
    while (i<((usize)(argc))) {
        struct String arg=std_string_StringFromCString(argv[i]);
        if (String_EqualsCString(&arg,"-o")) {
            if (i+1>=((usize)(argc))) {
                return (kek_compiler_SelfCompilerFail("missing -o value\n"));
            }
            outputPath=argv[i+1];
            i+=2;
            continue;
        }
        return (kek_compiler_SelfCompilerFail("unknown build option\n"));
    }
    Status status=kek_compiler_SelfCompilerRunBuild(inputPath,outputPath);
    if (status!=Status_Ok) {
        return (kek_compiler_SelfCompilerFail("build failed\n"));
    }
    return (0);
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
Status std_thread_ThreadStart(struct Thread* thread,ptr entry,ptr arg) {
    return (std_thread_ThreadHandleStart(&thread->handle,entry,arg));
}
Status std_thread_ThreadJoin(struct Thread* thread) {
    return (std_thread_ThreadHandleJoin(&thread->handle));
}
Status Thread_Join(struct Thread* this) {
    return (std_thread_ThreadJoin(this));
}
