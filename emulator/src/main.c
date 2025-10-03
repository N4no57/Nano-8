#include <stdlib.h>

#include "../include/cpu.h"
#include "../include/emulatorTests.h"

int main() {
  test_emulator();

  CPU cpu;
  CPU_init(&cpu);

  FloppyDisk disk = {0};

  FDC controller = {0};

  disk.sector_size = 512;
  disk.sectors = 18;
  disk.tracks = 80;
  disk.heads = 2;
  disk.size = disk.sector_size * disk.sectors * disk.tracks * disk.heads;
  disk.data = malloc(disk.size);

  mount_floppy_disk(&controller, &disk, 0);

  cpu.floppy_controller = controller;

  cpu.memory.data[0xFFF0] = 0x00; // reset vec
  cpu.memory.data[0xFFF1] = 0x10;

  reset(&cpu);

  // inline program injection
  cpu.memory.data[0x1000] = 0x04;
  cpu.memory.data[0x1001] = 0x00;
  cpu.memory.data[0x1002] = 0x0A;
  cpu.memory.data[0x1003] = 0x03;
  // end - inline program injection

  exec_inst(&cpu);

  return 0;
}