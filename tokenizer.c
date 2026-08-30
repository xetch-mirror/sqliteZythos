#include "tokenizer.h"

static int is_space(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
static int is_digit(char c) { return c >= '0' && c <= '9'; }
static int is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
static int is_alnum(char c) { return is_alpha(c) || is_digit(c); }

static int matches_keyword(const char *text, int length, const char *kw) {
    int i;
    for (i = 0; i < length; i++) {
        char a = text[i];
        char b = kw[i];
        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (b == 0) return 0;
        if (a != b) return 0;
    }
    return kw[length] == 0;
}

static const char *KEYWORDS[] = { "insert", "into", "values", "select", "from", "where", 0 };

static int is_keyword(const char *text, int length) {
    int i;
    for (i = 0; KEYWORDS[i] != 0; i++) {
        if (matches_keyword(text, length, KEYWORDS[i])) return 1;
    }
    return 0;
}

int tokenize(const char *input, Token *out_tokens, int max_tokens) {
    int count = 0;
    const char *p = input;

    while (*p != '\0') {
        if (is_space(*p)) { p++; continue; }
        if (count >= max_tokens - 1) break;

        char c = *p;

        if (c == '*') { out_tokens[count].type = TOK_STAR;   out_tokens[count].text = p; out_tokens[count].length = 1; count++; p++; continue; }
        if (c == ',') { out_tokens[count].type = TOK_COMMA;  out_tokens[count].text = p; out_tokens[count].length = 1; count++; p++; continue; }
        if (c == '(') { out_tokens[count].type = TOK_LPAREN; out_tokens[count].text = p; out_tokens[count].length = 1; count++; p++; continue; }
        if (c == ')') { out_tokens[count].type = TOK_RPAREN; out_tokens[count].text = p; out_tokens[count].length = 1; count++; p++; continue; }
        if (c == '=') { out_tokens[count].type = TOK_EQ;     out_tokens[count].text = p; out_tokens[count].length = 1; count++; p++; continue; }
        if (c == '<') { out_tokens[count].type = TOK_LT;     out_tokens[count].text = p; out_tokens[count].length = 1; count++; p++; continue; }
        if (c == '>') { out_tokens[count].type = TOK_GT;     out_tokens[count].text = p; out_tokens[count].length = 1; count++; p++; continue; }
        if (c == ';') { out_tokens[count].type = TOK_SEMI;   out_tokens[count].text = p; out_tokens[count].length = 1; count++; p++; continue; }

        if (c == '\'' || c == '"') {
            char quote = c;
            const char *start = p + 1;
            const char *q = start;
            while (*q != '\0' && *q != quote) q++;
            if (*q == '\0') return -1;
            out_tokens[count].type = TOK_STRING;
            out_tokens[count].text = start;
            out_tokens[count].length = (int)(q - start);
            count++;
            p = q + 1;
            continue;
        }

        if (is_digit(c)) {
            const char *start = p;
            while (is_digit(*p)) p++;
            out_tokens[count].type = TOK_NUMBER;
            out_tokens[count].text = start;
            out_tokens[count].length = (int)(p - start);
            count++;
            continue;
        }

        if (is_alpha(c)) {
            const char *start = p;
            while (is_alnum(*p)) p++;
            int length = (int)(p - start);
            out_tokens[count].type = is_keyword(start, length) ? TOK_KEYWORD : TOK_IDENT;
            out_tokens[count].text = start;
            out_tokens[count].length = length;
            count++;
            continue;
        }

        out_tokens[count].type = TOK_UNKNOWN;
        out_tokens[count].text = p;
        out_tokens[count].length = 1;
        count++;
        p++;
    }

    if (count < max_tokens) {
        out_tokens[count].type = TOK_EOF;
        out_tokens[count].text = p;
        out_tokens[count].length = 0;
    }

    return count;
}