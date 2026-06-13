#include "kek.h"

#include <stdarg.h>

static const char* DiagnosticSeverityName(enum KekDiagnosticSeverity severity) {
    switch (severity) {
        case KEK_DIAGNOSTIC_NOTE:
            return "note";
        case KEK_DIAGNOSTIC_WARNING:
            return "warning";
        case KEK_DIAGNOSTIC_ERROR:
            return "error";
    }
    return "diagnostic";
}

static const char* DiagnosticPhaseName(enum KekDiagnosticPhase phase) {
    switch (phase) {
        case KEK_PHASE_SOURCE:
            return "source";
        case KEK_PHASE_LEX:
            return "lex";
        case KEK_PHASE_PARSE:
            return "parse";
        case KEK_PHASE_TYPED_PARSE:
            return "typed-parse";
        case KEK_PHASE_SEMANTIC:
            return "semantic";
        case KEK_PHASE_CODEGEN:
            return "codegen";
    }
    return "unknown";
}

void InitKekDiagnosticBag(struct KekDiagnosticBag* bag, struct KekDiagnostic* storage, size_t capacity) {
    if (!bag) {
        return;
    }
    bag->items = storage;
    bag->count = 0;
    bag->capacity = capacity;
    bag->errorCount = 0;
}

void KekAddDiagnostic(struct KekDiagnosticBag* bag, enum KekDiagnosticSeverity severity, enum KekDiagnosticPhase phase, int fileIndex, struct SourceLocation location, const char* message) {
    if (!bag) {
        return;
    }
    if (severity == KEK_DIAGNOSTIC_ERROR) {
        bag->errorCount++;
    }
    if (bag->count >= bag->capacity || !bag->items) {
        return;
    }

    struct KekDiagnostic* diagnostic = &bag->items[bag->count++];
    memset(diagnostic, 0, sizeof(*diagnostic));
    diagnostic->severity = severity;
    diagnostic->phase = phase;
    diagnostic->fileIndex = fileIndex;
    diagnostic->location = location;
    snprintf(diagnostic->message, sizeof(diagnostic->message), "%s", message ? message : "");
}

void KekAddDiagnosticFormat(struct KekDiagnosticBag* bag, enum KekDiagnosticSeverity severity, enum KekDiagnosticPhase phase, int fileIndex, struct SourceLocation location, const char* format, ...) {
    if (!bag) {
        return;
    }

    char message[256];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format ? format : "", args);
    va_end(args);

    KekAddDiagnostic(bag, severity, phase, fileIndex, location, message);
}

static void PrintSourceLine(FILE* out, struct SourceFile* file, struct SourceLocation location) {
    if (!file || !file->content || location.offset > file->length) {
        return;
    }

    // Find the start of the line
    size_t lineStart = location.offset;
    while (lineStart > 0 && file->content[lineStart - 1] != '\n') {
        lineStart--;
    }

    // Find the end of the line
    size_t lineEnd = location.offset;
    while (lineEnd < file->length && file->content[lineEnd] != '\n') {
        lineEnd++;
    }

    // Print the source line
    size_t lineLength = lineEnd - lineStart;
    if (lineLength > 0) {
        fprintf(out, " %5zu | %.*s\n", location.line, (int)lineLength, file->content + lineStart);

        // Print the column marker
        fprintf(out, "       | ");
        size_t markerCol = location.offset - lineStart;
        for (size_t j = 0; j < markerCol; j++) {
            char c = file->content[lineStart + j];
            fputc(c == '\t' ? '\t' : ' ', out);
        }
        fputc('^', out);
        // Extend marker for multi-character tokens
        size_t tokenLen = location.length > 0 ? location.length : 1;
        for (size_t j = 1; j < tokenLen && (lineStart + markerCol + j) < lineEnd; j++) {
            fputc('~', out);
        }
        fputc('\n', out);
    }
}

void PrintKekDiagnostics(FILE* out, struct KekDiagnosticBag* bag, struct FileTable* table) {
    if (!out || !bag) {
        return;
    }

    for (size_t i = 0; i < bag->count; i++) {
        struct KekDiagnostic* diagnostic = &bag->items[i];
        const char* path = "<unknown>";
        struct SourceFile* file = NULL;
        if (table
            && diagnostic->fileIndex >= 0
            && (size_t)diagnostic->fileIndex < table->count) {
            file = &table->files[diagnostic->fileIndex];
            path = file->path;
        }
        fprintf(out, "%s:%zu:%zu: %s[%s]: %s\n",
            path,
            diagnostic->location.line,
            diagnostic->location.column,
            DiagnosticSeverityName(diagnostic->severity),
            DiagnosticPhaseName(diagnostic->phase),
            diagnostic->message);

        // Print the source line with column marker
        if (file && diagnostic->location.line > 0) {
            PrintSourceLine(out, file, diagnostic->location);
        }
    }
}

