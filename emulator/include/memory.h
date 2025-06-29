#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

typedef struct {
    uint8_t data[1024 * 64];
} Memory;

void memory_init(Memory *memory);

#endif //MEMORY_H
