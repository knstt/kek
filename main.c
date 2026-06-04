#include "kek.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void PrintUsage(const char* program) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s <source.kek>                 Print tokens\n", program);
    fprintf(stderr, "\nOpen ast_viewer.html in a browser and load the JSON file to view the AST.\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 1;
    }

    if(mkdir("out", 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error creating output directory: %s\n", strerror(errno));
        return 1;
    }

    const char* sourcePath = argv[1];
    const char* out_c = "out/out.c";
    const char* out_ast_json = "out/ast.json";

    struct FileTable fileTable = {0};
    int fileIndex = ReadFile(sourcePath, &fileTable);
    if (fileIndex < 0) {
        return 1;
    }

    struct Tokenizer tokenizer = CreateTokenizer(fileIndex, &fileTable);
    struct TokenArray tokens = TokenizeFile(&tokenizer);
    int result = 0;

    struct Parser parser = {0};
    parser.tokens = tokens.items;
    parser.count = tokens.count;
    parser.position = 0;
    parser.file = tokenizer.file;
    struct AstNode* ast = ParseAst(&parser);

    if (parser.errorCount > 0) {
        fprintf(stderr, "Parsing failed with %d errors\n", parser.errorCount);
        result = 1;
    }

    result = WriteCFile(out_c, &tokens, tokenizer.file);

    if (result != 0) {
        fprintf(stderr, "Error writing C file\n");
        return result;
    }

    result = WriteAstJsonFile(out_ast_json, ast, tokenizer.file);

    if (result != 0) {
        fprintf(stderr, "Error writing AST JSON file\n");
        return result;
    }

    
    FreeAst(ast);
    FreeTokenArray(&tokens);
    FreeFileTable(&fileTable);

    return result;
}
