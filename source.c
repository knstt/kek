#include "kek.h"

int ReadFile(const char* path, struct FileTable* table) {
    if (table->count >= MAX_FILES) {
        fprintf(stderr, "Error: File table is full.\n");
        return -1;
    }

    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", path);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = malloc((size_t)length + 1);
    if (!content) {
        fprintf(stderr, "Error: Could not allocate memory for file content.\n");
        fclose(file);
        return -1;
    }

    fread(content, 1, (size_t)length, file);
    content[length] = '\0';
    fclose(file);

    struct SourceFile* sourceFile = &table->files[table->count++];
    strncpy(sourceFile->path, path, MAX_PATH_LENGTH - 1);
    sourceFile->path[MAX_PATH_LENGTH - 1] = '\0';
    sourceFile->content = content;
    sourceFile->length = (size_t)length;

    return (int)table->count - 1;
}

void FreeFileTable(struct FileTable* table) {
    for (size_t i = 0; i < table->count; i++) {
        free(table->files[i].content);
        table->files[i].content = NULL;
        table->files[i].length = 0;
    }
    table->count = 0;
}
