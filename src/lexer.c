#include "kek/lexer.h"

#include <ctype.h>
#include <string.h>

typedef struct KekKeyword {
	const char *text;
	KekTokenKind kind;
} KekKeyword;

static const KekKeyword keywords[] = {
	{"alias", KEK_TOKEN_KW_ALIAS},
	{"alignof", KEK_TOKEN_KW_ALIGNOF},
	{"assert", KEK_TOKEN_KW_ASSERT},
	{"break", KEK_TOKEN_KW_BREAK},
	{"case", KEK_TOKEN_KW_CASE},
	{"cast", KEK_TOKEN_KW_CAST},
	{"continue", KEK_TOKEN_KW_CONTINUE},
	{"default", KEK_TOKEN_KW_DEFAULT},
	{"do", KEK_TOKEN_KW_DO},
	{"else", KEK_TOKEN_KW_ELSE},
	{"enum", KEK_TOKEN_KW_ENUM},
	{"export", KEK_TOKEN_KW_EXPORT},
	{"extern", KEK_TOKEN_KW_EXTERN},
	{"false", KEK_TOKEN_KW_FALSE},
	{"for", KEK_TOKEN_KW_FOR},
	{"if", KEK_TOKEN_KW_IF},
	{"in", KEK_TOKEN_KW_IN},
	{"len", KEK_TOKEN_KW_LEN},
	{"offsetof", KEK_TOKEN_KW_OFFSETOF},
	{"packed", KEK_TOKEN_KW_PACKED},
	{"panic", KEK_TOKEN_KW_PANIC},
	{"return", KEK_TOKEN_KW_RETURN},
	{"sizeof", KEK_TOKEN_KW_SIZEOF},
	{"struct", KEK_TOKEN_KW_STRUCT},
	{"switch", KEK_TOKEN_KW_SWITCH},
	{"this", KEK_TOKEN_KW_THIS},
	{"true", KEK_TOKEN_KW_TRUE},
	{"union", KEK_TOKEN_KW_UNION},
	{"unreachable", KEK_TOKEN_KW_UNREACHABLE},
	{"using", KEK_TOKEN_KW_USING},
	{"while", KEK_TOKEN_KW_WHILE},
};

static int at_end(const KekLexer *lexer)
{
	return lexer->index >= lexer->length;
}

static char peek(const KekLexer *lexer)
{
	return at_end(lexer) ? '\0' : lexer->source[lexer->index];
}

static char peek_next(const KekLexer *lexer)
{
	return lexer->index + 1 >= lexer->length ? '\0' : lexer->source[lexer->index + 1];
}

static char advance(KekLexer *lexer)
{
	char c = peek(lexer);
	if (c == '\0') {
		return c;
	}

	lexer->index++;
	if (c == '\n') {
		lexer->line++;
		lexer->column = 1;
	} else {
		lexer->column++;
	}

	return c;
}

static int match(KekLexer *lexer, char expected)
{
	if (peek(lexer) != expected) {
		return 0;
	}
	advance(lexer);
	return 1;
}

static int is_ident_start(char c)
{
	return isalpha((unsigned char)c) || c == '_';
}

static int is_ident_continue(char c)
{
	return isalnum((unsigned char)c) || c == '_';
}

static int is_keyword_text_at(const KekLexer *lexer, const char *text)
{
	size_t len = strlen(text);
	if (lexer->index + len > lexer->length) {
		return 0;
	}
	if (strncmp(&lexer->source[lexer->index], text, len) != 0) {
		return 0;
	}
	if (lexer->index + len == lexer->length) {
		return 1;
	}
	if (is_ident_continue(lexer->source[lexer->index + len])) {
		return 0;
	}
	return 1;
}

static KekToken make_token(KekTokenKind kind, const char *start, KekSourcePos pos, const KekLexer *lexer)
{
	KekToken token;
	token.kind = kind;
	token.start = start;
	token.length = (size_t)(&lexer->source[lexer->index] - start);
	token.pos = pos;
	token.message = NULL;
	return token;
}

static KekToken make_error(KekLexer *lexer, const char *start, KekSourcePos pos, const char *message)
{
	KekToken token;
	lexer->error_message = message;
	token.kind = KEK_TOKEN_ERROR;
	token.start = start;
	token.length = (size_t)(&lexer->source[lexer->index] - start);
	token.pos = pos;
	token.message = message;
	return token;
}

static void skip_line_comment(KekLexer *lexer)
{
	while (peek(lexer) != '\0' && peek(lexer) != '\n') {
		advance(lexer);
	}
}

static int skip_exclude_block(KekLexer *lexer)
{
	size_t saved_index = lexer->index;
	size_t saved_line = lexer->line;
	size_t saved_column = lexer->column;
	int depth;

	if (!is_keyword_text_at(lexer, "exclude")) {
		return 0;
	}

	while (is_ident_continue(peek(lexer))) {
		advance(lexer);
	}
	while (peek(lexer) == ' ' || peek(lexer) == '\r' || peek(lexer) == '\t' || peek(lexer) == '\n') {
		advance(lexer);
	}

	if (!match(lexer, '{')) {
		lexer->index = saved_index;
		lexer->line = saved_line;
		lexer->column = saved_column;
		return 0;
	}

	depth = 1;
	while (depth > 0 && peek(lexer) != '\0') {
		if (peek(lexer) == '{') {
			depth++;
		} else if (peek(lexer) == '}') {
			depth--;
		}
		advance(lexer);
	}

	return 1;
}

static void skip_whitespace_and_plain_comments(KekLexer *lexer)
{
	for (;;) {
		char c = peek(lexer);
		if (c == ' ' || c == '\r' || c == '\t' || c == '\n') {
			advance(lexer);
			continue;
		}

		if (c == '/' && peek_next(lexer) == '/') {
			skip_line_comment(lexer);
			continue;
		}

		if (skip_exclude_block(lexer)) {
			continue;
		}

		return;
	}
}

static KekToken identifier_or_keyword(KekLexer *lexer, const char *start, KekSourcePos pos)
{
	size_t len;
	size_t i;

	while (is_ident_continue(peek(lexer))) {
		advance(lexer);
	}

	len = (size_t)(&lexer->source[lexer->index] - start);
	for (i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
		if (strlen(keywords[i].text) == len && strncmp(start, keywords[i].text, len) == 0) {
			return make_token(keywords[i].kind, start, pos, lexer);
		}
	}

	return make_token(KEK_TOKEN_IDENTIFIER, start, pos, lexer);
}

static int digit_is_valid_for_base(char c, int base)
{
	if (base == 2) {
		return c == '0' || c == '1';
	}
	if (base == 16) {
		return isxdigit((unsigned char)c);
	}
	return isdigit((unsigned char)c);
}

static void consume_bad_numeric_tail(KekLexer *lexer)
{
	while (isalnum((unsigned char)peek(lexer)) || peek(lexer) == '_') {
		advance(lexer);
	}
}

static void consume_digit_sequence(KekLexer *lexer, int base, int *digit_count, int *bad_underscore, int prev_was_digit)
{
	while (digit_is_valid_for_base(peek(lexer), base) || peek(lexer) == '_') {
		if (peek(lexer) == '_') {
			if (!prev_was_digit) {
				*bad_underscore = 1;
			}
			prev_was_digit = 0;
		} else {
			(*digit_count)++;
			prev_was_digit = 1;
		}
		advance(lexer);
	}

	if (!prev_was_digit) {
		*bad_underscore = 1;
	}
}

static int starts_bad_numeric_tail(char c)
{
	return isalnum((unsigned char)c) || c == '_';
}

static KekToken number(KekLexer *lexer, const char *start, KekSourcePos pos)
{
	KekTokenKind kind = KEK_TOKEN_INTEGER;
	int digit_count = 1;
	int bad_underscore = 0;

	if (start[0] == '0' && (peek(lexer) == 'x' || peek(lexer) == 'X')) {
		advance(lexer);
		digit_count = 0;
		consume_digit_sequence(lexer, 16, &digit_count, &bad_underscore, 0);
		if (digit_count == 0) {
			consume_bad_numeric_tail(lexer);
			return make_error(lexer, start, pos, "expected hexadecimal digits after 0x");
		}
		if (bad_underscore) {
			consume_bad_numeric_tail(lexer);
			return make_error(lexer, start, pos, "invalid underscore in numeric literal");
		}
		if (starts_bad_numeric_tail(peek(lexer))) {
			consume_bad_numeric_tail(lexer);
			return make_error(lexer, start, pos, "invalid hexadecimal literal");
		}
		return make_token(kind, start, pos, lexer);
	}

	if (start[0] == '0' && (peek(lexer) == 'b' || peek(lexer) == 'B')) {
		advance(lexer);
		digit_count = 0;
		consume_digit_sequence(lexer, 2, &digit_count, &bad_underscore, 0);
		if (digit_count == 0) {
			consume_bad_numeric_tail(lexer);
			return make_error(lexer, start, pos, "expected binary digits after 0b");
		}
		if (bad_underscore) {
			consume_bad_numeric_tail(lexer);
			return make_error(lexer, start, pos, "invalid underscore in numeric literal");
		}
		if (starts_bad_numeric_tail(peek(lexer))) {
			consume_bad_numeric_tail(lexer);
			return make_error(lexer, start, pos, "invalid binary literal");
		}
		return make_token(kind, start, pos, lexer);
	}

	consume_digit_sequence(lexer, 10, &digit_count, &bad_underscore, 1);

	if (peek(lexer) == '.') {
		kind = KEK_TOKEN_FLOAT;
		advance(lexer);
		consume_digit_sequence(lexer, 10, &digit_count, &bad_underscore, 1);
	}

	if (peek(lexer) == 'e' || peek(lexer) == 'E') {
		int exponent_digit_count = 0;
		kind = KEK_TOKEN_FLOAT;
		advance(lexer);
		if (peek(lexer) == '+' || peek(lexer) == '-') {
			advance(lexer);
		}
		consume_digit_sequence(lexer, 10, &exponent_digit_count, &bad_underscore, 0);
		if (exponent_digit_count == 0) {
			consume_bad_numeric_tail(lexer);
			return make_error(lexer, start, pos, "expected exponent digits");
		}
	}

	if (bad_underscore) {
		consume_bad_numeric_tail(lexer);
		return make_error(lexer, start, pos, "invalid underscore in numeric literal");
	}

	if (starts_bad_numeric_tail(peek(lexer))) {
		consume_bad_numeric_tail(lexer);
		return make_error(lexer, start, pos, "invalid suffix on numeric literal");
	}

	return make_token(kind, start, pos, lexer);
}

static KekToken dot_or_float(KekLexer *lexer, const char *start, KekSourcePos pos)
{
	int digit_count = 0;
	int bad_underscore = 0;

	if (!isdigit((unsigned char)peek(lexer))) {
		return make_token(KEK_TOKEN_DOT, start, pos, lexer);
	}

	consume_digit_sequence(lexer, 10, &digit_count, &bad_underscore, 1);
	if (bad_underscore) {
		consume_bad_numeric_tail(lexer);
		return make_error(lexer, start, pos, "invalid underscore in numeric literal");
	}
	if (peek(lexer) == 'e' || peek(lexer) == 'E') {
		int exponent_digit_count = 0;
		advance(lexer);
		if (peek(lexer) == '+' || peek(lexer) == '-') {
			advance(lexer);
		}
		consume_digit_sequence(lexer, 10, &exponent_digit_count, &bad_underscore, 0);
		if (exponent_digit_count == 0) {
			consume_bad_numeric_tail(lexer);
			return make_error(lexer, start, pos, "expected exponent digits");
		}
	}
	if (starts_bad_numeric_tail(peek(lexer))) {
		consume_bad_numeric_tail(lexer);
		return make_error(lexer, start, pos, "invalid suffix on numeric literal");
	}
	return make_token(KEK_TOKEN_FLOAT, start, pos, lexer);
}

static int is_octal_digit(char c)
{
	return c >= '0' && c <= '7';
}

static int consume_c_escape(KekLexer *lexer)
{
	char c = advance(lexer);
	int count;

	if (c == '\0') {
		return 0;
	}

	switch (c) {
	case '\n':
		return 1;
	case '\'':
	case '"':
	case '?':
	case '\\':
	case 'a':
	case 'b':
	case 'f':
	case 'n':
	case 'r':
	case 't':
	case 'v':
		return 1;
	case '\r':
		if (peek(lexer) == '\n') {
			advance(lexer);
		}
		return 1;
	case 'x':
		if (!isxdigit((unsigned char)peek(lexer))) {
			return 0;
		}
		while (isxdigit((unsigned char)peek(lexer))) {
			advance(lexer);
		}
		return 1;
	default:
		if (is_octal_digit(c)) {
			for (count = 1; count < 3 && is_octal_digit(peek(lexer)); count++) {
				advance(lexer);
			}
			return 1;
		}
		return 0;
	}
}

static KekToken quoted(KekLexer *lexer, const char *start, KekSourcePos pos, char quote, KekTokenKind kind)
{
	int content_count = 0;

	while (peek(lexer) != '\0' && peek(lexer) != quote) {
		if (peek(lexer) == '\n' || peek(lexer) == '\r') {
			return make_error(lexer, start, pos, "newline in quoted literal");
		}
		if (peek(lexer) == '\\') {
			advance(lexer);
			if (!consume_c_escape(lexer)) {
				return make_error(lexer, start, pos, "invalid C escape sequence");
			}
			content_count++;
			continue;
		}
		advance(lexer);
		content_count++;
	}

	if (!match(lexer, quote)) {
		return make_error(lexer, start, pos, "unterminated quoted literal");
	}

	if (kind == KEK_TOKEN_CHAR && content_count == 0) {
		return make_error(lexer, start, pos, "empty character literal");
	}

	return make_token(kind, start, pos, lexer);
}

static KekToken shift_or_compare(KekLexer *lexer, const char *start, KekSourcePos pos, char shift_char,
	KekTokenKind single, KekTokenKind equal_kind, KekTokenKind shift_kind, KekTokenKind shift_equal_kind)
{
	if (match(lexer, '=')) {
		return make_token(equal_kind, start, pos, lexer);
	}
	if (match(lexer, shift_char)) {
		if (match(lexer, '=')) {
			return make_token(shift_equal_kind, start, pos, lexer);
		}
		return make_token(shift_kind, start, pos, lexer);
	}
	return make_token(single, start, pos, lexer);
}

void kek_lexer_init(KekLexer *lexer, const char *source_name, const char *source, size_t length)
{
	lexer->source_name = source_name;
	lexer->source = source;
	lexer->length = length;
	lexer->index = 0;
	lexer->line = 1;
	lexer->column = 1;
	lexer->error_message = NULL;
}

KekToken kek_lexer_next(KekLexer *lexer)
{
	KekSourcePos pos;
	const char *start;
	char c;

	skip_whitespace_and_plain_comments(lexer);

	pos.offset = lexer->index;
	pos.line = lexer->line;
	pos.column = lexer->column;
	start = &lexer->source[lexer->index];

	if (at_end(lexer)) {
		return make_token(KEK_TOKEN_EOF, start, pos, lexer);
	}

	c = advance(lexer);

	if (is_ident_start(c)) {
		return identifier_or_keyword(lexer, start, pos);
	}

	if (isdigit((unsigned char)c)) {
		return number(lexer, start, pos);
	}

	switch (c) {
	case '"':
		return quoted(lexer, start, pos, '"', KEK_TOKEN_STRING);
	case '\'':
		return quoted(lexer, start, pos, '\'', KEK_TOKEN_CHAR);
	case '/':
		if (match(lexer, '=')) {
			return make_token(KEK_TOKEN_SLASH_EQUAL, start, pos, lexer);
		}
		return make_token(KEK_TOKEN_SLASH, start, pos, lexer);
	case '(':
		return make_token(KEK_TOKEN_LPAREN, start, pos, lexer);
	case ')':
		return make_token(KEK_TOKEN_RPAREN, start, pos, lexer);
	case '{':
		return make_token(KEK_TOKEN_LBRACE, start, pos, lexer);
	case '}':
		return make_token(KEK_TOKEN_RBRACE, start, pos, lexer);
	case '[':
		return make_token(KEK_TOKEN_LBRACKET, start, pos, lexer);
	case ']':
		return make_token(KEK_TOKEN_RBRACKET, start, pos, lexer);
	case ',':
		return make_token(KEK_TOKEN_COMMA, start, pos, lexer);
	case '.':
		return dot_or_float(lexer, start, pos);
	case ';':
		return make_token(KEK_TOKEN_SEMICOLON, start, pos, lexer);
	case ':':
		if (match(lexer, ':')) {
			return make_token(KEK_TOKEN_COLON_COLON, start, pos, lexer);
		}
		return make_token(KEK_TOKEN_COLON, start, pos, lexer);
	case '#':
		return make_token(KEK_TOKEN_HASH, start, pos, lexer);
	case '@':
		return make_token(KEK_TOKEN_AT, start, pos, lexer);
	case '+':
		if (match(lexer, '+')) {
			return make_token(KEK_TOKEN_PLUS_PLUS, start, pos, lexer);
		}
		if (match(lexer, '=')) {
			return make_token(KEK_TOKEN_PLUS_EQUAL, start, pos, lexer);
		}
		return make_token(KEK_TOKEN_PLUS, start, pos, lexer);
	case '-':
		if (match(lexer, '>')) {
			return make_token(KEK_TOKEN_ARROW, start, pos, lexer);
		}
		if (match(lexer, '-')) {
			return make_token(KEK_TOKEN_MINUS_MINUS, start, pos, lexer);
		}
		if (match(lexer, '=')) {
			return make_token(KEK_TOKEN_MINUS_EQUAL, start, pos, lexer);
		}
		return make_token(KEK_TOKEN_MINUS, start, pos, lexer);
	case '*':
		if (match(lexer, '=')) {
			return make_token(KEK_TOKEN_STAR_EQUAL, start, pos, lexer);
		}
		return make_token(KEK_TOKEN_STAR, start, pos, lexer);
	case '%':
		if (match(lexer, '=')) {
			return make_token(KEK_TOKEN_PERCENT_EQUAL, start, pos, lexer);
		}
		return make_token(KEK_TOKEN_PERCENT, start, pos, lexer);
	case '&':
		if (match(lexer, '&')) {
			return make_token(KEK_TOKEN_AMP_AMP, start, pos, lexer);
		}
		if (match(lexer, '=')) {
			return make_token(KEK_TOKEN_AMP_EQUAL, start, pos, lexer);
		}
		return make_token(KEK_TOKEN_AMP, start, pos, lexer);
	case '|':
		if (match(lexer, '|')) {
			return make_token(KEK_TOKEN_PIPE_PIPE, start, pos, lexer);
		}
		if (match(lexer, '=')) {
			return make_token(KEK_TOKEN_PIPE_EQUAL, start, pos, lexer);
		}
		return make_token(KEK_TOKEN_PIPE, start, pos, lexer);
	case '^':
		if (match(lexer, '=')) {
			return make_token(KEK_TOKEN_CARET_EQUAL, start, pos, lexer);
		}
		return make_token(KEK_TOKEN_CARET, start, pos, lexer);
	case '~':
		return make_token(KEK_TOKEN_TILDE, start, pos, lexer);
	case '!':
		if (match(lexer, '=')) {
			return make_token(KEK_TOKEN_BANG_EQUAL, start, pos, lexer);
		}
		return make_token(KEK_TOKEN_BANG, start, pos, lexer);
	case '?':
		return make_token(KEK_TOKEN_QUESTION, start, pos, lexer);
	case '=':
		if (match(lexer, '=')) {
			return make_token(KEK_TOKEN_EQUAL_EQUAL, start, pos, lexer);
		}
		return make_token(KEK_TOKEN_EQUAL, start, pos, lexer);
	case '<':
		return shift_or_compare(lexer, start, pos, '<', KEK_TOKEN_LESS, KEK_TOKEN_LESS_EQUAL,
			KEK_TOKEN_LESS_LESS,
			KEK_TOKEN_LESS_LESS_EQUAL);
	case '>':
		return shift_or_compare(lexer, start, pos, '>', KEK_TOKEN_GREATER, KEK_TOKEN_GREATER_EQUAL,
			KEK_TOKEN_GREATER_GREATER,
			KEK_TOKEN_GREATER_GREATER_EQUAL);
	default:
		return make_error(lexer, start, pos, "unexpected character");
	}
}

const char *kek_token_kind_name(KekTokenKind kind)
{
	switch (kind) {
	case KEK_TOKEN_EOF: return "EOF";
	case KEK_TOKEN_ERROR: return "ERROR";
	case KEK_TOKEN_IDENTIFIER: return "IDENTIFIER";
	case KEK_TOKEN_INTEGER: return "INTEGER";
	case KEK_TOKEN_FLOAT: return "FLOAT";
	case KEK_TOKEN_STRING: return "STRING";
	case KEK_TOKEN_CHAR: return "CHAR";
	case KEK_TOKEN_KW_ALIAS: return "KW_ALIAS";
	case KEK_TOKEN_KW_ALIGNOF: return "KW_ALIGNOF";
	case KEK_TOKEN_KW_ASSERT: return "KW_ASSERT";
	case KEK_TOKEN_KW_BREAK: return "KW_BREAK";
	case KEK_TOKEN_KW_CASE: return "KW_CASE";
	case KEK_TOKEN_KW_CAST: return "KW_CAST";
	case KEK_TOKEN_KW_CONTINUE: return "KW_CONTINUE";
	case KEK_TOKEN_KW_DEFAULT: return "KW_DEFAULT";
	case KEK_TOKEN_KW_DO: return "KW_DO";
	case KEK_TOKEN_KW_ELSE: return "KW_ELSE";
	case KEK_TOKEN_KW_ENUM: return "KW_ENUM";
	case KEK_TOKEN_KW_EXPORT: return "KW_EXPORT";
	case KEK_TOKEN_KW_EXTERN: return "KW_EXTERN";
	case KEK_TOKEN_KW_FALSE: return "KW_FALSE";
	case KEK_TOKEN_KW_FOR: return "KW_FOR";
	case KEK_TOKEN_KW_IF: return "KW_IF";
	case KEK_TOKEN_KW_IN: return "KW_IN";
	case KEK_TOKEN_KW_LEN: return "KW_LEN";
	case KEK_TOKEN_KW_OFFSETOF: return "KW_OFFSETOF";
	case KEK_TOKEN_KW_PACKED: return "KW_PACKED";
	case KEK_TOKEN_KW_PANIC: return "KW_PANIC";
	case KEK_TOKEN_KW_RETURN: return "KW_RETURN";
	case KEK_TOKEN_KW_SIZEOF: return "KW_SIZEOF";
	case KEK_TOKEN_KW_STRUCT: return "KW_STRUCT";
	case KEK_TOKEN_KW_SWITCH: return "KW_SWITCH";
	case KEK_TOKEN_KW_THIS: return "KW_THIS";
	case KEK_TOKEN_KW_TRUE: return "KW_TRUE";
	case KEK_TOKEN_KW_UNION: return "KW_UNION";
	case KEK_TOKEN_KW_UNREACHABLE: return "KW_UNREACHABLE";
	case KEK_TOKEN_KW_USING: return "KW_USING";
	case KEK_TOKEN_KW_WHILE: return "KW_WHILE";
	case KEK_TOKEN_LPAREN: return "LPAREN";
	case KEK_TOKEN_RPAREN: return "RPAREN";
	case KEK_TOKEN_LBRACE: return "LBRACE";
	case KEK_TOKEN_RBRACE: return "RBRACE";
	case KEK_TOKEN_LBRACKET: return "LBRACKET";
	case KEK_TOKEN_RBRACKET: return "RBRACKET";
	case KEK_TOKEN_COMMA: return "COMMA";
	case KEK_TOKEN_DOT: return "DOT";
	case KEK_TOKEN_SEMICOLON: return "SEMICOLON";
	case KEK_TOKEN_COLON: return "COLON";
	case KEK_TOKEN_COLON_COLON: return "COLON_COLON";
	case KEK_TOKEN_HASH: return "HASH";
	case KEK_TOKEN_AT: return "AT";
	case KEK_TOKEN_ARROW: return "ARROW";
	case KEK_TOKEN_PLUS: return "PLUS";
	case KEK_TOKEN_MINUS: return "MINUS";
	case KEK_TOKEN_STAR: return "STAR";
	case KEK_TOKEN_SLASH: return "SLASH";
	case KEK_TOKEN_PERCENT: return "PERCENT";
	case KEK_TOKEN_AMP: return "AMP";
	case KEK_TOKEN_PIPE: return "PIPE";
	case KEK_TOKEN_CARET: return "CARET";
	case KEK_TOKEN_TILDE: return "TILDE";
	case KEK_TOKEN_BANG: return "BANG";
	case KEK_TOKEN_QUESTION: return "QUESTION";
	case KEK_TOKEN_EQUAL: return "EQUAL";
	case KEK_TOKEN_LESS: return "LESS";
	case KEK_TOKEN_GREATER: return "GREATER";
	case KEK_TOKEN_PLUS_PLUS: return "PLUS_PLUS";
	case KEK_TOKEN_MINUS_MINUS: return "MINUS_MINUS";
	case KEK_TOKEN_PLUS_EQUAL: return "PLUS_EQUAL";
	case KEK_TOKEN_MINUS_EQUAL: return "MINUS_EQUAL";
	case KEK_TOKEN_STAR_EQUAL: return "STAR_EQUAL";
	case KEK_TOKEN_SLASH_EQUAL: return "SLASH_EQUAL";
	case KEK_TOKEN_PERCENT_EQUAL: return "PERCENT_EQUAL";
	case KEK_TOKEN_AMP_EQUAL: return "AMP_EQUAL";
	case KEK_TOKEN_PIPE_EQUAL: return "PIPE_EQUAL";
	case KEK_TOKEN_CARET_EQUAL: return "CARET_EQUAL";
	case KEK_TOKEN_EQUAL_EQUAL: return "EQUAL_EQUAL";
	case KEK_TOKEN_BANG_EQUAL: return "BANG_EQUAL";
	case KEK_TOKEN_LESS_EQUAL: return "LESS_EQUAL";
	case KEK_TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
	case KEK_TOKEN_LESS_LESS: return "LESS_LESS";
	case KEK_TOKEN_GREATER_GREATER: return "GREATER_GREATER";
	case KEK_TOKEN_LESS_LESS_EQUAL: return "LESS_LESS_EQUAL";
	case KEK_TOKEN_GREATER_GREATER_EQUAL: return "GREATER_GREATER_EQUAL";
	case KEK_TOKEN_AMP_AMP: return "AMP_AMP";
	case KEK_TOKEN_PIPE_PIPE: return "PIPE_PIPE";
	}
	return "UNKNOWN";
}

int kek_token_is_keyword(KekTokenKind kind)
{
	return kind >= KEK_TOKEN_KW_ALIAS && kind <= KEK_TOKEN_KW_WHILE;
}
