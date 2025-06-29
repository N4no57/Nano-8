#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

typedef struct {
    uint8_t data[1024 * 64];
} Memory;

void memory_init(Memory *memory);
uint8_t readByte(Memory *memory, uint16_t address);
void writeByte(Memory *memory, uint16_t address, uint8_t data);

#endif //MEMORY_H
