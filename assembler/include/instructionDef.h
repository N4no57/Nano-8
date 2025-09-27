//
// Created by brego on 27/09/2025.
//

#ifndef INSTRUCTIONDEF_H
#define INSTRUCTIONDEF_H

#include <stdint.h>
#include <string.h>
#include "tokeniser.h"

typedef struct {
    const char *mnemonic;
    uint8_t base_opcode;
    int operand_count;
    void (*encode)(uint8_t base_opcode, int operand_count, uint8_t *out, Token tokens[]);
    uint16_t (*get_size)(Token tokens[]);
} InstructionDef;

InstructionDef *find_instruction(InstructionDef table[], int table_size, const char *mnemonic);

uint16_t get_size_hlt(Token tokens[]);
uint16_t get_size_nop(Token tokens[]);

void encode_hlt(uint8_t base_opcode, int operand_count, uint8_t *out, Token tokens[]);
void encode_nop(uint8_t base_opcode, int operand_count, uint8_t *out, Token tokens[]);

#endif //INSTRUCTIONDEF_H
