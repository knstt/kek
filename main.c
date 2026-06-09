#include "kek.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

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

    struct KekDiagnostic diagnostics[256];
    struct KekCompilation compilation;
    InitKekCompilation(&compilation, diagnostics, sizeof(diagnostics) / sizeof(diagnostics[0]));

    int result = CompileKekSmoke(SMOKE_SOURCE_PATH, OUT_C_PATH, OUT_AST_JSON_PATH, OUT_MODULE_SUMMARY_PATH, &compilation);
    PrintKekDiagnostics(stderr, &compilation.diagnostics, &compilation.fileTable);
    FreeKekCompilation(&compilation);
    return result;
}

