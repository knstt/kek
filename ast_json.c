#include "kek.h"

void WriteJsonEscaped(FILE* out, const char* text, size_t length) {
    fputc('"', out);
    for (size_t i = 0; i < length; i++) {
        unsigned char c = (unsigned char)text[i];
        switch (c) {
            case '"': fputs("\\\"", out); break;
            case '\\': fputs("\\\\", out); break;
            case '\n': fputs("\\n", out); break;
            case '\r': fputs("\\r", out); break;
            case '\t': fputs("\\t", out); break;
            default:
                if (c < 32) {
                    fprintf(out, "\\u%04x", c);
                } else {
                    fputc(c, out);
                }
                break;
        }
    }
    fputc('"', out);
}

static void WriteTokenTextJson(FILE* out, struct Token* token, struct SourceFile* file) {
    if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_NUMBER || token->type == TOKEN_STRING) {
        WriteJsonEscaped(out, file->content + token->location.offset, token->location.length);
    } else if (token->type == TOKEN_EOF) {
        WriteJsonEscaped(out, "", 0);
    } else {
        const char* lexeme = TokenLexeme(token, file);
        WriteJsonEscaped(out, lexeme, strlen(lexeme));
    }
}

void WriteAstJson(FILE* out, struct AstNode* node, struct SourceFile* file, int indent) {
    for (int i = 0; i < indent; i++) fputs("  ", out);
    fputs("{\n", out);

    for (int i = 0; i < indent + 1; i++) fputs("  ", out);
    fputs("\"type\": ", out);
    WriteJsonEscaped(out, AstNodeTypeNames[node->type], strlen(AstNodeTypeNames[node->type]));
    fputs(",\n", out);

    for (int i = 0; i < indent + 1; i++) fputs("  ", out);
    fprintf(out, "\"line\": %zu, \"column\": %zu, \"offset\": %zu, \"length\": %zu",
        node->location.line, node->location.column, node->location.offset, node->location.length);

    if (node->type == AST_TOKEN) {
        fputs(",\n", out);
        for (int i = 0; i < indent + 1; i++) fputs("  ", out);
        fputs("\"tokenType\": ", out);
        WriteJsonEscaped(out, TokenTypeNames[node->token.type], strlen(TokenTypeNames[node->token.type]));
        fputs(",\n", out);
        for (int i = 0; i < indent + 1; i++) fputs("  ", out);
        fputs("\"text\": ", out);
        WriteTokenTextJson(out, &node->token, file);
    }

    fputs(",\n", out);
    for (int i = 0; i < indent + 1; i++) fputs("  ", out);
    fputs("\"children\": [", out);
    if (node->childCount > 0) {
        fputc('\n', out);
        size_t index = 0;
        for (struct AstNode* child = node->firstChild; child; child = child->nextSibling) {
            WriteAstJson(out, child, file, indent + 2);
            if (index + 1 < node->childCount) {
                fputc(',', out);
            }
            fputc('\n', out);
            index++;
        }
        for (int i = 0; i < indent + 1; i++) fputs("  ", out);
    }
    fputs("]\n", out);

    for (int i = 0; i < indent; i++) fputs("  ", out);
    fputc('}', out);
}

int WriteAstJsonFile(const char* path, struct AstNode* ast, struct SourceFile* file) {
    FILE* out = fopen(path, "w");
    if (!out) {
        return -1;
    }
    WriteAstJson(out, ast, file, 0);
    fputc('\n', out);
    fclose(out);
    return 0;
}
