#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEMORY_SIZE 0x10000

typedef struct {
    uint8_t data[MEMORY_SIZE];
} Memory;

void memory_init(Memory *memory);

#endif //MEMORY_H
