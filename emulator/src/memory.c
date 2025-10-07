#include "../include/memory.h"
#include "../include/videoCard.h"
#include <string.h>

void memory_init(Memory *memory) {
    memset(memory, 3, sizeof(Memory));
}

uint8_t read_byte(const Memory *memory, const VideoCard *video, const uint16_t address) {
    if (address > 0x2000 && 0x20405 >= address) {
        if (address > 0x2000 && 0x203FF > address) {
            uint16_t index = address - 0x2000;
            return video->VRAMpage[index];
        }
        if (address == 0x2400) {
            return video->indexRegister;
        }
        if (address == 0x2401) {
            return video->modeRegister;
        }
        if (address == 0x2402) {
            return video->palleteIndexRegister;
        }
        if (address == 0x2403) {
            return video->palleteDataRegister;
        }
        if (address == 0x2404) {
            return video->statusRegister;
        }
        if (address == 0x2405) {
            return video->controlRegister;
        }
    }

    return memory->data[address];
}

uint16_t read_word(const Memory *memory, const uint16_t address) {
    return memory->data[address] | memory->data[address + 1] << 8;
}

void write_byte(Memory *memory, VideoCard *video, const uint16_t address, const uint8_t value) {
    if (address > 0x2000 && 0x20405 >= address) {
        if (address > 0x2000 && 0x203FF > address) {
            const uint16_t index = address - 0x2000;
            video->VRAMpage[index] = value;
        }
        if (address == 0x2400) {
            video->indexRegister = value;
            video->VRAMpage = video->VRAM[value];
        }
        if (address == 0x2401) {
            video->modeRegister = value;
        }
        if (address == 0x2402) {
            video->palleteIndexRegister = value;
        }
        if (address == 0x2403) {
            video->palleteDataRegister = value;
        }
        if (address == 0x2405) {
            video->controlRegister = value;
        }
        return;
    }

    memory->data[address] = value;
}

void write_word(Memory *memory, const uint16_t address, const uint16_t value) {
    memory->data[address] = value & 0xFF;
    memory->data[address + 1] = value >> 8 & 0xFF;
}