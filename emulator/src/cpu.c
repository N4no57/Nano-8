#include "../include/cpu.h"
#include <string.h>

uint8_t fetch_byte(const CPU *cpu, const uint16_t addr) {
    return cpu->memory.data[addr];
}

void set_byte(CPU *cpu, const uint16_t addr, const uint8_t value) {
    cpu->memory.data[addr] = value;
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
        case 0xA: // PC
            return cpu->PC;
        case 0xB: // SP
            return cpu->SP;
        case 0xC: // BP
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

void CPU_init(CPU *cpu) {
    memory_init(&cpu->memory);
    reset(cpu);
    memset(&cpu->ports, 0, sizeof(cpu->ports));
}

void reset(CPU *cpu) {
    cpu->R0 = cpu->R1 = cpu->R2 = cpu->R3 = 0;
    cpu->PC = 0xFFF0; // set PC to reset vector location
    cpu->SP = cpu->BP = 0x0100; // SP and BP set to arbitrary values for testing
    cpu->FR.flags = 0x0;
}

void execute(CPU *cpu) {
    while (1) {
        const uint8_t instruction = fetch_byte(cpu, cpu->PC++);
        switch (instruction) {
            case MOV_REG_REG: // MOV Rdest, Rsrc
                const uint8_t registers = fetch_byte(cpu, cpu->PC++);
                const uint8_t src = read_reg(cpu, registers & 0x0F);
                set_reg(cpu, registers & 0xF0, src);
            break;
            case MOV_REG_IMM: // MOV Rdest, imm8
                const uint8_t dst = fetch_byte(cpu, cpu->PC++) & 0xF0;
                const uint8_t value = fetch_byte(cpu, cpu->PC++);
                set_reg(cpu, dst, value);
            break;
            case 0x03:
                return;
            default:
                break;
        }
    }
}
