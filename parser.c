#include "parser.h"

static int tok_is_kw(const Token *t, const char *kw) {
    if (t->type != TOK_KEYWORD) return 0;
    int i;
    for (i = 0; i < t->length; i++) {
        char a = t->text[i];
        char b = kw[i];
        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (b == 0) return 0;
        if (a != b) return 0;
    }
    return kw[t->length] == 0;
}

static void copy_ident(char *dst, int dst_size, const Token *t) {
    int n = t->length;
    if (n > dst_size - 1) n = dst_size - 1;
    int i;
    for (i = 0; i < n; i++) dst[i] = t->text[i];
    dst[n] = '\0';
}

static void copy_value(Value *v, const Token *t) {
    int n = t->length;
    if (n > MAX_VALUE_LEN - 1) n = MAX_VALUE_LEN - 1;
    int i;
    for (i = 0; i < n; i++) v->text[i] = t->text[i];
    v->text[n] = '\0';
    v->is_number = (t->type == TOK_NUMBER);
}

static ParseResult parse_ident_list(const Token *tokens, int *pos, int token_count,
                                     char out_names[][MAX_IDENT_LEN], int *out_count) {
    if (tokens[*pos].type != TOK_LPAREN) return PARSE_ERROR_SYNTAX;
    (*pos)++;

    int count = 0;
    for (;;) {
        if (*pos >= token_count) return PARSE_ERROR_SYNTAX;
        if (tokens[*pos].type != TOK_IDENT) return PARSE_ERROR_SYNTAX;
        if (count >= MAX_COLUMNS) return PARSE_ERROR_TOO_MANY_COLUMNS;

        copy_ident(out_names[count], MAX_IDENT_LEN, &tokens[*pos]);
        count++;
        (*pos)++;

        if (tokens[*pos].type == TOK_COMMA) { (*pos)++; continue; }
        if (tokens[*pos].type == TOK_RPAREN) { (*pos)++; break; }
        return PARSE_ERROR_SYNTAX;
    }

    *out_count = count;
    return PARSE_SUCCESS;
}

static ParseResult parse_value_list(const Token *tokens, int *pos, int token_count,
                                     Value out_values[], int *out_count) {
    if (tokens[*pos].type != TOK_LPAREN) return PARSE_ERROR_SYNTAX;
    (*pos)++;

    int count = 0;
    for (;;) {
        if (*pos >= token_count) return PARSE_ERROR_SYNTAX;
        TokenType tt = tokens[*pos].type;
        if (tt != TOK_NUMBER && tt != TOK_STRING) return PARSE_ERROR_SYNTAX;
        if (count >= MAX_COLUMNS) return PARSE_ERROR_TOO_MANY_COLUMNS;

        copy_value(&out_values[count], &tokens[*pos]);
        count++;
        (*pos)++;

        if (tokens[*pos].type == TOK_COMMA) { (*pos)++; continue; }
        if (tokens[*pos].type == TOK_RPAREN) { (*pos)++; break; }
        return PARSE_ERROR_SYNTAX;
    }

    *out_count = count;
    return PARSE_SUCCESS;
}

static ParseResult parse_insert(const Token *tokens, int pos, int token_count, Statement *stmt) {
    stmt->type = STMT_INSERT;

    if (!tok_is_kw(&tokens[pos], "into")) return PARSE_ERROR_SYNTAX;
    pos++;

    if (tokens[pos].type != TOK_IDENT) return PARSE_ERROR_SYNTAX;
    copy_ident(stmt->table, MAX_IDENT_LEN, &tokens[pos]);
    pos++;

    ParseResult r = parse_ident_list(tokens, &pos, token_count, stmt->columns, &stmt->column_count);
    if (r != PARSE_SUCCESS) return r;

    if (!tok_is_kw(&tokens[pos], "values")) return PARSE_ERROR_SYNTAX;
    pos++;

    r = parse_value_list(tokens, &pos, token_count, stmt->values, &stmt->value_count);
    if (r != PARSE_SUCCESS) return r;

    if (stmt->value_count != stmt->column_count) return PARSE_ERROR_SYNTAX;

    return PARSE_SUCCESS;
}

static ParseResult parse_select(const Token *tokens, int pos, int token_count, Statement *stmt) {
    stmt->type = STMT_SELECT;
    stmt->where_op = OP_NONE;

    if (tokens[pos].type == TOK_STAR) {
        stmt->column_count = -1;
        pos++;
    } else {
        int count = 0;
        for (;;) {
            if (tokens[pos].type != TOK_IDENT) return PARSE_ERROR_SYNTAX;
            if (count >= MAX_COLUMNS) return PARSE_ERROR_TOO_MANY_COLUMNS;
            copy_ident(stmt->columns[count], MAX_IDENT_LEN, &tokens[pos]);
            count++;
            pos++;
            if (tokens[pos].type == TOK_COMMA) { pos++; continue; }
            break;
        }
        stmt->column_count = count;
    }

    if (!tok_is_kw(&tokens[pos], "from")) return PARSE_ERROR_SYNTAX;
    pos++;

    if (tokens[pos].type != TOK_IDENT) return PARSE_ERROR_SYNTAX;
    copy_ident(stmt->table, MAX_IDENT_LEN, &tokens[pos]);
    pos++;

    if (tok_is_kw(&tokens[pos], "where")) {
        pos++;
        if (tokens[pos].type != TOK_IDENT) return PARSE_ERROR_SYNTAX;
        copy_ident(stmt->where_column, MAX_IDENT_LEN, &tokens[pos]);
        pos++;

        TokenType op = tokens[pos].type;
        if (op == TOK_EQ) stmt->where_op = OP_EQ;
        else if (op == TOK_LT) stmt->where_op = OP_LT;
        else if (op == TOK_GT) stmt->where_op = OP_GT;
        else return PARSE_ERROR_SYNTAX;
        pos++;

        TokenType vt = tokens[pos].type;
        if (vt != TOK_NUMBER && vt != TOK_STRING) return PARSE_ERROR_SYNTAX;
        copy_value(&stmt->where_value, &tokens[pos]);
        pos++;
    }

    return PARSE_SUCCESS;
}

ParseResult parse_statement(const Token *tokens, int token_count, Statement *out_stmt) {
    if (token_count == 0 || tokens[0].type == TOK_EOF) return PARSE_ERROR_SYNTAX;

    if (tok_is_kw(&tokens[0], "insert")) {
        return parse_insert(tokens, 1, token_count, out_stmt);
    }
    if (tok_is_kw(&tokens[0], "select")) {
        return parse_select(tokens, 1, token_count, out_stmt);
    }

    return PARSE_ERROR_UNSUPPORTED;
}