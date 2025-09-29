//
// Created by brego on 27/09/2025.
//

#include "../include/instructionDef.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

uint16_t get_size_jmp(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    if (operand_count != expected_operand_count) {
        fprintf(stderr, "Error: operand count mismatch\n");
        exit(1);
    }


    if (operands[0].kind == ABSOLUTE) {
        return 3;
    }
    if (operands[0].kind == INDIRECT_REG) {
        return 2;
    }
    if (operands[0].kind == RELATIVE) {
        return 2;
    }
    if (operands[0].kind == INDIRECT_MEM) {
        return 3;
    }
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

void encode_mov(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]) {
    if (seg->size+5 >= seg->capacity) {
        seg->capacity *= 2;
        uint8_t *temp = realloc(seg->data, seg->capacity);
        if (!temp) {
            fprintf(stderr, "Error: out of memory\n");
            exit(1);
        }
        seg->data = temp;
    }

    if (operands[0].kind == REGISTER && operands[1].kind == REGISTER) {
        seg->data[seg->size++] = base_opcode;
        seg->data[seg->size++] = (operands[0].reg << 4) + operands[1].reg;
        return;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == IMMEDIATE) {
        seg->data[seg->size++] = base_opcode + 0b00000100;
        seg->data[seg->size++] = operands[0].reg << 4;
        seg->data[seg->size++] = (uint8_t)operands[1].imm;
        return;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == ABSOLUTE) {
        seg->data[seg->size++] = base_opcode + 0b00001000;
        seg->data[seg->size++] = operands[0].reg << 4;
        seg->data[seg->size++] = operands[1].imm & 0xFF;
        seg->data[seg->size++] = operands[1].imm >> 8 & 0xFF;
        return;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == INDIRECT_REG) {
        seg->data[seg->size++] = base_opcode + 0b00001100;
        seg->data[seg->size++] = operands[0].reg << 4;
        seg->data[seg->size++] = operands[1].mem_pair.reg_high << 4 | operands[1].mem_pair.reg_low;
        return;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == INDEXED_MEM) {
        seg->data[seg->size++] = base_opcode + 0b00010000;
        seg->data[seg->size++] = operands[0].reg << 4;
        seg->data[seg->size++] = operands[1].mem_pair.reg_high << 4 | operands[1].mem_pair.reg_low;
        seg->data[seg->size++] = operands[1].mem_pair.offset & 0xFF;
        seg->data[seg->size++] = operands[1].mem_pair.offset >> 8 & 0xFF;
        return;
    }
    if (operands[0].kind == ABSOLUTE && operands[1].kind == REGISTER) {
        seg->data[seg->size++] = base_opcode + 0b00010100;
        seg->data[seg->size++] = operands[0].imm & 0xFF;
        seg->data[seg->size++] = operands[0].imm >> 8 & 0xFF;
        seg->data[seg->size++] = operands[1].reg << 4;
        return;
    }
    if (operands[0].kind == INDIRECT_REG && operands[1].kind == REGISTER) {
        seg->data[seg->size++] = base_opcode + 0b00011000;
        seg->data[seg->size++] = operands[0].mem_pair.reg_high << 4 | operands[0].mem_pair.reg_low;
        seg->data[seg->size++] = operands[1].reg << 4;
        return;
    }
    if (operands[0].kind == INDEXED_MEM && operands[1].kind == REGISTER) {
        seg->data[seg->size++] = base_opcode + 0b00011100;
        seg->data[seg->size++] = operands[0].mem_pair.reg_high << 4 | operands[0].mem_pair.reg_low;
        seg->data[seg->size++] = operands[0].mem_pair.offset & 0xFF;
        seg->data[seg->size++] = operands[0].mem_pair.offset >> 8 & 0xFF;
        seg->data[seg->size++] = operands[1].reg << 4;
    }
}

void encode_jmp(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]) {
    if (seg->size+3 >= seg->capacity) {
        seg->capacity *= 2;
        uint8_t *temp = realloc(seg->data, seg->capacity);
        if (!temp) {
            fprintf(stderr, "Error: out of memory\n");
            exit(1);
        }
        seg->data = temp;
    }

    if (operands[0].kind == ABSOLUTE) {
        seg->data[seg->size++] = base_opcode;
        seg->data[seg->size++] = operands[0].imm & 0xFF;
        seg->data[seg->size++] = operands[0].imm >> 8 & 0xFF;
    }
    if (operands[0].kind == INDIRECT_REG) {
        seg->data[seg->size++] = base_opcode + 0b00000100;
    }
    if (operands[0].kind == RELATIVE) {
        seg->data[seg->size++] = base_opcode + 0b00001000;
        seg->data[seg->size++] = operands[0].mem_pair.reg_high << 4 | operands[0].mem_pair.reg_low;
    }
    if (operands[0].kind == INDIRECT_MEM) {
        seg->data[seg->size++] = base_opcode + 0b00001100;
        seg->data[seg->size++] = operands[0].imm & 0xFF;
        seg->data[seg->size++] = operands[0].imm >> 8 & 0xFF;
    }
}

void encode_hlt(const uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]) {
    if (seg->size >= seg->capacity) {
        seg->capacity *= 2;
        uint8_t *temp = realloc(seg->data, seg->capacity);
        if (!temp) {
            fprintf(stderr, "Error: out of memory\n");
            exit(1);
        }
        seg->data = temp;
    }
    seg->data[seg->size++] = base_opcode;
}

void encode_nop(const uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]) {
    if (seg->size >= seg->capacity) {
        seg->capacity *= 2;
        uint8_t *temp = realloc(seg->data, seg->capacity);
        if (!temp) {
            fprintf(stderr, "Error: out of memory\n");
            exit(1);
        }
        seg->data = temp;
    }
    seg->data[seg->size++] = base_opcode;
}
