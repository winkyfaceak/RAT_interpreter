#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef enum{
    TOK_NUMBER, TOK_IDENTIFIER, TOK_STRING,
    TOK_ASSIGN, TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_PRINT, TOK_IF, TOK_ELSE, TOK_WHILE, TOK_FUNC, TOK_RETURN,
    TOK_SEMICOLON, TOK_COMMA,
    TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LTE, TOK_GTE,
    TOK_EOF, TOK_UNKNOWN
} TokenType;

typedef static {

}