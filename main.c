#include "kek.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

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
