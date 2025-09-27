//
// Created by brego on 27/09/2025.
//

#include "../include/instructionDef.h"

InstructionDef *find_instruction(InstructionDef *table[], const char *mnemonic) {
    for (int i = 0; i < sizeof(table) / sizeof(InstructionDef); i++) {
        if (strcmp(mnemonic, table[i]->mnemonic) == 0) {
            return table[i];
        }
    }
    return NULL;
}