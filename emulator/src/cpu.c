#include "../include/cpu.h"
#include <string.h>
#include <stdio.h>

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
    uint8_t dst;
    uint8_t value;
    uint8_t src;
    uint8_t low;
    uint8_t high;
    uint16_t address;
    while (1) {
        const uint8_t instruction = fetch_byte(cpu, cpu->PC++);
        switch (instruction) {
            case MOV_REG_REG: // MOV Rdest, Rsrc
                const uint8_t registers = fetch_byte(cpu, cpu->PC++);
                src = read_reg(cpu, registers & 0x0F);
                set_reg(cpu, (registers & 0xF0) >> 4, src);
                break;
            case MOV_REG_IMM: // MOV Rdest, imm8
                dst = fetch_byte(cpu, cpu->PC++) & 0xF0;
                value = fetch_byte(cpu, cpu->PC++);
                set_reg(cpu, dst, value);
                break;
            case MOV_REG_ABS: // MOV Rdest, imm16
                dst = fetch_byte(cpu, cpu->PC++) & 0xF0;
                low = fetch_byte(cpu, cpu->PC++);
                high = fetch_byte(cpu, cpu->PC++);
                address = ((uint16_t)high << 8) | low;
                value = fetch_byte(cpu, address);
                set_reg(cpu, dst, value);
                break;
            case MOV_REG_IND:
                dst = fetch_byte(cpu, cpu->PC++) & 0xF0;
                src = fetch_byte(cpu, cpu->PC++);
                low = read_reg(cpu, src & 0x0F);
                high = read_reg(cpu, (src & 0xF0) >> 4);
                address = ((uint16_t)high << 8) | low;
                value = fetch_byte(cpu, address);
                set_reg(cpu, dst, value);
                break;
            case MOV_REG_IDX:
                dst = fetch_byte(cpu, cpu->PC++) & 0xF0;
                src = fetch_byte(cpu, cpu->PC++);
                low = read_reg(cpu, src & 0x0F);
                high = read_reg(cpu, (src & 0xF0) >> 4);
                address = ((uint16_t)high << 8) | low;
                low = fetch_byte(cpu, cpu->PC++);
                high = fetch_byte(cpu, cpu->PC++);
                address += ((uint16_t)high << 8) | low;
                value = fetch_byte(cpu, address);
                set_reg(cpu, dst, value);
                break;
            case MOV_ABS_REG:
                low = fetch_byte(cpu, cpu->PC++);
                high = fetch_byte(cpu, cpu->PC++);
                address = ((uint16_t)high << 8) | low;
                src = fetch_byte(cpu, cpu->PC++) & 0xF0;
                value = read_reg(cpu, src >> 4);
                set_byte(cpu, address, value);
                break;
            case MOV_IND_REG:
                dst = fetch_byte(cpu, cpu->PC++);
                src = fetch_byte(cpu, cpu->PC++) & 0xF0;
                low = read_reg(cpu, dst & 0x0F);
                high = read_reg(cpu, (dst & 0xF0) >> 4);
                address = ((uint16_t)high << 8) | low;
                value = read_reg(cpu, src >> 4);
                set_byte(cpu, address, value);
                break;
            case MOV_IDX_REG:
                dst = fetch_byte(cpu, cpu->PC++);
                src = fetch_byte(cpu, cpu->PC++) & 0xF0;
                low = read_reg(cpu, dst & 0x0F);
                high = read_reg(cpu, (dst & 0xF0) >> 4);
                address = ((uint16_t)high << 8) | low;
                low = fetch_byte(cpu, cpu->PC++);
                high = fetch_byte(cpu, cpu->PC++);
                address += ((uint16_t)high << 8) | low;
                value = read_reg(cpu, src >> 4);
                set_byte(cpu, address, value);
                break;
            case HLT:
                return;
            default:
                printf("Instruction not handled\nOpcode: %02x", instruction);
                return;
        }
    }
}
