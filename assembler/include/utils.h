//
// Created by Bernardo on 28/09/2025.
//

#ifndef UTILS_H
#define UTILS_H

#include "tokeniser.h"

typedef struct {
    enum { IMMEDIATE, REGISTER, ABSOLUTE, RELATIVE, INDIRECT_MEM, INDIRECT_REG, INDEXED_MEM} kind;
    union {
        int imm; // can be an actual immediate, resolved register or memory address. interpretation dependent on instruction/operand
        int reg;
        struct {
            int reg_high;
            int reg_low;
            int offset;
        } mem_pair;
    };
} ParsedOperand;

int get_reg(const char *s);

void consume_token(int *tok_idx, Token *t, const TokenList *tok_list);

int get_base(char c);

int is_base_mod(Token t);

// parses tokens until it reaches a "," then returns the parsed operand
inline ParsedOperand operand_parser(const TokenList *tokens, SymbolTable *symbol_table, int *tok_idx, Token *current_tok);

#endif //UTILS_H
