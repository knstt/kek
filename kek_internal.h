#ifndef KEK_INTERNAL_H
#define KEK_INTERNAL_H

#include "kek.h"

struct Parser {
    struct Token* tokens;
    size_t count;
    size_t position;
    struct SourceFile* file;
    struct AstNode* astNodes;
    size_t astNodeCount;
    size_t astNodeCapacity;
    int errorCount;
    struct KekDiagnosticBag* diagnostics;
};

struct KekNodePool {
    void* items;
    size_t count;
    size_t capacity;
    size_t itemSize;
};

struct KekFrontend {
    struct SourceFile* file;
    struct KekNodePool decls;
    struct KekNodePool types;
    struct KekNodePool exprs;
    struct KekNodePool stmts;
    struct KekNodePool params;
    struct KekNodePool fields;
    struct KekNodePool variants;
    int errorCount;
    struct KekDiagnosticBag* diagnostics;
};

struct AstNode* ParseAst(struct Parser* parser);
struct KekModule ParseKekModule(struct KekFrontend* frontend, struct AstNode* ast, struct SourceFile* file);
int BuildKekProgramSymbols(struct KekProgram* program, struct KekModule* modules, size_t moduleCount);
void AttachKekDocComments(struct KekModule* module, struct TokenArray* tokens, struct SourceFile* file);

#endif
