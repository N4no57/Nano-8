//
// Created by brego on 27/09/2025.
//

#ifndef INSTRUCTIONDEF_H
#define INSTRUCTIONDEF_H

#include <stdint.h>
#include <string.h>
#include "tokeniser.h"
#include "utils.h"

typedef struct {
    const char *mnemonic;
    uint8_t base_opcode;
    int operand_count;
    void (*encode)(uint8_t base_opcode, int operand_count, uint8_t *out, uint16_t *binary_index, ParsedOperand operands[]);
    uint16_t (*get_size)(int operand_count, int expected_operand_count, ParsedOperand operands[]);
} InstructionDef;

InstructionDef *find_instruction(InstructionDef table[], int table_size, const char *mnemonic);

uint16_t get_size_mov(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_hlt(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_nop(int operand_count, int expected_operand_count, ParsedOperand operands[]);

void encode_mov(uint8_t base_opcode, int operand_count, uint8_t *out, uint16_t *binary_index, ParsedOperand operands[]);
void encode_hlt(uint8_t base_opcode, int operand_count, uint8_t *out, uint16_t *binary_index, ParsedOperand operands[]);
void encode_nop(uint8_t base_opcode, int operand_count, uint8_t *out, uint16_t *binary_index, ParsedOperand operands[]);

#endif //INSTRUCTIONDEF_H
