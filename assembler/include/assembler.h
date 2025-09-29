//
// Created by Bernardo on 29/09/2025.
//

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdbool.h>

// assembler flags
extern bool objDump;

// assembler
int assemble(const char *input, const char *output);

#endif //ASSEMBLER_H
