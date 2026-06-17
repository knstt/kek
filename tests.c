#include "kek_internal.h"

#include <errno.h>
#include <sys/stat.h>

static int Fail(const char* message) {
    fprintf(stderr, "test failed: %s\n", message);
    return 1;
}

static int WriteTextFile(const char* path, const char* text) {
    FILE* out = fopen(path, "w");
    if (!out) {
        return -1;
    }
    fputs(text, out);
    fclose(out);
    return 0;
}

static int FilesEqual(const char* leftPath, const char* rightPath) {
    FILE* left = fopen(leftPath, "rb");
    FILE* right = fopen(rightPath, "rb");
    if (!left || !right) {
        if (left) {
            fclose(left);
        }
        if (right) {
            fclose(right);
        }
        return 0;
    }

    int equal = 1;
    for (;;) {
        int leftChar = fgetc(left);
        int rightChar = fgetc(right);
        if (leftChar != rightChar) {
            equal = 0;
            break;
        }
        if (leftChar == EOF || rightChar == EOF) {
            break;
        }
    }

    fclose(left);
    fclose(right);
    return equal;
}

static int HasDiagnostic(struct KekDiagnosticBag* bag, enum KekDiagnosticPhase phase, const char* text) {
    for (size_t i = 0; i < bag->count; i++) {
        if (bag->items[i].phase == phase && strstr(bag->items[i].message, text)) {
            return 1;
        }
    }
    return 0;
}

static int FileContains(const char* path, const char* text) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }
    long length = ftell(file);
    if (length < 0) {
        fclose(file);
        return 0;
    }
    rewind(file);

    char* buffer = malloc((size_t)length + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    size_t read = fread(buffer, 1, (size_t)length, file);
    fclose(file);
    buffer[read] = '\0';
    int contains = strstr(buffer, text) != NULL;
    free(buffer);
    return contains;
}

static int FileExists(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    fclose(file);
    return 1;
}

static int TestCliInterface(void) {
    if (system("bin/kek --help > out/cli_help.txt") != 0) {
        return Fail("CLI --help failed");
    }
    if (!FileContains("out/cli_help.txt", "kek build <input.kek>")) {
        return Fail("CLI help did not mention build usage");
    }

    if (system("bin/kek --version > out/cli_version.txt") != 0) {
        return Fail("CLI --version failed");
    }
    if (!FileContains("out/cli_version.txt", VERSION)) {
        return Fail("CLI version did not print VERSION");
    }

    remove("out/out.c");
    remove("out/ast.json");
    remove("out/module.txt");
    if (system("bin/kek build tmp.kek > out/cli_build_default.stdout 2> out/cli_build_default.stderr") != 0) {
        return Fail("CLI build default failed");
    }
    if (!FileExists("out/out.c") || !FileExists("out/ast.json") || !FileExists("out/module.txt")) {
        return Fail("CLI build default did not write expected outputs");
    }

    remove("out/custom.c");
    remove("out/custom.json");
    remove("out/custom.txt");
    if (system("bin/kek build tmp.kek -o out/custom.c --ast-json out/custom.json --summary out/custom.txt > out/cli_build_custom.stdout 2> out/cli_build_custom.stderr") != 0) {
        return Fail("CLI build custom outputs failed");
    }
    if (!FileExists("out/custom.c") || !FileExists("out/custom.json") || !FileExists("out/custom.txt")) {
        return Fail("CLI build custom did not write expected outputs");
    }

    remove("out/cli-out-dir/out.c");
    remove("out/cli-out-dir/ast.json");
    remove("out/cli-out-dir/module.txt");
    if (system("bin/kek build tmp.kek --out-dir out/cli-out-dir > out/cli_build_out_dir.stdout 2> out/cli_build_out_dir.stderr") != 0) {
        return Fail("CLI build with out-dir failed");
    }
    if (!FileExists("out/cli-out-dir/out.c") || !FileExists("out/cli-out-dir/ast.json") || !FileExists("out/cli-out-dir/module.txt")) {
        return Fail("CLI build with out-dir did not write expected outputs");
    }

    if (system("bin/kek build > out/cli_missing.stdout 2> out/cli_missing.stderr") == 0) {
        return Fail("CLI build without input succeeded");
    }
    if (!FileContains("out/cli_missing.stderr", "missing input path")) {
        return Fail("CLI missing input did not explain the error");
    }

    if (system("bin/kek build tmp.kek --wat > out/cli_unknown.stdout 2> out/cli_unknown.stderr") == 0) {
        return Fail("CLI unknown flag succeeded");
    }
    if (!FileContains("out/cli_unknown.stderr", "unknown flag: --wat")) {
        return Fail("CLI unknown flag did not explain the error");
    }

    return 0;
}

static int TestCompilationApiRegression(void) {
    struct KekDiagnostic diagnostics[256];
    struct KekCompilation compilation;
    InitKekCompilation(&compilation, diagnostics, sizeof(diagnostics) / sizeof(diagnostics[0]));
    int result = CompileKekSmoke("tmp.kek", "out/api_out.c", "out/api_ast.json", "out/api_module.txt", &compilation);
    if (result != 0) {
        PrintKekDiagnostics(stderr, &compilation.diagnostics, &compilation.fileTable);
        FreeKekCompilation(&compilation);
        return Fail("compilation API failed on tmp.kek");
    }
    FreeKekCompilation(&compilation);

    if (!FilesEqual("out/out.c", "out/api_out.c")) {
        return Fail("compilation API generated different C");
    }
    if (!FilesEqual("out/ast.json", "out/api_ast.json")) {
        return Fail("compilation API generated different AST JSON");
    }
    if (!FilesEqual("out/module.txt", "out/api_module.txt")) {
        return Fail("compilation API generated different module summary");
    }
    return 0;
}

static int TestDeferExternStructAndAddressOf(void) {
    const char* source =
        "extern \"C\" {\n"
        "struct CPoint { int x; int y; };\n"
        "void touch(struct CPoint* point) { point->x += 1; }\n"
        "}\n"
        "struct:GenericBox<T> { T:value; };\n"
        "struct:ByteBuffer { ptr<u8>:data; };\n"
        "ptr<CPoint>:IdentityPoint(ptr<CPoint>:point) { return(point); }\n"
        "i64:main(){\n"
        "CPoint:point = { x = 1, y = 2 };\n"
        "ptr:pointer = &point;\n"
        "ptr<CPoint>:typedPointer = &point;\n"
        "ptr<CPoint>:returnedPointer = IdentityPoint(typedPointer);\n"
        "u8:bytes[4] = {0};\n"
        "ByteBuffer:buffer = { data = &bytes };\n"
        "GenericBox<u8>:box = { value = 9 };\n"
        "ptr<GenericBox<u8>>:boxPointer = &box;\n"
        "assert(pointer != 0);\n"
        "assert(typedPointer != 0);\n"
        "assert(returnedPointer != 0);\n"
        "assert(buffer.data != 0);\n"
        "assert(boxPointer != 0);\n"
        "::touch(&point);\n"
        "int:value = 0;\n"
        "if (true) { defer value += 1; defer { value += 2; } assert(value == 0); }\n"
        "assert(value == 3);\n"
        "assert(point.x == 2);\n"
        "return(0);\n"
        "}\n";

    if (WriteTextFile("out/features.kek", source) != 0) {
        return Fail("could not write feature fixture");
    }

    struct KekDiagnostic diagnostics[128];
    struct KekCompilation compilation;
    InitKekCompilation(&compilation, diagnostics, sizeof(diagnostics) / sizeof(diagnostics[0]));
    int result = CompileKekSmoke("out/features.kek", "out/features.c", "out/features.json", "out/features.txt", &compilation);
    if (result != 0) {
        PrintKekDiagnostics(stderr, &compilation.diagnostics, &compilation.fileTable);
        FreeKekCompilation(&compilation);
        return Fail("feature fixture did not compile");
    }
    FreeKekCompilation(&compilation);

    if (!FileContains("out/features.c", "struct CPoint point=")) {
        return Fail("extern C struct type was not emitted as a C struct");
    }
    if (!FileContains("out/features.c", "ptr pointer=&point;")) {
        return Fail("address-of instance did not emit native pointer expression");
    }
    if (!FileContains("out/features.c", "struct CPoint* typedPointer=&point;")) {
        return Fail("typed pointer to extern struct was not emitted as a C pointer");
    }
    if (!FileContains("out/features.c", "u8* data;")) {
        return Fail("typed pointer field was not emitted as a C pointer");
    }
    if (!FileContains("out/features.c", "struct GenericBox__u8* boxPointer=&box;")) {
        return Fail("typed pointer to generic struct was not emitted as a C pointer");
    }
    if (!FileContains("out/features.c", "value+=2;") || !FileContains("out/features.c", "value+=1;")) {
        return Fail("defer statements were not emitted");
    }
    return 0;
}

static int TestStdlibExample(void) {
    struct KekDiagnostic diagnostics[256];
    struct KekCompilation compilation;
    InitKekCompilation(&compilation, diagnostics, sizeof(diagnostics) / sizeof(diagnostics[0]));
    int result = CompileKekSmoke("tests/fixtures/std_smoke.kek", "out/std_smoke.c", "out/std_smoke.json", "out/std_smoke.txt", &compilation);
    if (result != 0) {
        PrintKekDiagnostics(stderr, &compilation.diagnostics, &compilation.fileTable);
        FreeKekCompilation(&compilation);
        return Fail("stdlib example did not compile");
    }
    FreeKekCompilation(&compilation);

    if (!FileContains("out/std_smoke.c", "struct Slice__byte")
        || !FileContains("out/std_smoke.c", "StringBuilder_Write")
        || !FileContains("out/std_smoke.c", "Array__byte_Push")
        || !FileContains("out/std_smoke.c", "LinkedList__byte_PushBack")
        || !FileContains("out/std_smoke.c", "File_Write")
        || !FileContains("out/std_smoke.c", "std_FormatI64ToBuilder")
        || !FileContains("out/std_smoke.c", "std_FileOpen")) {
        return Fail("stdlib example did not emit expected stdlib symbols");
    }
    if (!FileContains("out/std_smoke.c", "struct Result__File std_FileOpen(str path,FileMode mode);")) {
        return Fail("stdlib generated C did not emit expected function prototype");
    }

    if (system("cc -std=c11 -Wall -Wextra -Werror -o out/std_smoke out/std_smoke.c") != 0) {
        return Fail("stdlib generated C did not compile");
    }
    if (system("./out/std_smoke") != 0) {
        return Fail("stdlib generated binary failed");
    }
    return 0;
}

static int ParseChildCount(struct SourceFile* file, struct TokenArray* tokens) {
    size_t astCapacity = tokens->count * 4 + 1;
    struct AstNode* nodes = malloc(sizeof(*nodes) * astCapacity);
    if (!nodes) {
        return -1;
    }
    struct Parser parser = {0};
    parser.tokens = tokens->items;
    parser.count = tokens->count;
    parser.file = file;
    parser.astNodes = nodes;
    parser.astNodeCapacity = astCapacity;
    struct AstNode* ast = ParseAst(&parser);
    int count = parser.errorCount == 0 ? (int)ast->childCount : -1;
    free(nodes);
    return count;
}

static int TestToolingLexComments(void) {
    struct FileTable table = {0};
    if (ReadFile("tests/fixtures/docs.kek", &table) < 0) {
        return Fail("could not load tests/fixtures/docs.kek");
    }

    struct SourceFile* file = &table.files[0];
    struct Token* normalStorage = malloc(sizeof(*normalStorage) * (file->length + 1));
    struct Token* toolingStorage = malloc(sizeof(*toolingStorage) * (file->length + 1));
    if (!normalStorage || !toolingStorage) {
        free(normalStorage);
        free(toolingStorage);
        FreeFileTable(&table);
        return Fail("could not allocate lexer test storage");
    }

    struct Tokenizer normalTokenizer = CreateTokenizer(0, &table);
    struct TokenArray normalTokens = TokenizeFile(&normalTokenizer, normalStorage, file->length + 1);

    struct KekLexOptions options = {0};
    options.emitComments = 1;
    struct Tokenizer toolingTokenizer = CreateTokenizerWithOptions(0, &table, options);
    struct TokenArray toolingTokens = TokenizeFile(&toolingTokenizer, toolingStorage, file->length + 1);

    size_t normalComments = 0;
    size_t docComments = 0;
    struct SourceLocation firstDoc = {0};
    for (size_t i = 0; i < normalTokens.count; i++) {
        if (normalTokens.items[i].type == TOKEN_COMMENT || normalTokens.items[i].type == TOKEN_DOC_COMMENT) {
            normalComments++;
        }
    }
    for (size_t i = 0; i < toolingTokens.count; i++) {
        if (toolingTokens.items[i].type == TOKEN_DOC_COMMENT) {
            if (docComments == 0) {
                firstDoc = toolingTokens.items[i].location;
            }
            docComments++;
        }
    }

    int normalChildCount = ParseChildCount(file, &normalTokens);
    int toolingChildCount = ParseChildCount(file, &toolingTokens);

    free(normalStorage);
    free(toolingStorage);
    FreeFileTable(&table);

    if (normalComments != 0) {
        return Fail("normal lexer emitted comments");
    }
    if (docComments == 0 || firstDoc.line != 7 || firstDoc.column != 1) {
        return Fail("tooling lexer did not emit doc comments at expected location");
    }
    if (normalChildCount < 0 || toolingChildCount < 0 || normalChildCount != toolingChildCount) {
        return Fail("parser changed structural AST shape when comments were present");
    }
    return 0;
}

static int TestRewritePrepLanguageSurface(void) {
    if (WriteTextFile("out/in_token.kek", "in\n") != 0) {
        return Fail("could not write in token fixture");
    }

    struct FileTable table = {0};
    if (ReadFile("out/in_token.kek", &table) < 0) {
        return Fail("could not load in token fixture");
    }
    struct SourceFile* file = &table.files[0];
    struct Token* storage = malloc(sizeof(*storage) * (file->length + 1));
    if (!storage) {
        FreeFileTable(&table);
        return Fail("could not allocate in token storage");
    }
    struct Tokenizer tokenizer = CreateTokenizer(0, &table);
    struct TokenArray tokens = TokenizeFile(&tokenizer, storage, file->length + 1);
    int inIsIdentifier = tokens.count > 0
        && tokens.items[0].type == TOKEN_IDENTIFIER
        && tokens.items[0].location.length == 2;
    free(storage);
    FreeFileTable(&table);
    if (!inIsIdentifier) {
        return Fail("in did not tokenize as an identifier");
    }

    const char* charSource =
        "i64:main(){\n"
        "u8:a='a';\n"
        "u8:n='\\n';\n"
        "u8:t='\\t';\n"
        "u8:z='\\0';\n"
        "u8:q='\\'';\n"
        "u8:s='\\\\';\n"
        "assert(a == 97);\n"
        "assert(n == 10);\n"
        "assert(t == 9);\n"
        "assert(z == 0);\n"
        "assert(q == 39);\n"
        "assert(s == 92);\n"
        "return(0);\n"
        "}\n";
    if (WriteTextFile("out/char_literals.kek", charSource) != 0) {
        return Fail("could not write char literal fixture");
    }
    struct KekDiagnostic charDiagnostics[128];
    struct KekCompilation charCompilation;
    InitKekCompilation(&charCompilation, charDiagnostics, sizeof(charDiagnostics) / sizeof(charDiagnostics[0]));
    int charResult = CompileKekSmoke("out/char_literals.kek", "out/char_literals.c", "out/char_literals.json", "out/char_literals.txt", &charCompilation);
    if (charResult != 0) {
        PrintKekDiagnostics(stderr, &charCompilation.diagnostics, &charCompilation.fileTable);
        FreeKekCompilation(&charCompilation);
        return Fail("char literal fixture did not compile");
    }
    FreeKekCompilation(&charCompilation);
    if (!FileContains("out/char_literals.c", "u8 a=97;")
        || !FileContains("out/char_literals.c", "u8 n=10;")
        || !FileContains("out/char_literals.c", "u8 s=92;")) {
        return Fail("char literals were not normalized in generated C");
    }
    if (system("cc -std=c11 -Wall -Wextra -Werror -o out/char_literals out/char_literals.c") != 0) {
        return Fail("char literal generated C did not compile");
    }
    if (system("./out/char_literals") != 0) {
        return Fail("char literal binary failed");
    }

    const char* forInSource =
        "i64:main(){\n"
        "u8:items[1] = { 1 };\n"
        "for (u8:item) in items { }\n"
        "return(0);\n"
        "}\n";
    if (WriteTextFile("out/for_in_removed.kek", forInSource) != 0) {
        return Fail("could not write retired iteration fixture");
    }
    struct KekDiagnostic forInDiagnostics[128];
    struct KekCompilation forInCompilation;
    InitKekCompilation(&forInCompilation, forInDiagnostics, sizeof(forInDiagnostics) / sizeof(forInDiagnostics[0]));
    int forInResult = CompileKekSmoke("out/for_in_removed.kek", "out/for_in_removed.c", "out/for_in_removed.json", "out/for_in_removed.txt", &forInCompilation);
    int hasForInDiagnostic = HasDiagnostic(&forInCompilation.diagnostics, KEK_PHASE_TYPED_PARSE, "for-in syntax is not supported");
    FreeKekCompilation(&forInCompilation);
    if (forInResult == 0 || !hasForInDiagnostic) {
        return Fail("retired for-in syntax did not fail with a typed parse diagnostic");
    }

    if (system("rg \"example\"\"/\" README.md Makefile tests.c tmp.kek std/*.kek *.c *.h > out/no_example_refs.txt") == 0) {
        return Fail("active files still reference the retired fixture directory");
    }

    return 0;
}

static int TestDiagnostics(void) {
    if (WriteTextFile("out/diagnostic_parse.kek", "main:() {\n") != 0) {
        return Fail("could not write parse diagnostic fixture");
    }
    if (WriteTextFile("out/diagnostic_duplicate.kek", "i32:A;\ni32:A;\n") != 0) {
        return Fail("could not write semantic diagnostic fixture");
    }
    if (WriteTextFile("out/diagnostic_ptr_empty.kek", "ptr<>:Bad;\n") != 0) {
        return Fail("could not write empty ptr diagnostic fixture");
    }
    if (WriteTextFile("out/diagnostic_ptr_many.kek", "ptr<u8, u16>:Bad;\n") != 0) {
        return Fail("could not write multi ptr diagnostic fixture");
    }

    struct KekDiagnostic parseDiagnostics[64];
    struct KekCompilation parseCompilation;
    InitKekCompilation(&parseCompilation, parseDiagnostics, sizeof(parseDiagnostics) / sizeof(parseDiagnostics[0]));
    int parseResult = CompileKekSmoke("out/diagnostic_parse.kek", "out/diagnostic_parse.c", "out/diagnostic_parse.json", "out/diagnostic_parse.txt", &parseCompilation);
    int hasParseDiagnostic = parseCompilation.diagnostics.errorCount > 0
        && (HasDiagnostic(&parseCompilation.diagnostics, KEK_PHASE_PARSE, "unterminated")
            || HasDiagnostic(&parseCompilation.diagnostics, KEK_PHASE_PARSE, "expected"));
    FreeKekCompilation(&parseCompilation);
    if (parseResult == 0 || !hasParseDiagnostic) {
        return Fail("parse diagnostic was not recorded");
    }

    struct KekDiagnostic semanticDiagnostics[64];
    struct KekCompilation semanticCompilation;
    InitKekCompilation(&semanticCompilation, semanticDiagnostics, sizeof(semanticDiagnostics) / sizeof(semanticDiagnostics[0]));
    int semanticResult = CompileKekSmoke("out/diagnostic_duplicate.kek", "out/diagnostic_duplicate.c", "out/diagnostic_duplicate.json", "out/diagnostic_duplicate.txt", &semanticCompilation);
    int hasSemanticDiagnostic = HasDiagnostic(&semanticCompilation.diagnostics, KEK_PHASE_SEMANTIC, "duplicate symbol");
    FreeKekCompilation(&semanticCompilation);
    if (semanticResult == 0 || !hasSemanticDiagnostic) {
        return Fail("semantic diagnostic was not recorded");
    }

    struct KekDiagnostic ptrEmptyDiagnostics[64];
    struct KekCompilation ptrEmptyCompilation;
    InitKekCompilation(&ptrEmptyCompilation, ptrEmptyDiagnostics, sizeof(ptrEmptyDiagnostics) / sizeof(ptrEmptyDiagnostics[0]));
    int ptrEmptyResult = CompileKekSmoke("out/diagnostic_ptr_empty.kek", "out/diagnostic_ptr_empty.c", "out/diagnostic_ptr_empty.json", "out/diagnostic_ptr_empty.txt", &ptrEmptyCompilation);
    int hasPtrEmptyDiagnostic = HasDiagnostic(&ptrEmptyCompilation.diagnostics, KEK_PHASE_TYPED_PARSE, "ptr<T> requires exactly one type argument");
    FreeKekCompilation(&ptrEmptyCompilation);
    if (ptrEmptyResult == 0 || !hasPtrEmptyDiagnostic) {
        return Fail("empty ptr generic diagnostic was not recorded");
    }

    struct KekDiagnostic ptrManyDiagnostics[64];
    struct KekCompilation ptrManyCompilation;
    InitKekCompilation(&ptrManyCompilation, ptrManyDiagnostics, sizeof(ptrManyDiagnostics) / sizeof(ptrManyDiagnostics[0]));
    int ptrManyResult = CompileKekSmoke("out/diagnostic_ptr_many.kek", "out/diagnostic_ptr_many.c", "out/diagnostic_ptr_many.json", "out/diagnostic_ptr_many.txt", &ptrManyCompilation);
    int hasPtrManyDiagnostic = HasDiagnostic(&ptrManyCompilation.diagnostics, KEK_PHASE_TYPED_PARSE, "ptr<T> requires exactly one type argument");
    FreeKekCompilation(&ptrManyCompilation);
    if (ptrManyResult == 0 || !hasPtrManyDiagnostic) {
        return Fail("multi ptr generic diagnostic was not recorded");
    }

    struct KekDiagnostic codegenDiagnostics[64];
    struct KekCompilation codegenCompilation;
    InitKekCompilation(&codegenCompilation, codegenDiagnostics, sizeof(codegenDiagnostics) / sizeof(codegenDiagnostics[0]));
    int codegenResult = CompileKekSmoke("tmp.kek", "out/missing-dir/out.c", "out/missing-dir/ast.json", "out/missing-dir/module.txt", &codegenCompilation);
    int hasCodegenDiagnostic = HasDiagnostic(&codegenCompilation.diagnostics, KEK_PHASE_CODEGEN, "could not write C file");
    FreeKekCompilation(&codegenCompilation);
    if (codegenResult == 0 || !hasCodegenDiagnostic) {
        return Fail("codegen diagnostic was not recorded");
    }

    return 0;
}

int main(void) {
    if (mkdir("out", 0755) != 0 && errno != EEXIST) {
        return Fail("could not create out directory");
    }
    if (TestCliInterface() != 0) {
        return 1;
    }
    if (TestCompilationApiRegression() != 0) {
        return 1;
    }
    if (TestToolingLexComments() != 0) {
        return 1;
    }
    if (TestDeferExternStructAndAddressOf() != 0) {
        return 1;
    }
    if (TestStdlibExample() != 0) {
        return 1;
    }
    if (TestRewritePrepLanguageSurface() != 0) {
        return 1;
    }
    if (TestDiagnostics() != 0) {
        return 1;
    }
    printf("API/tooling tests passed\n");
    return 0;
}
