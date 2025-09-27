//
// Created by brego on 27/09/2025.
//

#ifndef INSTRUCTIONDEF_H
#define INSTRUCTIONDEF_H
#include <stdint.h>
#include <string.h>

typedef struct {
    const char *mnemonic;
    uint8_t base_opcode;
    int operand_count;
    void (*encode)(uint8_t *out, char **operands);
} InstructionDef;

InstructionDef *find_instruction(InstructionDef *table[], const char *mnemonic);
void encode_hlt(uint8_t *out, char **operands);
void encode_nop(uint8_t *out, char **operands);

#endif //INSTRUCTIONDEF_H
