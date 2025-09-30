#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEMORY_SIZE 0x10000

typedef struct {
    uint8_t data[MEMORY_SIZE];
} Memory;

void memory_init(Memory *memory);

uint8_t read_byte(const Memory *memory, uint8_t address);
uint16_t read_word(const Memory *memory, uint8_t address);

void write_byte(Memory *memory, uint8_t address, uint8_t value);
void write_word(Memory *memory, uint8_t address, uint16_t value);

#endif //MEMORY_H
