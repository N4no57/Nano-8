//
// Created by brego on 28/09/2025.
//

#include "../include/objectFileWriter.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define VERSION "1.0.0"
#define DEFINED_TRUE 1
#define DEFINED_FALSE 0
#define RELOC_ABSOLUTE 0
#define RELOC_RELATIVE 1

#define SEGMENT_SIZE 22
#define HEADER_SIZE 12 // + (SEGMENT_SIZE * numSegments)

struct ObjectFile generateFileStruct(SymbolTable *sTable, AssemblingSegmentTable *segTable) {
    struct ObjectFile objectFile;
    objectFile.header.magic = 0x4E384F46;
    strcpy(objectFile.header.version, VERSION);
    objectFile.header.segmentTable.numSegments = segTable->count;
    objectFile.header.segmentTable.entries = malloc(sizeof(struct Segment) * segTable->count);

    uint16_t dataSize = 0;

    for (int i = 0; i < segTable->count; i++) {
        strcpy(objectFile.header.segmentTable.entries[i].name, segTable->segments[i].name);
        objectFile.header.segmentTable.entries[i].size = segTable->segments[i].size;
        objectFile.header.segmentTable.entries[i].file_offset = HEADER_SIZE + dataSize + (SEGMENT_SIZE * objectFile.header.segmentTable.numSegments); // TODO placeholder
        dataSize += segTable->segments[i].size;
    }

    objectFile.Data = malloc(dataSize);
    uint16_t dataIndex = 0;
    for (int i = 0; i < segTable->count; i++) {
        for (int j = 0; j < segTable->segments[i].size; j++) {
            objectFile.Data[dataIndex++] = segTable->segments[i].data[j];
        }
    }

    objectFile.symbolTable.numSymbols = sTable->count;
    objectFile.symbolTable.symbols = malloc(sizeof(struct ObjSymbol) * sTable->count);
    for (int i = 0; i < sTable->count; i++) {
        strcpy(objectFile.symbolTable.symbols[i].name, sTable->data[i].label);
        objectFile.symbolTable.symbols[i].segment_index = 0; // TODO placeholder
        objectFile.symbolTable.symbols[i].segment_offset = sTable->data[i].offset;
        objectFile.symbolTable.symbols[i].defined = sTable->data[i].defined;
    }

    objectFile.relocationTable.numRelocations = 1; // TODO placeholder
    objectFile.relocationTable.relocations = malloc(sizeof(struct RelocationTable) * 1);
    objectFile.relocationTable.relocations[0].segment_offset = 0;
    strcpy(objectFile.relocationTable.relocations[0].name, "placeholder");
    objectFile.relocationTable.relocations[0].type = RELOC_ABSOLUTE; // TODO end - placeholder

    return objectFile;
}
