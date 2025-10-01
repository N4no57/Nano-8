//
// Created by brego on 29/09/2025.
//

#ifndef LINKER_H
#define LINKER_H

#include "objectFileReader.h"
#include "flags.h"
#include "linkerFileParser.h"

#include <stdint.h>

struct LinkedSegment {
    char name[16];
    uint32_t size;
    uint32_t base_address;  // where it will start in final binary
    uint8_t *data;          // concatenated segment data
    uint32_t data_cap;
    uint32_t data_offset;
};

struct GlobalSymbol {
    char name[16];
    uint32_t absolute_address;
    uint8_t defined;        // 1 if defined, 0 if external
};

struct SegmentMapEntry {
    int global_index;
    uint32_t offset_adjust;
};

void linker(const struct ObjectFile *objs, size_t num_files, char *out, struct MemoryRegion *mem, struct SegmentRule *rules);

#endif //LINKER_H
