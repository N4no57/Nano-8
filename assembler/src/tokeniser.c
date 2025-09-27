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

TokenList tokenise(char **lines, SymbolTable *symbol_table) {
    TokenList token_list = {};
    int current_line = 0;
    int buff_index = 0;
    char buff[MAX_LINE_LENGTH] = {0};

    while (lines[current_line] != NULL) {
        buff_index = 0;
        memset(buff, 0, MAX_LINE_LENGTH);
        for (int i = 0; i < strlen(lines[current_line]); i++) {
            Token token = {};
            if (isspace(lines[current_line][i])) {
                while (isspace(lines[current_line][i])) {
                    i++;
                }
            }
            buff[buff_index] = lines[current_line][i];

            if (strcmp(buff, "hlt") == 0 && strcmp(buff, "nop") == 0) {
                token.type = TOKEN_MNEMONIC;
                token.str_val = strdup(buff);
                token_list_push(&token_list, token);
                memset(buff, 0, buff_index + 1);
                buff_index = -1;
            }

            if (buff[buff_index] == ':') {
                token.type = TOKEN_LABEL;
                token.str_val = strdup(buff);
                memset(buff, 0, buff_index + 1);
                buff_index = -1;
            }

            buff_index++;
        }
        current_line++;
    }

    return token_list;
}
