//
// Created by brego on 27/09/2025.
//

#ifndef INSTRUCTIONDEF_H
#define INSTRUCTIONDEF_H

#include <stdint.h>
#include "utils.h"

typedef struct {
    const char *mnemonic;
    uint8_t base_opcode;
    int operand_count;
    void (*encode)(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]);
    uint16_t (*get_size)(int operand_count, int expected_operand_count, ParsedOperand operands[]);
} InstructionDef;

InstructionDef *find_instruction(InstructionDef table[], int table_size, const char *mnemonic);

uint16_t get_size_mov(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_push(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_pop(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_inb(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_outb(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_add(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_sub(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_inc(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_dec(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_mul(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_div(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_and(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_or(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_xor(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_not(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_shl(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_shr(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_jmp(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_jz(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_jnz(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_jc(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_jnc(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_jo(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_jno(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_jn(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_jnn(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_call(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_ret(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_hlt(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_nop(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_cli(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_sti(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_int(int operand_count, int expected_operand_count, ParsedOperand operands[]);
uint16_t get_size_iret(int operand_count, int expected_operand_count, ParsedOperand operands[]);

void encode_mov(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]);
void encode_push(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]);
void encode_pop(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]);
void encode_inb(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]);
void encode_outb(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]);
void encode_complexArith(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]);
void encode_simpleArith(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]);
void encode_jmp(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]);
void encode_opcode(uint8_t base_opcode, int operand_count, AssemblingSegment *seg, ParsedOperand operands[]);

#endif //INSTRUCTIONDEF_H
