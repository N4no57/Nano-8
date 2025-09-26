#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEMORY_SIZE 0x10000

typedef struct {
    uint8_t data[MEMORY_SIZE];
} Memory;

void memory_init(Memory *memory);
uint8_t readByte(Memory *memory, uint16_t address);
void writeByte(Memory *memory, uint16_t address, uint8_t data);

#endif //MEMORY_H
