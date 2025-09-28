//
// Created by brego on 27/09/2025.
//

#include "../include/tokeniser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/utils.h"

char *mnemonics[] = {
    "mov",
    "add",
    "hlt",
    "nop"
};

char *registers[] = {
    "r0",
    "r1",
    "r2",
    "r3",
    "pc",
    "sp",
    "bp",
    "h",
    "l"
};

char *strndup(const char *s, const size_t n) {
    char *p;
    size_t n1;

    for (n1 = 0; n1 < n && s[n1] != '\0'; n1++)
        continue;
    p = malloc(n + 1);
    if (p != NULL) {
        memcpy(p, s, n1);
        p[n1] = '\0';
    }
    return p;
}

void initTokenList(TokenList *token_list) {
    token_list->count = 0;
    token_list->capacity = 8;
    token_list->data = malloc(token_list->capacity * sizeof(Token));
}

void freeTokenList(const TokenList *token_list) {
    for (int i = 0; i < token_list->count; i++) {
        if (token_list->data[i].type != TOKEN_NUMBER) {
            free(token_list->data[i].str_val);
        }
    }
    free(token_list->data);
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

int ismnemonic(const char *s) {
    for (int i = 0; i < sizeof(mnemonics) / sizeof(mnemonics[0]); i++) {
        if (strcmp(s, mnemonics[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int isregister(const char *s) {
    for (int i = 0; i < sizeof(registers) / sizeof(registers[0]); i++) {
        if (strcmp(s, registers[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void parse_number(TokenList *token_list, const char *line, int *i, int base) {
    char buff[64]; int bi = 0;
    if (isalnum(line[*i])) {
        while (isalnum(line[*i])) {
            buff[bi++] = line[(*i)++];
        }
        buff[bi] = '\0';
    }
    const Token t = { .type = TOKEN_NUMBER, .int_value =  strtol(buff, NULL, base) };
    token_list_push(token_list, t);
}

TokenList tokenise(char **lines) {
    TokenList token_list;
    initTokenList(&token_list);
    int line_no = 0;

    while (lines[line_no] != NULL) {
        const char *line = lines[line_no];
        int i = 0;

        while (line[i] != '\0') {
            if (isspace(line[i])) {
                i++;
                continue;
            }

            // comments
            if (line[i] == ';') break;

            // symbols
            if (line[i] == ',' || line[i] == ':' ||
                line[i] == '(' || line[i] == ')' ||
                line[i] == '[' || line[i] == ']' ||
                line[i] == '+' || line[i] == '-') {
                const Token t = { .type = TOKEN_SYMBOL, .str_val = strndup(&line[i], 1) };
                token_list_push(&token_list, t);
                i++;
                continue;
            }

            // number, immediate
            if (line[i] == '#' || line[i] == '$' || line[i] == '%') {
                const Token t1 = { .type = TOKEN_SYMBOL, .str_val = strndup(&line[i], 1) };
                token_list_push(&token_list, t1);
                int base = get_base(line[i]);
                i++;
                if (line[i] == '$' || line[i] == '%') { // "#" token most likely proceeded by these
                    base = get_base(line[i]);
                    const Token t2 = { .type = TOKEN_SYMBOL, .str_val = strndup(&line[i], 1) };
                    token_list_push(&token_list, t2);
                    i++;
                } // if not continue to number
                parse_number(&token_list, line, &i, base);
                continue;
            }

            // number, no base modifier
            if (isdigit(line[i])) {
                parse_number(&token_list, line, &i, 10);
                continue;
            }

            // identifier (mnemonic, register, label, directive)
            if (isalpha(line[i]) || line[i] == '.') {
                char buff[64]; int bi = 0;
                while (isalnum(line[i]) || line[i] == '.') {
                    buff[bi++] = line[i++];
                }
                buff[bi] = '\0';

                Token t = {0};
                if (buff[0] == '.') {
                    t.type = TOKEN_DIRECTIVE;
                } else if (ismnemonic(buff)) {
                    t.type = TOKEN_MNEMONIC;
                } else if (isregister(buff)) {
                    t.type = TOKEN_REGISTER;
                } else {
                    t.type = TOKEN_LABEL;
                }
                t.str_val = strdup(buff);
                token_list_push(&token_list, t);
                continue;
            }

            i++;
        }
        line_no++;
    }
    Token t = {TOKEN_EOF};
    token_list_push(&token_list, t);

    return token_list;
}
