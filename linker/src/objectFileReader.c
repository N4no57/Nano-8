//
// Created by brego on 29/09/2025.
//

#include "../include/objectFileReader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HAVE_PADDING 1

#define MAGIC_NUM 0x4E384F46

struct ObjectFile readObjectFile(const char *filename) {
    struct ObjectFile obj;

    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        perror("Error opening file");
        exit(1);
    }

    fread(&obj.header.magic, sizeof(obj.header.magic), 1, f);

    if (obj.header.magic != MAGIC_NUM) {
        perror("object file is invalid");
        exit(1);
    }

    fread(obj.header.version, sizeof(obj.header.version), 1, f);
    fread(&obj.header.segmentTable.numSegments, sizeof(obj.header.segmentTable.numSegments), 1, f);
    if (HAVE_PADDING) fseek(f, 4, SEEK_CUR);

    int data_size = 0;

    obj.header.segmentTable.entries = malloc(obj.header.segmentTable.numSegments * sizeof(struct Segment));
    for (int i = 0; i < obj.header.segmentTable.numSegments; i++) {
        fread(obj.header.segmentTable.entries[i].name, sizeof(obj.header.segmentTable.entries[i].name), 1, f);
        fread(&obj.header.segmentTable.entries[i].size, sizeof(obj.header.segmentTable.entries[i].size), 1, f);
        fread(&obj.header.segmentTable.entries[i].file_offset, sizeof(obj.header.segmentTable.entries[i].file_offset), 1, f);
        data_size += obj.header.segmentTable.entries[i].size;
        if (HAVE_PADDING) fseek(f, 10, SEEK_CUR);
    }

    obj.Data = malloc(data_size * (sizeof(uint8_t)));
    fread(obj.Data, sizeof(uint8_t), data_size, f);

    int pad = (16 - (data_size % 16) - 2) % 16;
    if (pad < 1) pad += 16;
    if (HAVE_PADDING) fseek(f, pad, SEEK_CUR);

    fread(&obj.symbolTable.numSymbols, sizeof(obj.symbolTable.numSymbols), 1, f);
    obj.symbolTable.symbols = malloc(obj.symbolTable.numSymbols * sizeof(struct Symbol));
    for (int i = 0; i < obj.symbolTable.numSymbols; i++) {
        fread(obj.symbolTable.symbols[i].name, sizeof(obj.symbolTable.symbols[i].name), 1, f);
        fread(&obj.symbolTable.symbols[i].segment_index, sizeof(obj.symbolTable.symbols[i].segment_index), 1, f);
        fread(&obj.symbolTable.symbols[i].segment_offset, sizeof(obj.symbolTable.symbols[i].segment_offset), 1, f);
        fread(&obj.symbolTable.symbols[i].defined, sizeof(obj.symbolTable.symbols[i].defined), 1, f);
        if (HAVE_PADDING) fseek(f, 11, SEEK_CUR);
    }

    fread(&obj.relocationTable.numRelocations, sizeof(obj.relocationTable.numRelocations), 1, f);
    obj.relocationTable.relocations = malloc(obj.relocationTable.numRelocations * sizeof(struct RelocationEntry));
    obj.relocationTable.capacity = obj.relocationTable.numRelocations;
    if (HAVE_PADDING) fseek(f, 14, SEEK_CUR);
    for (int i = 0; i < obj.relocationTable.numRelocations; i++) {
        fread(obj.relocationTable.relocations[i].name, sizeof(obj.relocationTable.relocations[i].name), 1, f);
        fread(&obj.relocationTable.relocations[i].segment_index, sizeof(obj.relocationTable.relocations[i].segment_index), 1, f);
        fread(&obj.relocationTable.relocations[i].segment_offset, sizeof(obj.relocationTable.relocations[i].segment_offset), 1, f);
        fread(&obj.relocationTable.relocations[i].addend, sizeof(obj.relocationTable.relocations[i].addend), 1, f);
        fread(&obj.relocationTable.relocations[i].type, sizeof(obj.relocationTable.relocations[i].type), 1, f);
        if (HAVE_PADDING) fseek(f, 9, SEEK_CUR);
    }

    fclose(f);

    return obj;
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
        struct Symbol *sym = &obj->symbolTable.symbols[i];
        printf("  Symbol %d: '%s', segment=%u, offset=0x%04X, defined=%u\n",
               i, sym->name, sym->segment_index, sym->segment_offset, sym->defined);
    }

    printf("Relocations (%u):\n", obj->relocationTable.numRelocations);
    for (int i = 0; i < obj->relocationTable.numRelocations; i++) {
        struct RelocationEntry *rel = &obj->relocationTable.relocations[i];
        printf("  Relocation %d: segment=%u offset=0x%04X, addend=0x%04X symbol='%s', type=%u\n",
               i, rel->segment_index, rel->segment_offset, rel->addend, rel->name, rel->type);
    }

    printf("==========================\n");
}
