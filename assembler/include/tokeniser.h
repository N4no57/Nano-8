//
// Created by brego on 27/09/2025.
//

#ifndef TOKENISER_H
#define TOKENISER_H

#include <stdint.h>
#include "symbolTable.h"

#define MAX_LINE_LENGTH 1024

typedef enum {
    TOKEN_MNEMONIC,
    TOKEN_REGISTER,
    TOKEN_NUMBER,
    TOKEN_LABEL,
    TOKEN_SYMBOL,
    TOKEN_DIRECTIVE,
} TokenType;

typedef struct {
    TokenType type;
    union {
        char *str_val;
        int int_value;
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

#endif //TOKENISER_H
