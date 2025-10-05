#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "flags.h"
#include "memory.h"
#include "floppyDiskController.h"

#define MAX_PORTS 256

enum base_ops {
    MOV = 0x00,
    PUSH = 0x20,
    POP = 0x40,
    INB = 0x60,
    OUTB = 0x80,
    MOV_SP = 0xA0,
    ADD = 0x01,
    SUB = 0x11,
    CMP = 0x21,
    INC = 0x31,
    DEC = 0x41,
    MUL = 0x51,
    DIV = 0x61,
    AND = 0x71,
    OR = 0x81,
    XOR = 0x91,
    NOT = 0xA1,
    SHL = 0xB1,
    SHR = 0xC1,
    JMP = 0x02,
    JZ = 0x12,
    JNZ = 0x22,
    JC = 0x32,
    JNC = 0x42,
    JO = 0x52,
    JNO = 0x62,
    JN = 0x72,
    JNN = 0x82,
    CALL = 0x92,
    RET = 0xA2,
    HLT = 0x03,
    NOP = 0x13,
    CLI = 0x23,
    STI = 0x33,
    IRET = 0x43,
};

typedef struct {
    uint16_t PC;
    uint16_t SP;
    uint16_t BP;
    uint8_t R0, R1, R2, R3, H, L;
    flags FR;

    Memory memory;
    uint8_t ports[MAX_PORTS];

    FDC floppy_controller;
} CPU;

void CPU_init(CPU *cpu);
void reset(CPU *cpu);
void exec_inst(CPU *cpu);
void execute(CPU *cpu);

#endif //CPU_H
