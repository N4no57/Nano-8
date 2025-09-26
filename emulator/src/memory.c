#include "../include/memory.h"
#include <string.h>

void memory_init(Memory *memory) {
    memset(memory, 3, sizeof(Memory));
}
