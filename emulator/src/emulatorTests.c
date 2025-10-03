//
// Created by Bernardo on 02/10/2025.
//

#include <string.h>

#include "../include/emulatorTests.h"
#include "../include/cpu.h"

int test_mov_reg_reg(void) {
    CPU cpu;
    CPU_init(&cpu);
    cpu.PC = 0x0000;

    uint8_t program[] = {
        0x00, 0x20,
        0x03
    };

    cpu.R0 = 0x10;

    memcpy(cpu.memory.data, program, sizeof(program));

    execute(&cpu);

    ASSERT_EQ(cpu.R0, cpu.R2, "R2 should be equal R0");
    printf("PASS: MOV_reg_reg\n");

    return 0;
}

int test_mov_reg_imm(void) {
    CPU cpu;
    CPU_init(&cpu);
    cpu.PC = 0x0000;

    uint8_t program[] = {
        0x04, 0x00, 0x10,
        0x03
    };

    memcpy(cpu.memory.data, program, sizeof(program));

    execute(&cpu);

    ASSERT_EQ(cpu.R0, 16, "R0 should be 0x10");
    printf("PASS: MOV_reg_imm\n");

    return 0;
}

int test_mov_reg_abs(void) {
    CPU cpu;
    CPU_init(&cpu);
    cpu.PC = 0x0000;

    uint8_t program[] = {
        0x08, 0x10, 0x0D, 0x00,
        0x03
    };

    memcpy(cpu.memory.data, program, sizeof(program));

    cpu.memory.data[0x0D] = 0x15;

    execute(&cpu);

    ASSERT_EQ(cpu.R1, cpu.memory.data[0x0D], "R1 should be equal -address 0xD");
    printf("PASS: MOV_reg_abs\n");

    return 0;
}

int test_mov_reg_ind(void) {
    CPU cpu;
    CPU_init(&cpu);
    cpu.PC = 0x0000;

    uint8_t program[] = {
        0x0C, 0x30, 0x10,
        0x03,
    };

    cpu.R0 = 0x0F;
    cpu.R1 = 0x10;

    memcpy(cpu.memory.data, program, sizeof(program));

    cpu.memory.data[0x100F] = 0xAF;

    execute(&cpu);

    ASSERT_EQ(cpu.R3, cpu.memory.data[0x100F], "R3 should be equal to address 0x100F");
    printf("PASS: MOV_reg_ind\n");

    return 0;
}

int test_mov_reg_idx(void) {
    CPU cpu;
    CPU_init(&cpu);
    cpu.PC = 0x0000;

    cpu.R0 = cpu.R1 = 0x10;

    uint8_t program[] = {
        0x10, 0x30, 0x10, 0xFB, 0xFF,
        0x03,
    };

    memcpy(cpu.memory.data, program, sizeof(program));

    cpu.memory.data[0x100B] = 0xBA;

    execute(&cpu);

    ASSERT_EQ(cpu.R3, cpu.memory.data[0x100B], "R3 should be equal to memory address 0x100B");
    printf("PASS: MOV_reg_idx\n");

    return 0;
}

int test_mov_abs_reg(void) {
    CPU cpu;
    CPU_init(&cpu);
    cpu.PC = 0x0000;

    uint8_t program[] = {
        0x14, 0x00, 0x10, 0x20,
        0x03,
    };

    memcpy(cpu.memory.data, program, sizeof(program));

    cpu.R2 = 0x31;

    execute(&cpu);

    ASSERT_EQ(cpu.memory.data[0x1000], cpu.R2, "mem address 0x1000 should be R2");
    printf("PASS: MOV_abs_reg\n");

    return 0;
}

int test_mov_ind_reg(void) {
    CPU cpu;
    CPU_init(&cpu);
    cpu.PC = 0x0000;

    uint8_t program[] = {
        0x18, 0x10, 0x20,
        0x03
    };

    memcpy(cpu.memory.data, program, sizeof(program));

    cpu.R0 = cpu.R1 = 0x10;

    cpu.R2 = 0x14;

    execute(&cpu);

    ASSERT_EQ(cpu.memory.data[0x1010], cpu.R2, "mem address 0x1010 should be equal to R2");
    printf("PASS: MOV_ind_reg\n");

    return 0;
}

int test_mov_idx_reg(void) {
    CPU cpu;
    CPU_init(&cpu);
    cpu.PC = 0x0000;

    uint8_t program[] = {
        0x1C, 0x10, 0x0A, 0x00, 0x20,
        0x03,
    };

    memcpy(cpu.memory.data, program, sizeof(program));

    cpu.memory.data[0x1000] = 0xBA;

    cpu.R2 = 0xDE;

    cpu.R0 = cpu.R1 = 0x10;

    execute(&cpu);

    ASSERT_EQ(cpu.memory.data[0x101A], 0xDE, "mem address 0x101A should be 0xDE");
    printf("PASS: MOV_idx_reg\n");

    return 0;
}

int test_mov() {
    test_mov_reg_reg();
    test_mov_reg_imm();
    test_mov_reg_abs();
    test_mov_reg_ind();
    test_mov_reg_idx();
    test_mov_abs_reg();
    test_mov_ind_reg();
    test_mov_idx_reg();

    return 0;
}

void test_emulator() {
    test_mov();
}
