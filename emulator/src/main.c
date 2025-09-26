#include "../include/cpu.h"

int main() {
  CPU cpu;
  CPU_init(&cpu);

  cpu.memory.data[0xFFF0] = MOV_REG_IMM;
  cpu.memory.data[0xFFF1] = 0x00;
  cpu.memory.data[0xFFF2] = 32;
  cpu.memory.data[0xFFF3] = 0x03; // HLT

  execute(&cpu);

  return 0;
}