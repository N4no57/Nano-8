//
// Created by brego on 01/10/2025.
//

#ifndef LINKERFILEPARSER_H
#define LINKERFILEPARSER_H

#include <stdint.h>

struct MemoryRegion {
    char name[16];
    uint32_t start;
    uint32_t size;
    int fill;
    uint32_t current_offset;
};

struct SegmentRule {
    char name[16];
    char load_to[16];  // region name
    uint32_t explicit_start; // 0 if none
};

extern int memRegion_count;
extern int segRule_count;

void parseFile(char *fileName, struct MemoryRegion **memRegions, struct SegmentRule **rules);

#endif //LINKERFILEPARSER_H
