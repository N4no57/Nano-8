//
// Created by brego on 29/09/2025.
//

#ifndef OBJ_H
#define OBJ_H

#include <stdint.h>

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
    uint32_t magic; // magic number should be 0x4E384F46
    char version[6];
    struct SegmentTable segmentTable;
};

struct Symbol {
    char name[16];
    uint16_t segment_index;
    uint16_t segment_offset; // offset = abs_addrs - segment_start
    uint8_t defined;
};

struct SymbolTable {
    uint16_t numSymbols;
    struct Symbol *symbols;
};

struct RelocationEntry {
    char name[16];
    uint16_t segment_index;
    uint16_t segment_offset;
    uint8_t type;
};

struct RelocationTable {
    uint16_t numRelocations;
    struct RelocationEntry *relocations;
    uint16_t capacity; // value used only for generating the relocation table
};

struct ObjectFile {
    struct Header header;
    uint8_t *Data; // size = sum of size of every segment
    struct SymbolTable symbolTable;
    struct RelocationTable relocationTable;
};

struct ObjectFile readObjectFile(const char *filename);

#endif //OBJ_H
