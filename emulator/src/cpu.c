#include "../include/cpu.h"

#include <io.h>
#include <string.h>
#include <stdio.h>

uint8_t fetch_byte(CPU *cpu) {
    return read_byte(&cpu->memory, cpu->PC++);
}

uint16_t fetch_word(CPU *cpu) {
    cpu->PC += 2;
    return read_word(&cpu->memory, cpu->PC-2);
}

void set_reg(CPU *cpu, const uint8_t reg, const uint8_t value) {
    switch (reg) {
        case 0x0: // R0
            cpu->R0 = value;
            break;
        case 0x1: // R1
            cpu->R1 = value;
            break;
        case 0x2: // R2
            cpu->R2 = value;
            break;
        case 0x3: // R3
            cpu->R3 = value;
            break;
        case 0xA: // PC
            cpu->PC = value;
            break;
        case 0xB: // SP
            cpu->SP = value;
            break;
        case 0xC: // BP
            cpu->BP = value;
            break;
        case 0xE: // H reg
            cpu->H = value;
            break;
        case 0xF: // L reg
            cpu->L = value;
            break;
        default:
            break;
    }
}

uint8_t read_reg(const CPU *cpu, const uint8_t reg) {
    switch (reg) {
        case 0x0: // R0
            return cpu->R0;
        case 0x1: // R1
            return cpu->R1;
        case 0x2: // R2
            return cpu->R2;
        case 0x3: // R3
            return cpu->R3;
        case 0xB: // PC
            return cpu->PC;
        case 0xC: // SP
            return cpu->SP;
        case 0xD: // BP
            return cpu->BP;
        case 0xE: // H reg
            return cpu->H;
        case 0xF: // L reg
            return cpu->L;
        default:
            break;
    }
    return 0;
}

void set_flags(CPU *cpu, const uint16_t value, const uint8_t values[2]) {
    cpu->FR.Z = value == 0;
    cpu->FR.N = (value & 0x80) != 0;
    cpu->FR.C = value > 255;
    if (values[0] > 0 && values[1] > 0) cpu->FR.O = (value & 0x80) != 0;
    else if (values[0] < 0 && values[1] < 0) cpu->FR.O = (value & 0x80) == 0;
}

void decode_reg_reg(CPU *cpu, uint16_t *values) {
    uint8_t tmp = fetch_byte(cpu);
    values[0] = tmp & 0x0F; // reg 1
    values[1] = (tmp >> 4) & 0x0F; // reg 2
}

void decode_reg_imm(CPU *cpu, uint16_t *values) {
    values[0] = fetch_byte(cpu) & 0xF; // register
    values[1] = fetch_byte(cpu); // immediate
}

void decode_reg_abs(CPU *cpu, uint16_t *values) {
    values[0] = fetch_byte(cpu) & 0xF; // register
    values[1] = fetch_word(cpu); // absolute
}

void decode_reg_indreg(CPU *cpu, uint16_t *values) {
    values[0] = fetch_byte(cpu) & 0xF;
    const uint8_t tmp = fetch_byte(cpu);
    values[1] = tmp & 0x0F;         // idx reg low
    values[2] = (tmp >> 4) & 0x0F;  // idx reg high
}

void decode_reg_idx(CPU *cpu, uint16_t *values) {
    values[0] = fetch_byte(cpu) & 0xF;  // reg 1
    const uint8_t tmp = fetch_byte(cpu);
    values[1] = tmp & 0x0F;             // idx reg low
    values[2] = (tmp >> 4) & 0x0F;      // idx reg high
    values[3] = fetch_word(cpu);        // index
}

void decode_abs_reg(CPU *cpu, uint16_t *values) {
    values[0] = fetch_word(cpu); // absolute
    values[1] = fetch_byte(cpu) & 0xF; // register
}

void decode_indreg_reg(CPU *cpu, uint16_t *values) {
    const uint8_t tmp = fetch_byte(cpu);
    values[0] = tmp & 0x0F; // idx reg low
    values[1] = (tmp >> 4) & 0x0F; // idx reg high
    values[2] = fetch_byte(cpu) & 0xF;
}

void decode_idx_reg(CPU *cpu, uint16_t *values) {
    const uint8_t tmp = fetch_byte(cpu);
    values[0] = tmp & 0x0F;             // idx reg low
    values[1] = (tmp >> 4) & 0x0F;      // idx reg high
    values[2] = fetch_word(cpu);        // index
    values[3] = fetch_byte(cpu) & 0xF;  // reg 1
}

void decode_reg(CPU *cpu, uint16_t *values) {
    values[0] = fetch_byte(cpu) & 0xF;
}

void decode_imm(CPU *cpu, uint16_t *values) {
    values[0] = fetch_byte(cpu);
}

void decode_abs(CPU *cpu, uint16_t *values) {
    values[0] = fetch_word(cpu);
}

void decode_indreg(CPU *cpu, uint16_t *values) {
    const uint8_t tmp = fetch_byte(cpu);
    values[0] = tmp & 0x0F;             // idx reg low
    values[1] = (tmp >> 4) & 0x0F;      // idx reg high
}

void CPU_init(CPU *cpu) {
    memory_init(&cpu->memory);
    memset(&cpu->ports, 0, sizeof(cpu->ports));
}

void reset(CPU *cpu) {
    cpu->R0 = cpu->R1 = cpu->R2 = cpu->R3 = 0;
    cpu->PC = 0xFFF0; // set PC to reset vector location
    cpu->SP = cpu->BP = 0x0100; // SP and BP set to arbitrary values for testing
    cpu->FR.flags = 0x0;
    // get jump address
    const uint16_t address = fetch_word(cpu);
    cpu->PC = address;
}

void execute(CPU *cpu) {
    while (1) {
        const uint8_t instruction = fetch_byte(cpu);
        const uint8_t base_opcode = (instruction & 0b11) == 0 ? instruction & 0b11100011 : instruction & 0b11110011;
        switch (base_opcode) {
            case MOV:
                execute_mov(cpu, instruction);
                break;
            case PUSH:
                break;
            case HLT:
                return;
            default:
                printf("Instruction not handled\nOpcode: %02x", instruction);
                return;
        }
    }
}
