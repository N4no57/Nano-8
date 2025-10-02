//
// Created by Bernardo on 02/10/2025.
//

#include <string.h>

#include "../include/emulatorTests.h"
#include "../include/cpu.h"

void test_mov() {
    CPU cpu;
    CPU_init(&cpu);
    cpu.PC = 0x0000;

    uint8_t program[] {
        0x04, 0x00, 0x10,       // mov r0, #$10
        0x00, 0x20,             // mov r2, r0
        0x08, 0x10, 0x0D, 0x00, // mov r1, $D
        0x0C, 0x30, 0x21,       // mov r3, (r2, r1)
        0x03,
        0x10
    };

    memcpy(cpu.memory.data, program, sizeof(program));

    execute(&cpu);
}

void test_emulator() {
    test_mov();
}
