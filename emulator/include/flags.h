#ifndef FLAGS_H
#define FLAGS_H

#include <stdint.h>

typedef union {
    struct {
        uint8_t C : 1;
        uint8_t Z : 1;
        uint8_t O : 1;
        uint8_t N : 1;
        uint8_t I : 1;
    };
    uint8_t flags;
} flags;

#endif //FLAGS_H
