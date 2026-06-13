#include "kek_internal.h"

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

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
        symbolCapacity += compilation->modules[i].declCount
            + compilation->modules[i].paramCount
            + compilation->modules[i].typedStmtCount;
        scopeCapacity += compilation->modules[i].declKindCounts[KEK_DECL_FUNCTION]
            + compilation->modules[i].stmtKindCounts[KEK_STMT_BLOCK]
            + compilation->modules[i].stmtKindCounts[KEK_STMT_FOR];
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
