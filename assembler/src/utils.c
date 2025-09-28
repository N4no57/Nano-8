//
// Created by brego on 28/09/2025.
//

#include "../include/utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int get_reg(const char *s) {
    if (strcmp(s, "r0") == 0) return 0;
    if (strcmp(s, "r1") == 0) return 1;
    if (strcmp(s, "r2") == 0) return 2;
    if (strcmp(s, "r3") == 0) return 3;
    if (strcmp(s, "pc") == 0) return 12;
    if (strcmp(s, "sp") == 0) return 13;
    if (strcmp(s, "bp") == 0) return 14;
    if (strcmp(s, "h") == 0) return 15;
    if (strcmp(s, "l") == 0) return 16;
    return -1;
}

void consume_token(int *tok_idx, Token *t, const TokenList *tok_list) {
    if (*tok_idx >= tok_list->count) {
        printf("Out of tokens\n");
        exit(1);
    }
    *t = tok_list->data[*tok_idx];
    (*tok_idx)++;
}

Token peek(int *tok_idx, TokenList *tok_list, int n) {
    if (*tok_idx+n >= tok_list->count) {
        fprintf(stderr, "Out of tokens\n");
        exit(1);
    }
    return tok_list->data[*tok_idx+n];
}

int get_base(char c) {
    if (c == '$') {
        return 16;
    }
    if (c == '%') {
        return 2;
    }
    return 10;
}

int is_base_mod(Token t) {
    if (t.type == TOKEN_SYMBOL) {
        if (t.str_val[0] == '$' || t.str_val[0] == '%') {
            return 1;
        }
    }
    return 0;
}

// parses tokens until it reaches a "," then returns the parsed operand
ParsedOperand operand_parser(const TokenList *tokens, SymbolTable *symbol_table, int *tok_idx, Token *current_tok) {
    ParsedOperand operand = {0, {0}};

    while (current_tok->type != TOKEN_EOF && current_tok->type != TOKEN_MNEMONIC &&
        !(current_tok->type == TOKEN_SYMBOL && current_tok->str_val[0] == ',')) {
        // go through and find out what argument this is and if it is part of a bigger piece or a standalone
        // e.g. R0 -> enum REGISTER, #1023 -> enum IMMEDIATE and so on
        switch (current_tok->type) {
            // most verbose case. needs careful handling.
            case TOKEN_SYMBOL:
                if (current_tok->str_val[0] == '(') { // indirect reg/mem
                    consume_token(tok_idx, current_tok, tokens); // consume "("
                    if (current_tok->type == TOKEN_REGISTER) { // expect another reg after a ","
                        operand.kind = INDIRECT_REG;
                        int reg = 0;
                        reg = get_reg(current_tok->str_val);
                        if (reg == -1) {
                            printf("Invalid register\n");
                            exit(1);
                        }
                        operand.mem_pair.reg_high = reg;
                        consume_token(tok_idx, current_tok, tokens);
                        consume_token(tok_idx, current_tok, tokens);
                        reg = get_reg(current_tok->str_val);
                        if (reg == -1) {
                            printf("Invalid register\n");
                            exit(1);
                        }
                        operand.mem_pair.reg_low = reg;
                        consume_token(tok_idx, current_tok, tokens);
                        consume_token(tok_idx, current_tok, tokens);
                    }
                    return operand;
                }

                // most complex one of them all
                if (current_tok->str_val[0] == '[') { // indexed [(r0, r1)±num]
                    consume_token(tok_idx, current_tok, tokens);
                    if (!(current_tok->type == TOKEN_SYMBOL && current_tok->str_val[0] == '(')) {
                        fprintf(stderr, "Invalid token\n");
                        exit(1);
                    }
                    operand = get_reg_pair(tokens, symbol_table, tok_idx, current_tok); // (r0, r1)
                    operand.kind = INDEXED_MEM;
                    if (current_tok->type == TOKEN_SYMBOL &&
                        !(current_tok->str_val[0] == '+' || current_tok->str_val[0] == '-')) {
                        fprintf(stderr, "Invalid token\n");
                        exit(1);
                    }
                    char sign = current_tok->str_val[0];
                    consume_token(tok_idx, current_tok, tokens);
                    if (current_tok->type != TOKEN_NUMBER) {
                        fprintf(stderr, "Invalid token\n");
                        exit(1);
                    }
                    int num = current_tok->int_value;
                    if (sign == '-') num *= -1;
                    operand.mem_pair.offset = num;
                    consume_token(tok_idx, current_tok, tokens); // offset num
                    consume_token(tok_idx, current_tok, tokens); // "]"
                    return operand;
                }

                if (is_base_mod(*current_tok)) { // check if base modifier, expect to be succeeded by a number token
                    operand.kind = ABSOLUTE;
                    consume_token(tok_idx, current_tok, tokens); // next token expected to be number token
                    if (current_tok->type != TOKEN_NUMBER) { // check if not
                        printf("Invalid token\n");
                        exit(1);
                    }
                    // is assumed to already have base converted
                    operand.imm = current_tok->int_value;
                    consume_token(tok_idx, current_tok, tokens);
                    return operand;
                }

                if (current_tok->str_val[0] == '#') { // check if immediate value
                    // may be succeeded by base modifier
                    operand.kind = IMMEDIATE;
                    consume_token(tok_idx, current_tok, tokens);
                    if (is_base_mod(*current_tok)) {
                        consume_token(tok_idx, current_tok, tokens);
                    }
                    // expect number tok
                    if (current_tok->type != TOKEN_NUMBER) { // check if not
                        printf("Invalid token\n");
                        exit(1);
                    }
                    // is assumed to already have base converted
                    operand.imm = current_tok->int_value;
                    consume_token(tok_idx, current_tok, tokens);
                    return operand;
                }
                break;


            case TOKEN_REGISTER:
                operand.kind = REGISTER;
                const int reg = get_reg(current_tok->str_val);
                if (reg == -1) {
                    printf("Invalid register\n");
                    exit(1);
                }
                operand.reg = reg;
                consume_token(tok_idx, current_tok, tokens);
                return operand;


            case TOKEN_NUMBER: // number without base modifiers
                operand.kind = ABSOLUTE;
                operand.imm = current_tok->int_value;
                consume_token(tok_idx, current_tok, tokens);
                return operand;
            default: break;
        }
    }

    return operand;
}