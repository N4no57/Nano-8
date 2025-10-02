//
// Created by Bernardo on 28/09/2025.
//

#ifndef UTILS_H
#define UTILS_H

#include "symbolTable.h"
#include "tokeniser.h"
#include "objectFileWriter.h"

typedef struct {
    enum { NONE, IMMEDIATE, REGISTER, ABSOLUTE, RELATIVE, INDIRECT_MEM, INDIRECT_REG, INDEXED_MEM} kind;
    union {
        uint64_t imm; // can be an actual immediate, resolved register or memory address. interpretation dependent on instruction/operand
        uint64_t reg;
        struct {
            uint64_t reg_high;
            uint64_t reg_low;
            int64_t offset;
        } mem_pair;
    };
} ParsedOperand;

void relocationTableAppend(struct RelocationTable *relocTable, const char *name,
    const uint16_t segment_index, const uint16_t segment_offset, const int16_t addend, const uint8_t type);

int get_reg(const char *s);

void consume_token(int *tok_idx, Token *t, const TokenList *tok_list);

int get_base(char c);

int is_base_mod(Token t);

// parses tokens until it reaches a "," then returns the parsed operand
ParsedOperand operand_parser(const TokenList *tokens, SymbolTable *symbol_table, int *tok_idx, Token *current_tok,
    struct RelocationTable *reloc_table, const AssemblingSegmentTable *segTable, AssemblingSegment current_seg, Token mnemonic,
    const struct ConstTable *constTable);

#endif //UTILS_H
