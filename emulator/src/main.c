#include <stdlib.h>
#include <string.h>

#include "../include/cpu.h"
#include "../include/emulatorTests.h"
#include "../include/videoCard.h"

void init_paletteRAM(VideoCard *card) {
    uint8_t palette[64] = {0};

    for (int i = 0; i < sizeof(palette); i++) {
        palette[i] = i;
    }

    memcpy(card->palleteRAM, palette, sizeof(palette));
}

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

    VideoCard video_card;

    video_card.width = 320;
    video_card.height = 200;

    video_card.modeRegister = 0;

    videoCardInit(&video_card);
    init_paletteRAM(&video_card);

    unsigned char str[] = "Hello World!";
    uint8_t color_entry = 0b11110000;
    uint16_t entry[strlen(str)];

    for (int i = 0; i < strlen(str); i++) {
        entry[i] = str[i] | color_entry << 8;
    }

    memcpy(&video_card.VRAM[0][0], entry, sizeof(entry));

    video_card.palleteRAM[0b00001111] = 0xFF;

    while (1) {
        tick_fdc(&cpu.floppy_controller);
        videoCardTick(&video_card);
        exec_inst(&cpu);
    }
  return 0;
}