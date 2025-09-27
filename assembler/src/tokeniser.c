//
// Created by brego on 27/09/2025.
//

#include "../include/tokeniser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initTokenList(TokenList *token_list) {
    token_list->count = 0;
    token_list->capacity = 8;
    token_list->data = malloc(token_list->capacity * sizeof(Token));
}

void token_list_push(TokenList *token_list, Token T) {
    if (token_list->count >= token_list->capacity) {
        token_list->capacity *= 2;
        Token *tmp = realloc(token_list->data, token_list->capacity * sizeof(Token));
        if (!tmp) {
            printf("Failed to allocate memory for token list\n");
            exit(1);
        }
        token_list->data = tmp;
    }
    token_list->data[token_list->count] = T;
    token_list->count++;
}

