#include "../include/cpu.h"
#include <string.h>

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
