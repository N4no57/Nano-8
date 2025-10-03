#include "../include/cpu.h"
#include "../include/emulatorTests.h"

int main() {
  test_emulator();

  CPU cpu;
  CPU_init(&cpu);

  cpu.memory.data[0xFFF0] = 0x00; // reset vec
  cpu.memory.data[0xFFF1] = 0x10;

  reset(&cpu);

  // inline program injection
  cpu.memory.data[0x1000] = 0x04;
  cpu.memory.data[0x1001] = 0x00;
  cpu.memory.data[0x1002] = 0x0A;
  cpu.memory.data[0x1003] = 0x03;
  // end - inline program injection

  execute(&cpu);

  return 0;
}