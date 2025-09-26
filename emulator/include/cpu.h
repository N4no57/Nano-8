#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "flags.h"
#include "memory.h"

#define MAX_PORTS 256

enum instruction {
    MOV_REG_REG = 0x00,
    MOV_REG_IMM = 0x04,
    HLT = 0x03,
};

typedef struct {
    uint16_t PC;
    uint16_t SP;
    uint16_t BP;
    uint8_t R0, R1, R2, R3, H, L;
    flags FR;

    Memory memory;
    uint8_t ports[MAX_PORTS];
} CPU;

void CPU_init(CPU *cpu);
void reset(CPU *cpu);
void execute(CPU *cpu);

#endif //CPU_H
