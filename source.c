#include "kek.h"

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
