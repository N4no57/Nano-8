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

uint16_t get_size_hlt(Token tokens[], int operand_count, int expected_operand_count) {
    if (operand_count != expected_operand_count) {
        fprintf(stderr, "Error: operand count mismatch\n");
        exit(1);
    }
    return 1;
}

uint16_t get_size_nop(Token tokens[], int operand_count, int expected_operand_count) {
    if (operand_count != expected_operand_count) {
        fprintf(stderr, "Error: operand count mismatch\n");
        exit(1);
    }
    return 1;
}

void encode_hlt(const uint8_t base_opcode, int operand_count, uint8_t *out, uint16_t *binary_index, Token tokens[]) {
    out[(*binary_index)++] = base_opcode;
}

void encode_nop(const uint8_t base_opcode, int operand_count, uint8_t *out, uint16_t *binary_index, Token tokens[]) {
    out[(*binary_index)++] = base_opcode;
}
