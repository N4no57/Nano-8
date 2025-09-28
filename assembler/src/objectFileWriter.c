//
// Created by brego on 28/09/2025.
//

#include "../include/objectFileWriter.h"

struct Segment {
    char name[16];
    uint16_t size;
    uint32_t file_offset;
};

struct SegmentTable {
    uint16_t numSegments;
    struct Segment *entries;
};

struct Header {
    uint32_t magic;
    char version[5];
    struct SegmentTable segments;
};