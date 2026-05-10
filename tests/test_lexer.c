#include "kek/lexer.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_kind(KekLexer *lexer, KekTokenKind expected, const char *text)
{
	KekToken token = kek_lexer_next(lexer);
	if (token.kind != expected) {
		fprintf(stderr, "expected %s, got %s at %zu:%zu\n",
			kek_token_kind_name(expected), kek_token_kind_name(token.kind), token.pos.line, token.pos.column);
		failures++;
		return;
	}
	if (text != NULL && (strlen(text) != token.length || strncmp(token.start, text, token.length) != 0)) {
		fprintf(stderr, "expected text `%s`, got `%.*s`\n", text, (int)token.length, token.start);
		failures++;
	}
}

static void expect_error_message(const char *source, const char *text, const char *message)
{
	KekLexer lexer;
	KekToken token;
	kek_lexer_init(&lexer, "<test>", source, strlen(source));

	token = kek_lexer_next(&lexer);
	if (token.kind != KEK_TOKEN_ERROR) {
		fprintf(stderr, "expected ERROR for `%s`, got %s\n", source, kek_token_kind_name(token.kind));
		failures++;
		return;
	}
	if (strlen(text) != token.length || strncmp(token.start, text, token.length) != 0) {
		fprintf(stderr, "expected error text `%s`, got `%.*s`\n", text, (int)token.length, token.start);
		failures++;
	}
	if (token.message == NULL || strcmp(token.message, message) != 0) {
		fprintf(stderr, "expected error message `%s`, got `%s`\n",
			message, token.message == NULL ? "<null>" : token.message);
		failures++;
	}
}

static void test_basic_declaration(void)
{
	const char *source = "export i32 Add(i32 v1, i32 v2) { return v1 + v2; }\n";
	KekLexer lexer;
	kek_lexer_init(&lexer, "<test>", source, strlen(source));

	expect_kind(&lexer, KEK_TOKEN_KW_EXPORT, "export");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "i32");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "Add");
	expect_kind(&lexer, KEK_TOKEN_LPAREN, "(");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "i32");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "v1");
	expect_kind(&lexer, KEK_TOKEN_COMMA, ",");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "i32");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "v2");
	expect_kind(&lexer, KEK_TOKEN_RPAREN, ")");
	expect_kind(&lexer, KEK_TOKEN_LBRACE, "{");
	expect_kind(&lexer, KEK_TOKEN_KW_RETURN, "return");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "v1");
	expect_kind(&lexer, KEK_TOKEN_PLUS, "+");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "v2");
	expect_kind(&lexer, KEK_TOKEN_SEMICOLON, ";");
	expect_kind(&lexer, KEK_TOKEN_RBRACE, "}");
	expect_kind(&lexer, KEK_TOKEN_EOF, NULL);
}

static void test_numbers_and_operators(void)
{
	const char *source = "u8 mask = 0b1010_0001; u32 color = 0xFF_AA_00_12; x += 1.5e-2 + .5 + 5.;";
	KekLexer lexer;
	kek_lexer_init(&lexer, "<test>", source, strlen(source));

	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "u8");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "mask");
	expect_kind(&lexer, KEK_TOKEN_EQUAL, "=");
	expect_kind(&lexer, KEK_TOKEN_INTEGER, "0b1010_0001");
	expect_kind(&lexer, KEK_TOKEN_SEMICOLON, ";");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "u32");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "color");
	expect_kind(&lexer, KEK_TOKEN_EQUAL, "=");
	expect_kind(&lexer, KEK_TOKEN_INTEGER, "0xFF_AA_00_12");
	expect_kind(&lexer, KEK_TOKEN_SEMICOLON, ";");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "x");
	expect_kind(&lexer, KEK_TOKEN_PLUS_EQUAL, "+=");
	expect_kind(&lexer, KEK_TOKEN_FLOAT, "1.5e-2");
	expect_kind(&lexer, KEK_TOKEN_PLUS, "+");
	expect_kind(&lexer, KEK_TOKEN_FLOAT, ".5");
	expect_kind(&lexer, KEK_TOKEN_PLUS, "+");
	expect_kind(&lexer, KEK_TOKEN_FLOAT, "5.");
	expect_kind(&lexer, KEK_TOKEN_SEMICOLON, ";");
	expect_kind(&lexer, KEK_TOKEN_EOF, NULL);
}

static void test_docs_and_comments(void)
{
	const char *source = "// plain\n/// doc line\nalias RawHandle = ptr;\n";
	KekLexer lexer;
	kek_lexer_init(&lexer, "<test>", source, strlen(source));

	expect_kind(&lexer, KEK_TOKEN_KW_ALIAS, "alias");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "RawHandle");
	expect_kind(&lexer, KEK_TOKEN_EQUAL, "=");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "ptr");
	expect_kind(&lexer, KEK_TOKEN_SEMICOLON, ";");
	expect_kind(&lexer, KEK_TOKEN_EOF, NULL);
}

static void test_exclude_block(void)
{
	const char *source = "alias A = u8; exclude { this is not lexed { 123_bad } } alias B = u16;";
	KekLexer lexer;
	kek_lexer_init(&lexer, "<test>", source, strlen(source));

	expect_kind(&lexer, KEK_TOKEN_KW_ALIAS, "alias");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "A");
	expect_kind(&lexer, KEK_TOKEN_EQUAL, "=");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "u8");
	expect_kind(&lexer, KEK_TOKEN_SEMICOLON, ";");
	expect_kind(&lexer, KEK_TOKEN_KW_ALIAS, "alias");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "B");
	expect_kind(&lexer, KEK_TOKEN_EQUAL, "=");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "u16");
	expect_kind(&lexer, KEK_TOKEN_SEMICOLON, ";");
	expect_kind(&lexer, KEK_TOKEN_EOF, NULL);
}

static void test_block_comments_are_not_comments(void)
{
	const char *source = "/* nope */";
	KekLexer lexer;
	kek_lexer_init(&lexer, "<test>", source, strlen(source));

	expect_kind(&lexer, KEK_TOKEN_SLASH, "/");
	expect_kind(&lexer, KEK_TOKEN_STAR, "*");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "nope");
	expect_kind(&lexer, KEK_TOKEN_STAR, "*");
	expect_kind(&lexer, KEK_TOKEN_SLASH, "/");
	expect_kind(&lexer, KEK_TOKEN_EOF, NULL);
}

static void test_namespace_and_positions(void)
{
	const char *source = "\nMyPackage::MyPackageFunc();\n";
	KekLexer lexer;
	KekToken token;
	kek_lexer_init(&lexer, "<test>", source, strlen(source));

	token = kek_lexer_next(&lexer);
	if (token.kind != KEK_TOKEN_IDENTIFIER || token.pos.line != 2 || token.pos.column != 1) {
		fprintf(stderr, "bad first token position or kind\n");
		failures++;
	}
	expect_kind(&lexer, KEK_TOKEN_COLON_COLON, "::");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "MyPackageFunc");
	expect_kind(&lexer, KEK_TOKEN_LPAREN, "(");
	expect_kind(&lexer, KEK_TOKEN_RPAREN, ")");
	expect_kind(&lexer, KEK_TOKEN_SEMICOLON, ";");
	expect_kind(&lexer, KEK_TOKEN_EOF, NULL);
}

static void test_comparison_and_shift_operators(void)
{
	const char *source = "a <= b >= c << 1 >> 2 <<= 3 >>= 4;";
	KekLexer lexer;
	kek_lexer_init(&lexer, "<test>", source, strlen(source));

	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "a");
	expect_kind(&lexer, KEK_TOKEN_LESS_EQUAL, "<=");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "b");
	expect_kind(&lexer, KEK_TOKEN_GREATER_EQUAL, ">=");
	expect_kind(&lexer, KEK_TOKEN_IDENTIFIER, "c");
	expect_kind(&lexer, KEK_TOKEN_LESS_LESS, "<<");
	expect_kind(&lexer, KEK_TOKEN_INTEGER, "1");
	expect_kind(&lexer, KEK_TOKEN_GREATER_GREATER, ">>");
	expect_kind(&lexer, KEK_TOKEN_INTEGER, "2");
	expect_kind(&lexer, KEK_TOKEN_LESS_LESS_EQUAL, "<<=");
	expect_kind(&lexer, KEK_TOKEN_INTEGER, "3");
	expect_kind(&lexer, KEK_TOKEN_GREATER_GREATER_EQUAL, ">>=");
	expect_kind(&lexer, KEK_TOKEN_INTEGER, "4");
	expect_kind(&lexer, KEK_TOKEN_SEMICOLON, ";");
	expect_kind(&lexer, KEK_TOKEN_EOF, NULL);
}

static void test_bad_numeric_literals(void)
{
	expect_error_message("534r3ojn32", "534r3ojn32", "invalid suffix on numeric literal");
	expect_error_message("123abc", "123abc", "invalid suffix on numeric literal");
	expect_error_message("123_", "123_", "invalid underscore in numeric literal");
	expect_error_message("1__000", "1__000", "invalid underscore in numeric literal");
	expect_error_message("1_e2", "1_e2", "invalid underscore in numeric literal");
	expect_error_message("1.2_", "1.2_", "invalid underscore in numeric literal");
	expect_error_message("1._", "1._", "invalid underscore in numeric literal");
	expect_error_message(".5_", ".5_", "invalid underscore in numeric literal");
	expect_error_message(".5abc", ".5abc", "invalid suffix on numeric literal");
	expect_error_message("1e", "1e", "expected exponent digits");
	expect_error_message("1e+", "1e+", "expected exponent digits");
	expect_error_message("1e-", "1e-", "expected exponent digits");
	expect_error_message("1.0e_name", "1.0e_name", "expected exponent digits");
	expect_error_message("0b", "0b", "expected binary digits after 0b");
	expect_error_message("0b_", "0b_", "expected binary digits after 0b");
	expect_error_message("0b102", "0b102", "invalid binary literal");
	expect_error_message("0b10_2", "0b10_2", "invalid underscore in numeric literal");
	expect_error_message("0b10abc", "0b10abc", "invalid binary literal");
	expect_error_message("0x", "0x", "expected hexadecimal digits after 0x");
	expect_error_message("0x_", "0x_", "expected hexadecimal digits after 0x");
	expect_error_message("0x_FF", "0x_FF", "invalid underscore in numeric literal");
	expect_error_message("0xFF_", "0xFF_", "invalid underscore in numeric literal");
	expect_error_message("0xFF__AA", "0xFF__AA", "invalid underscore in numeric literal");
	expect_error_message("0xG", "0xG", "expected hexadecimal digits after 0x");
	expect_error_message("0x1G", "0x1G", "invalid hexadecimal literal");
}

static void test_c_strings(void)
{
	KekLexer lexer;
	const char *source = "\"hello\\n\\x21\" '\\101' \"line\\\ncontinued\"";

	kek_lexer_init(&lexer, "<test>", source, strlen(source));
	expect_kind(&lexer, KEK_TOKEN_STRING, "\"hello\\n\\x21\"");
	expect_kind(&lexer, KEK_TOKEN_CHAR, "'\\101'");
	expect_kind(&lexer, KEK_TOKEN_STRING, "\"line\\\ncontinued\"");
	expect_kind(&lexer, KEK_TOKEN_EOF, NULL);

	expect_error_message("\"bad\\q\"", "\"bad\\q", "invalid C escape sequence");
	expect_error_message("\"bad\\x\"", "\"bad\\x", "invalid C escape sequence");
	expect_error_message("\"bad\\8\"", "\"bad\\8", "invalid C escape sequence");
	expect_error_message("\"bad\\u1234\"", "\"bad\\u", "invalid C escape sequence");
	expect_error_message("\"bad\\", "\"bad\\", "invalid C escape sequence");
	expect_error_message("\"bad\n\"", "\"bad", "newline in quoted literal");
	expect_error_message("\"bad\r\n\"", "\"bad", "newline in quoted literal");
	expect_error_message("\"unterminated", "\"unterminated", "unterminated quoted literal");
	expect_error_message("'bad\n'", "'bad", "newline in quoted literal");
	expect_error_message("'\\x'", "'\\x", "invalid C escape sequence");
	expect_error_message("''", "''", "empty character literal");
}

static void test_unexpected_character_errors(void)
{
	expect_error_message("$", "$", "unexpected character");
	expect_error_message("`", "`", "unexpected character");
	expect_error_message("\\", "\\", "unexpected character");
}

int main(void)
{
	test_basic_declaration();
	test_numbers_and_operators();
	test_docs_and_comments();
	test_exclude_block();
	test_block_comments_are_not_comments();
	test_namespace_and_positions();
	test_comparison_and_shift_operators();
	test_bad_numeric_literals();
	test_c_strings();
	test_unexpected_character_errors();

	if (failures != 0) {
		fprintf(stderr, "%d lexer test failure(s)\n", failures);
		return 1;
	}

	printf("lexer tests passed\n");
	return 0;
}
