#include "../include/memory.h"
#include <string.h>

void memory_init(Memory *memory) {
    memset(memory, 3, sizeof(Memory));
}

uint8_t read_byte(const Memory *memory, const uint8_t address) {
    return memory->data[address];
}

uint16_t read_word(const Memory *memory, const uint8_t address) {
    return memory->data[address] | memory->data[address + 1] << 8;
}

void write_byte(Memory *memory, const uint8_t address, const uint8_t value) {
    memory->data[address] = value;
}

void write_word(Memory *memory, const uint8_t address, const uint16_t value) {
    memory->data[address] = value & 0xFF;
    memory->data[address + 1] = value >> 8 & 0xFF;
}