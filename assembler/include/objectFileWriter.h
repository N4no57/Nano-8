//
// Created by brego on 28/09/2025.
//

#ifndef OBJECT_FILE_H
#define OBJECT_FILE_H

#include <stdint.h>
#include "utils.h"

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

struct ObjSymbol {
    char name[16];
    uint16_t segment_index;
    uint16_t segment_offset; // offset = abs_addrs - segment_start
    uint8_t defined;
};

struct ObjSymbolTable {
    uint16_t numSymbols;
    struct ObjSymbol *symbols;
};

struct RelocationEntry {
    uint16_t segment_offset;
    char name[16];
    uint8_t type;
};

struct RelocationTable {
    uint16_t numRelocations;
    struct RelocationEntry *relocations;
};

struct ObjectFile {
    struct Header header;
    uint8_t *Data; // size = sum of size of every segment
    struct ObjSymbolTable symbolTable;
    struct RelocationTable relocationTable;
};

struct ObjectFile generateFileStruct(SymbolTable *sTable, AssemblingSegmentTable *segTable);
void dumpObjectFile(const struct ObjectFile *obj);

#endif //OBJECT_FILE_H
