#include <stdlib.h>
#include <string.h>

#include "../include/cpu.h"
#include "../include/emulatorTests.h"

int main() {
    //test_emulator();

    CPU cpu;
    CPU_init(&cpu);

    FloppyDisk disk = {0};

    FDC controller = {0};

    cpu.floppy_controller = controller;

    cpu.floppy_controller.phase = PHASE_CMD;

    disk.sector_size = 512;
    disk.sectors = 18;
    disk.tracks = 80;
    disk.heads = 2;
    disk.filename = "floppy.img";
    disk.size = disk.sector_size * disk.sectors * disk.tracks * disk.heads;
    disk.data = malloc(disk.size);
    memset(disk.data, 0, disk.size);

    mount_floppy_disk(&cpu.floppy_controller, &disk, 0);



    FILE *f = fopen("rombin.bin", "rb");
    fread(&cpu.memory.data[0x8000], 1, 0xFFFF-0x8000, f);

    reset(&cpu);

    while (1) {
        tick_fdc(&cpu.floppy_controller);
        exec_inst(&cpu);
    }
  return 0;
}