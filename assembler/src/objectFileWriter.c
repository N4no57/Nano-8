//
// Created by brego on 28/09/2025.
//

#include "../include/objectFileWriter.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define VERSION "1.0.0"

#define SEGMENT_SIZE 32
#define HEADER_SIZE 16 // + (SEGMENT_SIZE * numSegments)

struct ObjectFile generateFileStruct(SymbolTable *sTable, AssemblingSegmentTable *segTable, struct RelocationTable *relocTable) {
    struct ObjectFile objectFile;
    objectFile.header.magic = 0x4E384F46;
    strcpy(objectFile.header.version, VERSION);
    objectFile.header.segmentTable.numSegments = segTable->count;
    objectFile.header.segmentTable.entries = malloc(sizeof(struct Segment) * segTable->count);

    uint16_t dataSize = 0;

    for (int i = 0; i < segTable->count; i++) {
        memset(objectFile.header.segmentTable.entries[i].name, 0, 16);
        strcpy(objectFile.header.segmentTable.entries[i].name, segTable->segments[i].name);
        objectFile.header.segmentTable.entries[i].size = segTable->segments[i].size;
        objectFile.header.segmentTable.entries[i].file_offset = HEADER_SIZE + dataSize + (SEGMENT_SIZE * objectFile.header.segmentTable.numSegments);
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
        memset(objectFile.symbolTable.symbols[i].name, 0, 16);
        strcpy(objectFile.symbolTable.symbols[i].name, sTable->data[i].label);
        objectFile.symbolTable.symbols[i].segment_index = get_segment_index(segTable, sTable->data[i].segment);
        objectFile.symbolTable.symbols[i].segment_offset = sTable->data[i].offset;
        objectFile.symbolTable.symbols[i].defined = sTable->data[i].defined;
    }

    objectFile.relocationTable.numRelocations = relocTable->numRelocations;
    objectFile.relocationTable.relocations = malloc(sizeof(struct RelocationEntry) * relocTable->numRelocations);
    for (int i = 0; i < relocTable->numRelocations; i++) {
        memset(objectFile.relocationTable.relocations[i].name, 0, 16);
        strcpy(objectFile.relocationTable.relocations[i].name, relocTable->relocations[i].name);
        objectFile.relocationTable.relocations[i].segment_index = relocTable->relocations[i].segment_index;
        objectFile.relocationTable.relocations[i].segment_offset = relocTable->relocations[i].segment_offset;
        objectFile.relocationTable.relocations[i].type = relocTable->relocations[i].type;
    }

    return objectFile;
}

void freeObjectFile(const struct ObjectFile *obj) {
    free(obj->header.segmentTable.entries);
    free(obj->Data);
    free(obj->symbolTable.symbols);
    free(obj->relocationTable.relocations);
}

void writeObjectFile(const struct ObjectFile *obj, const char *fileName) {
    char padding[16] = {0};

    FILE *f = fopen(fileName, "wb");
    if (!f) {
        perror("Failed to open file");
        exit(1);
    }

    fwrite(&obj->header.magic, sizeof(obj->header.magic), 1, f);
    fwrite(&obj->header.version, sizeof(obj->header.version), 1, f);
    fwrite(&obj->header.segmentTable.numSegments, sizeof(obj->header.segmentTable.numSegments), 1, f);
    fwrite(padding, 4, 1, f); // padding

    int data_size = 0;

    for (int i = 0; i < obj->header.segmentTable.numSegments; i++) {
        data_size += obj->header.segmentTable.entries[i].size;
        fwrite(obj->header.segmentTable.entries[i].name, sizeof(obj->header.segmentTable.entries[i].name), 1, f);
        fwrite(&obj->header.segmentTable.entries[i].size, sizeof(obj->header.segmentTable.entries[i].size), 1, f);
        fwrite(&obj->header.segmentTable.entries[i].file_offset, sizeof(obj->header.segmentTable.entries[i].file_offset), 1, f);
        fwrite(padding, 10, 1, f); // padding
    }

    fwrite(obj->Data, data_size, 1, f);

    const int padding_required = data_size % 16;

    if (padding_required) {
        fwrite(padding, 16 - (padding_required + sizeof(obj->symbolTable.numSymbols)), 1, f);
    }

    fwrite(&obj->symbolTable.numSymbols, sizeof(obj->symbolTable.numSymbols), 1, f);
    for (int i = 0; i < obj->symbolTable.numSymbols; i++) {
        fwrite(obj->symbolTable.symbols[i].name, sizeof(obj->symbolTable.symbols[i].name), 1, f);
        fwrite(&obj->symbolTable.symbols[i].segment_index, sizeof(obj->symbolTable.symbols[i].segment_index), 1, f);
        fwrite(&obj->symbolTable.symbols[i].segment_offset, sizeof(obj->symbolTable.symbols[i].segment_offset), 1, f);
        fwrite(&obj->symbolTable.symbols[i].defined, sizeof(obj->symbolTable.symbols[i].defined), 1, f);
    }

    fwrite(&obj->relocationTable.numRelocations, sizeof(obj->relocationTable.numRelocations), 1, f);
    for (int i = 0; i < obj->relocationTable.numRelocations; i++) {
        fwrite(obj->relocationTable.relocations[i].name, sizeof(obj->relocationTable.relocations[i].name), 1, f);
        fwrite(&obj->relocationTable.relocations[i].segment_offset, sizeof(obj->relocationTable.relocations[i].segment_offset), 1, f);
        fwrite(&obj->relocationTable.relocations[i].type, sizeof(obj->relocationTable.relocations[i].type), 1, f);
    }

    fclose(f);
}

void dumpObjectFile(const struct ObjectFile *obj) {
    printf("=== Nano-8 Object File ===\n");
    printf("Magic: 0x%08X\n", obj->header.magic);
    printf("Version: %s\n", obj->header.version);
    printf("Segments (%u):\n", obj->header.segmentTable.numSegments);

    int seg_offset = 0;

    for (int i = 0; i < obj->header.segmentTable.numSegments; i++) {
        struct Segment *seg = &obj->header.segmentTable.entries[i];
        printf("  Segment %d: '%s', size=%u, file_offset=0x%04X\n",
               i, seg->name, seg->size, seg->file_offset);
        printf("    Data: ");
        for (int j = 0; j < seg->size; j++) {
            printf("%02X ", obj->Data[j + seg_offset]); // could adjust for segment offset
        }
        seg_offset += seg->size;
        printf("\n");
    }

    printf("Symbols (%u):\n", obj->symbolTable.numSymbols);
    for (int i = 0; i < obj->symbolTable.numSymbols; i++) {
        struct ObjSymbol *sym = &obj->symbolTable.symbols[i];
        printf("  Symbol %d: '%s', segment=%u, offset=0x%04X, defined=%u\n",
               i, sym->name, sym->segment_index, sym->segment_offset, sym->defined);
    }

    printf("Relocations (%u):\n", obj->relocationTable.numRelocations);
    for (int i = 0; i < obj->relocationTable.numRelocations; i++) {
        struct RelocationEntry *rel = &obj->relocationTable.relocations[i];
        printf("  Relocation %d: segment=%u offset=0x%04X, symbol='%s', type=%u\n",
               i, rel->segment_index, rel->segment_offset, rel->name, rel->type);
    }

    printf("==========================\n");
}