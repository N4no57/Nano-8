#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "flags.h"
#include "memory.h"

typedef struct {
    uint16_t PC;
    uint16_t SP;
    uint16_t BP;
    uint8_t R0, R1, R2, R3, H, L;
    flags FR;

    Memory memory;
    uint8_t ports[256];
} CPU;

void CPU_init(CPU *cpu);
void reset(CPU *cpu);

#endif //CPU_H
