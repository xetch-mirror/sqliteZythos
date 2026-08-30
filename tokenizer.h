#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef enum {
    TOK_EOF = 0,
    TOK_KEYWORD,
    TOK_IDENT,
    TOK_NUMBER,
    TOK_STRING,
    TOK_STAR,
    TOK_COMMA,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_EQ,
    TOK_LT,
    TOK_GT,
    TOK_SEMI,
    TOK_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    const char *text;
    int length;
} Token;

#define MAX_TOKENS 128

int tokenize(const char *input, Token *out_tokens, int max_tokens);

#endif