//
// Created by brego on 27/09/2025.
//

#include "../include/instructionDef.h"

#include <stdio.h>
#include <stdlib.h>

InstructionDef *find_instruction(InstructionDef table[], int table_int, const char *mnemonic) {
    for (int i = 0; i < table_int; i++) {
        if (strcmp(mnemonic, table[i].mnemonic) == 0) {
            return &table[i];
        }
    }
    return NULL;
}

uint16_t get_size_mov(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    if (operand_count != expected_operand_count) {
        fprintf(stderr, "Error: operand count mismatch\n");
        exit(1);
    }

    if (operands[0].kind == REGISTER && operands[1].kind == REGISTER) {
        return 2;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == IMMEDIATE) {
        return 3;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == ABSOLUTE) {
        return 4;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == INDIRECT_REG) {
        return 3;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == INDEXED_MEM) {
        return 5;
    }
    if (operands[0].kind == ABSOLUTE && operands[1].kind == REGISTER) {
        return 4;
    }
    if (operands[0].kind == INDIRECT_REG && operands[1].kind == REGISTER) {
        return 3;
    }
    if (operands[0].kind == INDEXED_MEM && operands[1].kind == REGISTER) {
        return 5;
    }
    // no more possible operand combinations if somehow skip past all if statements something is wrong...
}

uint16_t get_size_hlt(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    if (operand_count != expected_operand_count) {
        fprintf(stderr, "Error: operand count mismatch\n");
        exit(1);
    }
    return 1;
}

uint16_t get_size_nop(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    if (operand_count != expected_operand_count) {
        fprintf(stderr, "Error: operand count mismatch\n");
        exit(1);
    }
    return 1;
}

void encode_mov(uint8_t base_opcode, int operand_count, uint8_t *out, uint16_t *binary_index, ParsedOperand operands[]) {
    if (operands[0].kind == REGISTER && operands[1].kind == REGISTER) {
        out[(*binary_index)++] = base_opcode;
        out[(*binary_index)++] = (operands[0].reg << 4) + operands[1].reg;
        return;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == IMMEDIATE) {
        out[(*binary_index)++] = base_opcode + 0b00000100;
        out[(*binary_index)++] = operands[0].reg << 4;
        out[(*binary_index)++] = (uint8_t)operands[1].imm;
        return;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == ABSOLUTE) {
        out[(*binary_index)++] = base_opcode + 0b00001000;
        out[(*binary_index)++] = operands[0].reg << 4;
        out[(*binary_index)++] = operands[1].imm & 0xFF;
        out[(*binary_index)++] = operands[1].imm >> 8 & 0xFF;
        return;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == INDIRECT_REG) { // TODO
        return;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == INDEXED_MEM) { // TODO
        return;
    }
    if (operands[0].kind == ABSOLUTE && operands[1].kind == REGISTER) { // TODO
        return;
    }
    if (operands[0].kind == INDIRECT_REG && operands[1].kind == REGISTER) { // TODO
        return;
    }
    if (operands[0].kind == INDEXED_MEM && operands[1].kind == REGISTER) { // TODO
        return;
    }
}

void encode_hlt(const uint8_t base_opcode, int operand_count, uint8_t *out, uint16_t *binary_index, ParsedOperand operands[]) {
    out[(*binary_index)++] = base_opcode;
}

void encode_nop(const uint8_t base_opcode, int operand_count, uint8_t *out, uint16_t *binary_index, ParsedOperand operands[]) {
    out[(*binary_index)++] = base_opcode;
}
