//
// Created by Bernardo on 28/09/2025.
//

#ifndef UTILS_H
#define UTILS_H

typedef struct {
    enum { IMMEDIATE, REGISTER, ABSOLUTE, RELATIVE, INDIRECT_MEM, INDIRECT_REG, INDEXED_MEM} kind;
    union {
        int imm;
        int reg;
        struct {
            int reg_high;
            int reg_low;
            int offset;
        } mem_pair;
    };
} ParsedOperand;

#endif //UTILS_H
