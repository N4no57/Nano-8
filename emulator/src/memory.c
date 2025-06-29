#include "../include/memory.h"
#include <string.h>

void memory_init(Memory *memory) {
    memset(memory, 0, sizeof(Memory));
}

uint8_t readByte(Memory *memory, uint16_t address) {
    return memory->data[address];
}

void writeByte(Memory *memory, uint16_t address, uint8_t data) {
    memory->data[address] = data;
}
