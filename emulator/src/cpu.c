#include "../include/cpu.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

void set_flags(CPU *cpu, const uint16_t value, const uint8_t values[2], const uint8_t mask) {
    // mask = 0b0000, 1 - O, 2 - C, 3 - N, 4 - Z
    if ((mask & 0b1) == 0) cpu->FR.Z = value == 0;
    if ((mask & 0b10) == 0) cpu->FR.N = (value & 0x80) != 0;
    if ((mask & 0b100) == 0) cpu->FR.C = value > 255;
    if ((mask & 0b1000) == 0) {
        if (values[0] > 0 && values[1] > 0) cpu->FR.O = (value & 0x80) != 0;
        else if (values[0] < 0 && values[1] < 0) cpu->FR.O = (value & 0x80) == 0;
    }
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

void execute_mov(CPU *cpu, const uint8_t instruction) {
    const uint8_t type = instruction & 0b00011100;
    if (type == 0) { // mov reg, reg
        uint16_t values[2]; // 0: dest, 1: src
        decode_reg_reg(cpu, values);
        set_reg(cpu, values[0], read_reg(cpu, values[1]));
    } else if (type == 0b001 << 2) { // mov reg, #imm
        uint16_t values[2]; // 0: dest, 1: src
        decode_reg_imm(cpu, values);
        set_reg(cpu, values[0], values[1]);
    } else if (type == 0b010 << 2) { // mov reg, abs
        uint16_t values[2];
        decode_reg_abs(cpu, values);
        set_reg(cpu, values[0], read_byte(&cpu->memory, values[1]));
    } else if (type == 0b011 << 2) { // mov reg, (h(igh)Reg, l(ow)Reg)
        uint16_t values[3]; // 0: dest, 1: low reg, 2: high reg
        decode_reg_indreg(cpu, values);
        set_reg(cpu, values[0],
            read_byte(&cpu->memory, read_reg(cpu, values[1]) | read_reg(cpu, values[2]) << 8));
    } else if (type == 0b100 << 2) { // mov reg, [(h(igh)Reg, l(ow)Reg)±offset]
        uint16_t values[4]; // 0: dest, 1: low reg, 2: high reg, 3: offset
        decode_reg_idx(cpu, values);
        const uint16_t address = (read_reg(cpu, values[1]) | read_reg(cpu, values[2]) << 8) + (int16_t)values[3];
        set_reg(cpu, address, read_byte(&cpu->memory, address));
    } else if (type == 0b101 << 2) { // mov abs, reg
        uint16_t values[2]; // 0: abs, 1: reg
        decode_abs_reg(cpu, values);
        write_byte(&cpu->memory, values[0], read_reg(cpu, values[1]));
    } else if (type == 0b110 << 2) { // mov (h(igh)Reg, l(ow)Reg), reg
        uint16_t values[3]; // 0: low reg, 1: high reg 2: reg
        decode_indreg_reg(cpu, values);
        write_byte(&cpu->memory,read_reg(cpu, values[0])
                 + read_reg(cpu, values[1]), read_reg(cpu, values[2]));
    } else if (type == 0b111 << 2) { // mov [(h(igh)Reg, l(ow)Reg)±offset], reg
        uint16_t values[4]; // 0: low reg, 1: high reg, 2: offset, 3: dest
        decode_idx_reg(cpu, values);
        const uint16_t address = (read_reg(cpu, values[0]) | read_reg(cpu, values[1]) << 8) + (int16_t)values[2];
        write_byte(&cpu->memory, address, read_reg(cpu, values[3]));
    }
}

void execute_complexArith(CPU *cpu, const uint8_t instruction, const uint8_t arith_type, const uint8_t flag_mask) {
    // arith_types:
    // 0 - add,
    // 1 - sub,
    // 2 - mul,
    // 3 - div
    // 4 - and
    // 5 - or
    // 6 - xor
    // 7 - cmp
    const uint8_t type = instruction & 0b00001100;
    uint16_t values[2];
    int8_t operand[2];
    if (type == 0) { // inst reg_1, reg_2
        decode_reg_reg(cpu, values);
        operand[1] = (int8_t)read_reg(cpu, values[1]);
    } else if (type == 0b01 << 2) {
        decode_reg_imm(cpu, values);
        operand[1] = (int8_t)values[1];
    }
    operand[0] = read_reg(cpu, values[0]); // NOLINT(*-narrowing-conversions)
    uint16_t result;
    switch (arith_type) {
        case 0:
            result = operand[0] + operand[1];
            break;
        case 1:
            result = operand[0] - operand[1];
            break;
        case 2:
            result = operand[0] * operand[1];
            break;
        case 3:
            result = operand[0] / operand[1];
            break;
        case 4:
            result = operand[0] & operand[1];
            break;
        case 5:
            result = operand[0] | operand[1];
            break;
        case 6:
            result = operand[0] ^ operand[1];
            break;
        case 7:
            result = operand[0] - operand[1];
            break;
        default:
            printf("arithmetic type error\n");
            exit(10);
    }

    if (arith_type != 7) set_reg(cpu, values[0], result);
    set_flags(cpu, result, operand, flag_mask);
}

void execute_simpleArith(CPU *cpu, const uint8_t arith_type, const uint8_t flag_mask) {
    // arith_types:
    // 0 - inc,
    // 1 - dec,
    // 2 - shl,
    // 3 - shr,
    // 4 - not
    uint16_t values[1];
    decode_reg(cpu, values);

    const uint8_t reg_val = read_reg(cpu, values[0]);
    uint16_t result;
    switch (arith_type) {
        case 0:
            result = reg_val + 1;
            break;
        case 1:
            result = reg_val - 1;
            break;
        case 2:
            result = reg_val << 1;
            break;
        case 3:
            result = reg_val >> 1;
            break;
        case 4:
            result = !reg_val;
            break;
        default:
            printf("arithmetic type error\n");
            exit(20);
    }

    set_reg(cpu, values[0], result);
    set_flags(cpu, result, &reg_val, flag_mask);
}

void execute_jmp(CPU *cpu, const uint8_t instruction, uint8_t push_old_pc) {
    const uint8_t type = instruction & 0b0001100;

    uint16_t values[2];
    uint16_t address = 0;
    if (type == 0) { // absolute
        decode_abs(cpu, values);
        address = cpu->PC;
        cpu->PC = values[0];
    } else if (type == 0b01 << 2) { // indirect reg
        decode_indreg(cpu, values);
        address = cpu->PC;
        cpu->PC = read_reg(cpu, values[1]) << 8 | read_reg(cpu, values[0]);
    } else if (type == 0b10 << 2) { // relative
        decode_imm(cpu, values);
        address = cpu->PC;
        cpu->PC += (int8_t)values[0];
    } else if (type == 0b11 << 2) { // indirect mem
        decode_abs(cpu, values);
        address = cpu->PC;
        cpu->PC = read_byte(&cpu->memory, values[0]);
    }
    if (push_old_pc) {
        cpu->SP += -2;
        write_word(&cpu->memory, --cpu->SP, address);
    }
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
                const uint8_t type = instruction & 0b0001100;

                if (type == 0) { // register
                    write_byte(&cpu->memory, --cpu->SP, read_reg(cpu, fetch_byte(cpu) >> 4));
                } else if (type == 0b01 << 2) { // immediate
                    write_byte(&cpu->memory, --cpu->SP, fetch_byte(cpu));
                }
                break;
            case POP:
                set_reg(cpu, fetch_byte(cpu), read_byte(&cpu->memory, cpu->SP++));
                break;
            case INB:
                uint8_t port = fetch_byte(cpu);
                uint8_t reg = fetch_byte(cpu) >> 4;
                set_reg(cpu, reg, cpu->ports[port]);
                break;
            case OUTB: // TODO
                break;
            case ADD:
                execute_complexArith(cpu, instruction, 0, 0b1111);
                break;
            case SUB:
                execute_complexArith(cpu, instruction, 1, 0b1111);
                break;
            case CMP:
                execute_complexArith(cpu, instruction, 7, 0b1111);
                break;
            case INC:
                execute_simpleArith(cpu, 0, 0b0111);
                break;
            case DEC:
                execute_simpleArith(cpu, 1, 0b0011);
                break;
            case MUL:
                execute_complexArith(cpu, instruction, 2, 0b0111);
                break;
            case DIV:
                execute_complexArith(cpu, instruction, 3, 0b0011);
                break;
            case AND:
                execute_complexArith(cpu, instruction, 4, 0b0011);
                break;
            case OR:
                execute_complexArith(cpu, instruction, 5, 0b0011);
                break;
            case XOR:
                execute_complexArith(cpu, instruction, 6, 0b0011);
                break;
            case NOT:
                execute_simpleArith(cpu, 4, 0b0011);
                break;
            case SHL:
                execute_simpleArith(cpu, 2, 0b0111);
                break;
            case SHR:
                execute_simpleArith(cpu, 3, 0b0111);
                break;
            case JMP:
                execute_jmp(cpu, instruction, 0);
                break;
            case JZ:
                if (cpu->FR.Z) execute_jmp(cpu, instruction, 0);
                break;
            case JNZ:
                if (!cpu->FR.Z) execute_jmp(cpu, instruction, 0);
                break;
            case JC:
                if (cpu->FR.C) execute_jmp(cpu, instruction, 0);
                break;
            case JNC:
                if (!cpu->FR.C) execute_jmp(cpu, instruction, 0);
                break;
            case JO:
                if (cpu->FR.O) execute_jmp(cpu, instruction, 0);
                break;
            case JNO:
                if (!cpu->FR.O) execute_jmp(cpu, instruction, 0);
                break;
            case JN:
                if (cpu->FR.N) execute_jmp(cpu, instruction, 0);
                break;
            case JNN:
                if (!cpu->FR.N) execute_jmp(cpu, instruction, 0);
                break;
            case CALL:
                execute_jmp(cpu, instruction, 1);
                break;
            case RET:
                // assumes return address is at the top of the stack
                cpu->PC = read_word(&cpu->memory, cpu->SP);
                cpu->SP+=2;
                break;
            case HLT:
                return;
            case NOP:
                break;
            case CLI:
                cpu->FR.I = 0;
                break;
            case STI:
                cpu->FR.I = 1;
                break;
            case IRET: // TODO
                break;
            default:
                printf("Instruction not handled\nOpcode: %02x", instruction);
                return;
        }
    }
}
