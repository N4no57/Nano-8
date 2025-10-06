//
// Created by brego on 27/09/2025.
//

#ifndef TOKENISER_H
#define TOKENISER_H

#include <stdint.h>

#define MAX_LINE_LENGTH 1024

typedef enum {
    TOKEN_MNEMONIC,
    TOKEN_REGISTER,
    TOKEN_NUMBER,
    TOKEN_LABEL,
    TOKEN_SYMBOL,
    TOKEN_DIRECTIVE,
    TOKEN_STRING,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    union {
        char *str_val;
        uint64_t int_value;
    };
} Token;

typedef struct {
    Token *data;
    size_t count;
    size_t capacity;
} TokenList;

void initTokenList(TokenList *token_list);
void token_list_push(TokenList *token_list, Token T);
void freeTokenList(const TokenList *token_list);

TokenList tokenise(char **lines);

int matches(Token tok, TokenType type, const char *str, int int_val);

#endif //TOKENISER_H
