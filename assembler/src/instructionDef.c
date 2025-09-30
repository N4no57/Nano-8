//
// Created by brego on 27/09/2025.
//

#include "../include/instructionDef.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

[[noreturn]]
static void operandError(char *mnemonic, ParsedOperand *ops) {
    fprintf(stderr, "Error: unsupported %s operand combination (%d,%d)\n",
            mnemonic, ops[0].kind, ops[1].kind);
    exit(1);
}

static void ensure_capacity(AssemblingSegment *seg, size_t needed) {
    while (seg->size + needed >= seg->capacity) {
        seg->capacity *= 2;
        uint8_t *tmp = realloc(seg->data, seg->capacity);
        if (!tmp) {
            fprintf(stderr, "Error: out of memory\n");
            exit(1);
        }
        seg->data = tmp;
    }
}

uint8_t get_size_reg_reg(int operand_count, ParsedOperand *ops) {
    return 2;
}

uint8_t get_size_reg_imm(int operand_count, ParsedOperand *ops) {
    return 3;
}

uint8_t get_size_reg_abs(int operand_count, ParsedOperand *ops) {
    return 4;
}

uint8_t get_size_reg_indreg(int operand_count, ParsedOperand *ops) {
    return 3;
}

uint8_t get_size_reg_indmem(int operand_count, ParsedOperand *ops) {
    return 4;
}

uint8_t get_size_reg_idx(int operand_count, ParsedOperand *ops) {
    return 5;
}

void encode_reg_reg(uint8_t *data, size_t *data_idx, ParsedOperand *ops) {
    data[(*data_idx)++] = (ops[0].reg << 4) + ops[1].reg;
}

void encode_reg_imm(uint8_t *data, size_t *data_idx, ParsedOperand *ops) {
    data[(*data_idx)++] = ops[0].reg << 4;
    data[(*data_idx)++] = (uint8_t)ops[1].imm;
}

void encode_reg_abs(uint8_t *data, size_t *data_idx, ParsedOperand *ops) {
     data[(*data_idx)++] = ops[0].reg << 4;
     data[(*data_idx)++] = ops[1].imm & 0xFF;
     data[(*data_idx)++] = ops[1].imm >> 8 & 0xFF;
}

void encode_reg_indreg(uint8_t *data, size_t *data_idx, ParsedOperand *ops) {
     data[(*data_idx)++] = ops[0].reg << 4;
     data[(*data_idx)++] = ops[1].mem_pair.reg_high << 4 | ops[1].mem_pair.reg_low;
}

void encode_reg_idx(uint8_t *data, size_t *data_idx, ParsedOperand *ops) {
     data[(*data_idx)++] = ops[0].reg << 4;
     data[(*data_idx)++] = ops[1].mem_pair.reg_high << 4 | ops[1].mem_pair.reg_low;
     data[(*data_idx)++] = ops[1].mem_pair.offset & 0xFF;
     data[(*data_idx)++] = ops[1].mem_pair.offset >> 8 & 0xFF;
}

void encode_abs_reg(uint8_t *data, size_t *data_idx, ParsedOperand *ops) {
    data[(*data_idx)++] = ops[0].imm & 0xFF;
    data[(*data_idx)++] = ops[0].imm >> 8 & 0xFF;
    data[(*data_idx)++] = ops[1].reg << 4;
}

void encode_indreg_reg(uint8_t *data, size_t *data_idx, ParsedOperand *ops) {
    data[(*data_idx)++] = ops[0].mem_pair.reg_high << 4 | ops[0].mem_pair.reg_low;
    data[(*data_idx)++] = ops[1].reg << 4;
}

void encode_idx_reg(uint8_t *data, size_t *data_idx, ParsedOperand *ops) {
    data[(*data_idx)++] = ops[0].mem_pair.reg_high << 4 | ops[0].mem_pair.reg_low;
    data[(*data_idx)++] = ops[0].mem_pair.offset & 0xFF;
    data[(*data_idx)++] = ops[0].mem_pair.offset >> 8 & 0xFF;
    data[(*data_idx)++] = ops[1].reg << 4;
}

void encode_reg(uint8_t *data, size_t *data_idx, ParsedOperand *ops) {
    data[(*data_idx)++] = ops[0].reg << 4;
}

void encode_imm(uint8_t *data, size_t *data_idx, ParsedOperand *ops) {
    data[(*data_idx)++] = ops[0].imm;
}

void encode_abs(uint8_t *data, size_t *data_idx, ParsedOperand *ops) {
    data[(*data_idx)++] = ops[0].imm & 0xFF;
    data[(*data_idx)++] = ops[0].imm >> 8 & 0xFF;
}

void encode_indreg(uint8_t *data, size_t *data_idx, ParsedOperand *ops) {
    data[(*data_idx)++] = ops[0].mem_pair.reg_high << 4 | ops[0].mem_pair.reg_low;
}

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
    operandError("mov", operands);
}

uint16_t get_size_push(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 2;
}

uint16_t get_size_pop(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 2;
}

uint16_t get_size_inb(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 3;
}

uint16_t get_size_outb(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 3;
}

uint16_t get_size_add(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    if (operands[0].kind == REGISTER && operands[1].kind == REGISTER) {
        return get_size_reg_reg(operand_count, operands);
    }
    if (operands[0].kind == REGISTER && operands[1].kind == IMMEDIATE) {
        return get_size_reg_imm(operand_count, operands);
    }
    operandError("add", operands);
}

uint16_t get_size_sub(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    if (operands[0].kind == REGISTER && operands[1].kind == REGISTER) {
        return get_size_reg_reg(operand_count, operands);
    }
    if (operands[0].kind == REGISTER && operands[1].kind == IMMEDIATE) {
        return get_size_reg_imm(operand_count, operands);
    }
    operandError("sub", operands);
}

uint16_t get_size_inc(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 2;
}

uint16_t get_size_dec(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 2;
}

uint16_t get_size_mul(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    if (operands[0].kind == REGISTER && operands[1].kind == REGISTER) {
        return get_size_reg_reg(operand_count, operands);
    }
    if (operands[0].kind == REGISTER && operands[1].kind == IMMEDIATE) {
        return get_size_reg_imm(operand_count, operands);
    }
    operandError("mul", operands);
}

uint16_t get_size_div(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    if (operands[0].kind == REGISTER && operands[1].kind == REGISTER) {
        return get_size_reg_reg(operand_count, operands);
    }
    if (operands[0].kind == REGISTER && operands[1].kind == IMMEDIATE) {
        return get_size_reg_imm(operand_count, operands);
    }
    operandError("div", operands);
}

uint16_t get_size_and(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    if (operands[0].kind == REGISTER && operands[1].kind == REGISTER) {
        return get_size_reg_reg(operand_count, operands);
    }
    if (operands[0].kind == REGISTER && operands[1].kind == IMMEDIATE) {
        return get_size_reg_imm(operand_count, operands);
    }
    operandError("and", operands);
}

uint16_t get_size_or(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    if (operands[0].kind == REGISTER && operands[1].kind == REGISTER) {
        return get_size_reg_reg(operand_count, operands);
    }
    if (operands[0].kind == REGISTER && operands[1].kind == IMMEDIATE) {
        return get_size_reg_imm(operand_count, operands);
    }
    operandError("or", operands);
}

uint16_t get_size_xor(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    if (operands[0].kind == REGISTER && operands[1].kind == REGISTER) {
        return get_size_reg_reg(operand_count, operands);
    }
    if (operands[0].kind == REGISTER && operands[1].kind == IMMEDIATE) {
        return get_size_reg_imm(operand_count, operands);
    }
    operandError("xor", operands);
}

uint16_t get_size_not(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 2;
}

uint16_t get_size_shl(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 2;
}

uint16_t get_size_shr(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 2;
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
    operandError("jmp", operands);
}

uint16_t get_size_jz(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
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
    operandError("jz", operands);
}

uint16_t get_size_jnz(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
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
    operandError("jnz", operands);
}

uint16_t get_size_jc(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
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
    operandError("jc", operands);
}

uint16_t get_size_jnc(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
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
    operandError("jnc", operands);
}

uint16_t get_size_jo(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
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
    operandError("jo", operands);
}

uint16_t get_size_jno(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
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
    operandError("jno", operands);
}

uint16_t get_size_jn(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
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
    operandError("jn", operands);
}

uint16_t get_size_jnn(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
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
    operandError("jnn", operands);
}

uint16_t get_size_call(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
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
    operandError("call", operands);
}

uint16_t get_size_ret(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 1;
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

uint16_t get_size_cli(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 1;
}

uint16_t get_size_sti(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 1;
}

uint16_t get_size_int(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 2;
}

uint16_t get_size_iret(int operand_count, int expected_operand_count, ParsedOperand operands[]) {
    return 1;
}

void encode_mov(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]) {
    ensure_capacity(seg, 5);

    if (operands[0].kind == REGISTER && operands[1].kind == REGISTER) {
        seg->data[seg->size++] = base_opcode;
        encode_reg_reg(seg->data, &seg->size, operands);
        return;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == IMMEDIATE) {
        seg->data[seg->size++] = base_opcode + 0b00000100;
        encode_reg_imm(seg->data, &seg->size, operands);
        return;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == ABSOLUTE) {
        seg->data[seg->size++] = base_opcode + 0b00001000;
        encode_reg_abs(seg->data, &seg->size, operands);
        return;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == INDIRECT_REG) {
        seg->data[seg->size++] = base_opcode + 0b00001100;
        encode_reg_indreg(seg->data, &seg->size, operands);
        return;
    }
    if (operands[0].kind == REGISTER && operands[1].kind == INDEXED_MEM) {
        seg->data[seg->size++] = base_opcode + 0b00010000;
        encode_reg_idx(seg->data, &seg->size, operands);
        return;
    }
    if (operands[0].kind == ABSOLUTE && operands[1].kind == REGISTER) {
        seg->data[seg->size++] = base_opcode + 0b00010100;
        encode_abs_reg(seg->data, &seg->size, operands);
        return;
    }
    if (operands[0].kind == INDIRECT_REG && operands[1].kind == REGISTER) {
        seg->data[seg->size++] = base_opcode + 0b00011000;
        encode_indreg_reg(seg->data, &seg->size, operands);
        return;
    }
    if (operands[0].kind == INDEXED_MEM && operands[1].kind == REGISTER) {
        seg->data[seg->size++] = base_opcode + 0b00011100;
        encode_idx_reg(seg->data, &seg->size, operands);
    }
}

void encode_push(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]) {
    ensure_capacity(seg, 1);

    if (operands[0].kind == REGISTER) {
        seg->data[seg->size++] = base_opcode;
        encode_reg(seg->data, &seg->size, operands);
    }
    if (operands[0].kind == IMMEDIATE) {
        seg->data[seg->size++] = base_opcode + 0b00000100;
        encode_imm(seg->data, &seg->size, operands);
    }
}

void encode_pop(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]) {
    ensure_capacity(seg, 1);

    if (operands[0].kind == REGISTER) {
        seg->data[seg->size++] = base_opcode;
        encode_reg(seg->data, &seg->size, operands);
    }
}

void encode_inb(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]) {
    ensure_capacity(seg, 3);

    if (operands[0].kind == REGISTER && operands[1].kind == ABSOLUTE) {
        seg->data[seg->size++] = base_opcode;
        encode_reg_abs(seg->data, &seg->size, operands);
    }
}

void encode_outb(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]) {
    ensure_capacity(seg, 3);

    if (operands[0].kind == ABSOLUTE && operands[1].kind == REGISTER) {
        seg->data[seg->size++] = base_opcode;
        encode_abs_reg(seg->data, &seg->size, operands);
    }
    if (operands[0].kind == ABSOLUTE && operands[1].kind == IMMEDIATE) {
        seg->data[seg->size++] = base_opcode + 0b00000100;
        seg->data[seg->size++] = operands[0].imm & 0xFF;
        seg->data[seg->size++] = operands[0].imm >> 8 & 0xFF;
        seg->data[seg->size++] = operands[1].imm & 0xFF;
    }
}

void encode_complexArith(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]) {
    ensure_capacity(seg, 3);

    if (operands[0].kind == REGISTER && operands[1].kind == REGISTER) {
        seg->data[seg->size++] = base_opcode;
        encode_reg_reg(seg->data, &seg->size, operands);
    }
    if (operands[0].kind == REGISTER && operands[1].kind == IMMEDIATE) {
        seg->data[seg->size++] = base_opcode + 0b00000100;
        encode_reg_indreg(seg->data, &seg->size, operands);
    }
}

void encode_simpleArith(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]) {
    ensure_capacity(seg, 1);

    if (operands[0].kind == REGISTER) {
        seg->data[seg->size++] = base_opcode;
        encode_reg(seg->data, &seg->size, operands);
    }
}

void encode_jmp(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]) {
    ensure_capacity(seg, 3);

    if (operands[0].kind == ABSOLUTE) {
        seg->data[seg->size++] = base_opcode;
        encode_abs(seg->data, &seg->size, operands);
    }
    if (operands[0].kind == INDIRECT_REG) {
        seg->data[seg->size++] = base_opcode + 0b00000100;
        encode_indreg(seg->data, &seg->size, operands);
    }
    if (operands[0].kind == RELATIVE) {
        seg->data[seg->size++] = base_opcode + 0b00001000;
        encode_imm(seg->data, &seg->size, operands);
    }
    if (operands[0].kind == INDIRECT_MEM) {
        seg->data[seg->size++] = base_opcode + 0b00001100;
        encode_abs(seg->data, &seg->size, operands);
    }
}

void encode_opcode(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]) {
    ensure_capacity(seg, 0);

    seg->data[seg->size++] = base_opcode;
}
