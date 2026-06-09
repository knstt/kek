#include "kek.h"

#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct CompilationUnit {
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

static int LoadImportDirectory(const char* path, struct FileTable* table) {
    DIR* dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "Error: Could not open import directory %s\n", path);
        return -1;
    }

    int result = 0;
    struct dirent* entry = NULL;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' || !EndsWith(entry->d_name, ".kek")) {
            continue;
        }

        char filePath[MAX_PATH_LENGTH];
        int written = snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);
        if (written < 0 || (size_t)written >= sizeof(filePath)) {
            fprintf(stderr, "Error: Import path is too long: %s/%s\n", path, entry->d_name);
            result = -1;
            break;
        }

        if (!FileAlreadyLoaded(table, filePath) && ReadFile(filePath, table) < 0) {
            result = -1;
            break;
        }
    }

    closedir(dir);
    return result;
}

static int LoadImports(struct SourceFile* file, struct FileTable* table) {
    const char* importPrefix = "#import(";
    size_t importPrefixLength = strlen(importPrefix);
    const char* cursor = file->content;

    while ((cursor = strstr(cursor, importPrefix)) != NULL) {
        const char* start = cursor + importPrefixLength;
        const char* end = strchr(start, ')');
        if (!end) {
            fprintf(stderr, "Error: Unterminated import in %s\n", file->path);
            return -1;
        }

        size_t length = (size_t)(end - start);
        if (length == 0 || length >= MAX_PATH_LENGTH) {
            fprintf(stderr, "Error: Invalid import path in %s\n", file->path);
            return -1;
        }

        char importPath[MAX_PATH_LENGTH];
        memcpy(importPath, start, length);
        importPath[length] = '\0';
        if (LoadImportDirectory(importPath, table) != 0) {
            return -1;
        }

        cursor = end + 1;
    }

    return 0;
}

static int ParseUnit(struct CompilationUnit* unit, struct FileTable* table) {
    struct SourceFile* sourceFile = &table->files[unit->fileIndex];
    size_t tokenCapacity = sourceFile->length + 1;
    unit->tokens = malloc(sizeof(*unit->tokens) * tokenCapacity);
    if (!unit->tokens) {
        fprintf(stderr, "Error: Could not allocate token storage for %s\n", sourceFile->path);
        return -1;
    }

    struct Tokenizer tokenizer = CreateTokenizer(unit->fileIndex, table);
    struct TokenArray tokens = TokenizeFile(&tokenizer, unit->tokens, tokenCapacity);

    size_t astNodeCapacity = tokens.count * 4 + 1;
    unit->astNodes = malloc(sizeof(*unit->astNodes) * astNodeCapacity);
    if (!unit->astNodes) {
        fprintf(stderr, "Error: Could not allocate AST storage for %s\n", sourceFile->path);
        return -1;
    }

    struct Parser parser = {0};
    parser.tokens = tokens.items;
    parser.count = tokens.count;
    parser.position = 0;
    parser.file = sourceFile;
    parser.astNodes = unit->astNodes;
    parser.astNodeCapacity = astNodeCapacity;
    unit->ast = ParseAst(&parser);

    if (parser.errorCount > 0) {
        fprintf(stderr, "Parsing failed with %d errors in %s\n", parser.errorCount, sourceFile->path);
        return -1;
    }

    size_t declCapacity = unit->ast->childCount + 1;
    unit->decls = malloc(sizeof(*unit->decls) * declCapacity);
    if (!unit->decls) {
        fprintf(stderr, "Error: Could not allocate typed declaration storage for %s\n", sourceFile->path);
        return -1;
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
        fprintf(stderr, "Error: Could not allocate typed AST storage for %s\n", sourceFile->path);
        return -1;
    }

    struct KekFrontend frontend = {0};
    frontend.decls = unit->decls;
    frontend.declCapacity = declCapacity;
    frontend.types = unit->types;
    frontend.typeCapacity = typeCapacity;
    frontend.exprs = unit->exprs;
    frontend.exprCapacity = exprCapacity;
    frontend.stmts = unit->stmts;
    frontend.stmtCapacity = stmtCapacity;
    frontend.params = unit->params;
    frontend.paramCapacity = paramCapacity;
    frontend.fields = unit->fields;
    frontend.fieldCapacity = fieldCapacity;
    frontend.variants = unit->variants;
    frontend.variantCapacity = variantCapacity;
    unit->module = ParseKekModule(&frontend, unit->ast, sourceFile);
    if (unit->module.errorCount > 0) {
        fprintf(stderr, "Typed parsing failed with %d errors in %s\n", unit->module.errorCount, sourceFile->path);
        return -1;
    }

    return 0;
}

static void FreeCompilationUnit(struct CompilationUnit* unit) {
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

static const char* SMOKE_SOURCE_PATH = "tmp.kek";
static const char* OUT_C_PATH = "out/out.c";
static const char* OUT_AST_JSON_PATH = "out/ast.json";
static const char* OUT_MODULE_SUMMARY_PATH = "out/module.txt";

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    if (mkdir("out", 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error creating output directory: %s\n", strerror(errno));
        return 1;
    }

    struct FileTable fileTable = {0};
    int fileIndex = ReadFile(SMOKE_SOURCE_PATH, &fileTable);
    if (fileIndex < 0) {
        return 1;
    }

    int result = 0;
    if (LoadImports(&fileTable.files[fileIndex], &fileTable) != 0) {
        FreeFileTable(&fileTable);
        return 1;
    }

    struct CompilationUnit units[MAX_FILES] = {0};
    struct AstNode* asts[MAX_FILES] = {0};
    struct SourceFile* files[MAX_FILES] = {0};
    size_t unitCount = 0;
    for (size_t i = 0; i < fileTable.count; i++) {
        if ((int)i == fileIndex) {
            continue;
        }
        units[unitCount].fileIndex = (int)i;
        if (ParseUnit(&units[unitCount], &fileTable) != 0) {
            result = 1;
            break;
        }
        asts[unitCount] = units[unitCount].ast;
        files[unitCount] = &fileTable.files[i];
        unitCount++;
    }

    if (result == 0) {
        units[unitCount].fileIndex = fileIndex;
        if (ParseUnit(&units[unitCount], &fileTable) != 0) {
            result = 1;
        } else {
            asts[unitCount] = units[unitCount].ast;
            files[unitCount] = &fileTable.files[fileIndex];
            unitCount++;
        }
    }

    struct KekModule modules[MAX_FILES] = {0};
    if (result == 0) {
        for (size_t i = 0; i < unitCount; i++) {
            modules[i] = units[i].module;
        }
    }

    if (result == 0) {
        result = WriteTypedCFileForModules(OUT_C_PATH, modules, unitCount);
    }

    if (result != 0) {
        fprintf(stderr, "Error writing C file\n");
        for (size_t i = 0; i < unitCount; i++) {
            FreeCompilationUnit(&units[i]);
        }
        FreeFileTable(&fileTable);
        return result;
    }

    result = WriteAstJsonFile(OUT_AST_JSON_PATH, asts[unitCount - 1], files[unitCount - 1]);

    if (result != 0) {
        fprintf(stderr, "Error writing AST JSON file\n");
    }

    size_t symbolCapacity = 1;
    size_t scopeCapacity = 1 + unitCount;
    for (size_t i = 0; i < unitCount; i++) {
        symbolCapacity += units[i].module.declCount
            + units[i].module.paramCount
            + units[i].module.typedStmtCount;
        scopeCapacity += units[i].module.declKindCounts[KEK_DECL_FUNCTION]
            + units[i].module.stmtKindCounts[KEK_STMT_BLOCK]
            + units[i].module.stmtKindCounts[KEK_STMT_FOR];
    }

    struct KekSymbol* symbols = malloc(sizeof(*symbols) * symbolCapacity);
    if (!symbols) {
        fprintf(stderr, "Error: Could not allocate global symbol storage\n");
        result = 1;
    }
    struct KekScope* scopes = malloc(sizeof(*scopes) * scopeCapacity);
    if (!scopes) {
        fprintf(stderr, "Error: Could not allocate symbol scope storage\n");
        result = 1;
    }

    struct KekProgram program = {0};
    if (result == 0) {
        program.symbols = symbols;
        program.symbolCapacity = symbolCapacity;
        program.scopes = scopes;
        program.scopeCapacity = scopeCapacity;
        if (BuildKekProgramSymbols(&program, modules, unitCount) != 0) {
            result = 1;
        }
    }

    if (WriteKekModuleSummaryFile(OUT_MODULE_SUMMARY_PATH, modules, unitCount, &program) != 0) {
        result = 1;
    }
    free(symbols);
    free(scopes);

    for (size_t i = 0; i < unitCount; i++) {
        FreeCompilationUnit(&units[i]);
    }
    FreeFileTable(&fileTable);

    return result;
}
