#include "../include/cpu.h"

int main() {
  CPU cpu;
  CPU_init(&cpu);

  cpu.memory.data[0xFFF0] = 0x00; // reset vec
  cpu.memory.data[0xFFF1] = 0x10;

  reset(&cpu);

  // inline program injection
  cpu.memory.data[0x1000] = ADD_REG_REG;
  cpu.memory.data[0x1001] = 0x01;
  // end - inline program injection

  cpu.R0 = 0x7F;
  cpu.R1 = 0x01;

  execute(&cpu);

  return 0;
}