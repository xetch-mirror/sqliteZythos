#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

#define MAX_COLUMNS 16
#define MAX_IDENT_LEN 32
#define MAX_VALUE_LEN 256

typedef enum { STMT_INSERT, STMT_SELECT } StatementType;
typedef enum { OP_EQ, OP_LT, OP_GT, OP_NONE } CompareOp;

typedef struct {
    int is_number;
    char text[MAX_VALUE_LEN];
} Value;

typedef struct {
    StatementType type;
    char table[MAX_IDENT_LEN];

    int column_count;
    char columns[MAX_COLUMNS][MAX_IDENT_LEN];

    int value_count;
    Value values[MAX_COLUMNS];

    CompareOp where_op;
    char where_column[MAX_IDENT_LEN];
    Value where_value;
} Statement;

typedef enum {
    PARSE_SUCCESS,
    PARSE_ERROR_SYNTAX,
    PARSE_ERROR_UNSUPPORTED,
    PARSE_ERROR_TOO_MANY_COLUMNS
} ParseResult;

ParseResult parse_statement(const Token *tokens, int token_count, Statement *out_stmt);

#endif