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

    int data_size = 0;

    obj.header.segmentTable.entries = malloc(obj.header.segmentTable.numSegments * sizeof(struct Segment));
    for (int i = 0; i < obj.header.segmentTable.numSegments; i++) {
        fread(obj.header.segmentTable.entries[i].name, sizeof(obj.header.segmentTable.entries[i].name), 1, f);
        fread(&obj.header.segmentTable.entries[i].size, sizeof(obj.header.segmentTable.entries[i].size), 1, f);
        fread(&obj.header.segmentTable.entries[i].file_offset, sizeof(obj.header.segmentTable.entries[i].file_offset), 1, f);
        data_size += obj.header.segmentTable.entries[i].size;
        if (HAVE_PADDING) fseek(f, 10, SEEK_CUR);
    }

    obj.Data = malloc(data_size * (sizeof(obj.Data)));
    fread(obj.Data, sizeof(obj.Data), data_size, f);

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
        fread(&obj.relocationTable.relocations[i].type, sizeof(obj.relocationTable.relocations[i].type), 1, f);
        if (HAVE_PADDING) fseek(f, 11, SEEK_CUR);
    }

    fclose(f);

    return obj;
}
