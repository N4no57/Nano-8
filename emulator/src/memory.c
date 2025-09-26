#include "../include/memory.h"

void memory_init(Memory *memory) {
    for (int i = 0; i < MEMORY_SIZE; i++) {
        memory->data[i] = 0x03;
    }
}
