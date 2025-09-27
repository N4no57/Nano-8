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

uint16_t get_size_hlt(Token tokens[]) {
    // if (tokens != NULL) {
    //     printf("Unexpected tokens, %s", tokens[0].str_val);
    //     exit(1);
    // }
    return 1;
}

uint16_t get_size_nop(Token tokens[]) {
    // if (tokens != NULL) {
    //     printf("Unexpected tokens, %s", tokens[0].str_val);
    //     exit(1);
    // }
    return 1;
}

void encode_hlt(const uint8_t base_opcode, int operand_count, uint8_t *out, Token tokens[]) {
    out[0] = base_opcode;
}

void encode_nop(const uint8_t base_opcode, int operand_count, uint8_t *out, Token tokens[]) {
    out[0] = base_opcode;
}
