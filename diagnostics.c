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

void PrintKekDiagnostics(FILE* out, struct KekDiagnosticBag* bag, struct FileTable* table) {
    if (!out || !bag) {
        return;
    }

    for (size_t i = 0; i < bag->count; i++) {
        struct KekDiagnostic* diagnostic = &bag->items[i];
        const char* path = "<unknown>";
        if (table
            && diagnostic->fileIndex >= 0
            && (size_t)diagnostic->fileIndex < table->count) {
            path = table->files[diagnostic->fileIndex].path;
        }
        fprintf(out, "%s:%zu:%zu: %s[%s]: %s\n",
            path,
            diagnostic->location.line,
            diagnostic->location.column,
            DiagnosticSeverityName(diagnostic->severity),
            DiagnosticPhaseName(diagnostic->phase),
            diagnostic->message);
    }
}

