#include "kek/lexer.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, char **out_data, size_t *out_length)
{
	FILE *file;
	long size;
	size_t read_count;
	char *data;

	file = fopen(path, "rb");
	if (file == NULL) {
		fprintf(stderr, "%s: %s\n", path, strerror(errno));
		return 0;
	}

	if (fseek(file, 0, SEEK_END) != 0) {
		fprintf(stderr, "%s: failed to seek\n", path);
		fclose(file);
		return 0;
	}

	size = ftell(file);
	if (size < 0) {
		fprintf(stderr, "%s: failed to get file size\n", path);
		fclose(file);
		return 0;
	}
	rewind(file);

	data = (char *)malloc((size_t)size + 1);
	if (data == NULL) {
		fprintf(stderr, "%s: out of memory\n", path);
		fclose(file);
		return 0;
	}

	read_count = fread(data, 1, (size_t)size, file);
	fclose(file);
	if (read_count != (size_t)size) {
		fprintf(stderr, "%s: failed to read file\n", path);
		free(data);
		return 0;
	}

	data[size] = '\0';
	*out_data = data;
	*out_length = (size_t)size;
	return 1;
}

static void print_token(const KekToken *token)
{
	printf("%zu:%zu %-22s ", token->pos.line, token->pos.column, kek_token_kind_name(token->kind));
	if (token->kind == KEK_TOKEN_EOF) {
		printf("<eof>\n");
		return;
	}
	printf("`%.*s`", (int)token->length, token->start);
	if (token->message != NULL) {
		printf(" %s", token->message);
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	char *source;
	size_t length;
	KekLexer lexer;
	KekToken token;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <file.kek>\n", argv[0]);
		return 2;
	}

	if (!read_file(argv[1], &source, &length)) {
		return 1;
	}

	kek_lexer_init(&lexer, argv[1], source, length);
	do {
		token = kek_lexer_next(&lexer);
		print_token(&token);
		if (token.kind == KEK_TOKEN_ERROR) {
			free(source);
			return 1;
		}
	} while (token.kind != KEK_TOKEN_EOF);

	free(source);
	return 0;
}
