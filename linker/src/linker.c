//
// Created by brego on 29/09/2025.
//

#include "../include/linker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t segment_table_capacity = 8;
uint32_t segment_table_count = 0;
struct LinkedSegment *linkedSegments = NULL;

void append_segment(struct LinkedSegment seg) {
    if (segment_table_count >= segment_table_capacity) {
        segment_table_capacity *= 2;
        struct LinkedSegment *tmp = realloc(linkedSegments, sizeof(struct LinkedSegment) * segment_table_capacity);
        if (!tmp) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        linkedSegments = tmp;
    }

    for (int i = 0; i < segment_table_count; i++) {
        seg.base_address += linkedSegments[i].size;
    }

    linkedSegments[segment_table_count] = seg;
    segment_table_count++;
}

void append_segment_data(const uint8_t *data, const uint16_t size, const int segment_idx) {
    if (linkedSegments[segment_idx].size + size >= linkedSegments[segment_idx].data_cap) {
        linkedSegments[segment_idx].data_cap += size;
        uint8_t *tmp = realloc(linkedSegments[segment_idx].data, linkedSegments[segment_idx].data_cap);
        if (!tmp) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        linkedSegments[segment_idx].data = tmp;
    }

    memcpy(linkedSegments[segment_idx].data + linkedSegments[segment_idx].size, data, size);
    linkedSegments[segment_idx].size += size;
}

int find_segment(char name[16]) {
    for (int i = 0; i < segment_table_count; i++) {
        if (strcmp(name, linkedSegments[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

void update_segment_base_addresses() { // if any overlapping addresses just shift it along... very simple, this will be easy
    if (segment_table_count == 1) { // never called without at least 1 in the table
        return;
    }

    for (int i = 0; i < segment_table_count-1; i++) {
        uint32_t upper_address = linkedSegments[i].base_address + linkedSegments[i].size;
        if (upper_address >= linkedSegments[i+1].base_address) {
            linkedSegments[i+1].base_address = upper_address;
        }
    }
}

uint32_t symbol_table_capacity = 8;
uint32_t symbol_table_count = 0;
struct GlobalSymbol *symbolTable = NULL;

void append_symbol(struct GlobalSymbol sym) {
    if (symbol_table_count >= symbol_table_capacity) {
        symbol_table_capacity *= 2;
        struct GlobalSymbol *tmp = realloc(symbolTable, sizeof(struct GlobalSymbol) * symbol_table_capacity);
        if (!tmp) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        symbolTable = tmp;
    }

    symbolTable[symbol_table_count] = sym;
    symbol_table_count++;
}

int find_symbol(char name[16]) {
    for (int i = 0; i < symbol_table_count; i++) {
        if (strcmp(name, symbolTable[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

struct SegmentMapEntry **segmentMap = NULL;

void append_segmentMapEntry(const struct SegmentMapEntry entry, const uint32_t obj_idx, const int local_segment_idx) {
    segmentMap[obj_idx][local_segment_idx] = entry;
}

void linker(const struct ObjectFile *objs, const size_t num_files, char *out) {
    linkedSegments = malloc(sizeof(struct LinkedSegment) * segment_table_capacity);
    if (!linkedSegments) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    symbolTable = malloc(sizeof(struct GlobalSymbol) * symbol_table_capacity);
    if (!symbolTable) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    segmentMap = malloc(sizeof(struct SegmentMapEntry *) * num_files);
    if (!segmentMap) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_files; i++) {
        segmentMap[i] = malloc(sizeof(struct SegmentMapEntry) * objs[i].header.segmentTable.numSegments);
        if (!segmentMap[i]) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
    }

    for (size_t i = 0; i < num_files; i++) {
        uint32_t offset_adjust[objs[i].header.segmentTable.numSegments];
        uint32_t data_offset = 0;
        for (int j = 0; j < objs[i].header.segmentTable.numSegments; j++) {
            char *segName = objs[i].header.segmentTable.entries[j].name;
            int seg_idx = find_segment(segName);
            if (seg_idx != -1) {
                uint8_t *data = objs[i].Data + data_offset;
                offset_adjust[j] = linkedSegments[seg_idx].size;
                append_segment_data(data, objs[i].header.segmentTable.entries[j].size, seg_idx);
                update_segment_base_addresses();
            } else {
                struct LinkedSegment seg;
                seg.size = objs[i].header.segmentTable.entries[j].size;
                seg.data_cap = seg.size;
                seg.data = malloc(seg.data_cap);
                if (!seg.data) {
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                memcpy(seg.data, objs[i].Data + data_offset, seg.size);
                strcpy(seg.name, segName);
                seg.base_address = 0x0000;
                append_segment(seg);
                offset_adjust[j] = 0;
            }
            data_offset += objs[i].header.segmentTable.entries[j].size;
        }

        for (int j = 0; j < objs[i].symbolTable.numSymbols; j++) {
            const int local_segment_idx = objs[i].symbolTable.symbols[j].segment_index;
            const int global_segment_idx = find_segment(objs[i].header.segmentTable.entries[local_segment_idx].name);
            segmentMap[i][local_segment_idx].global_index = global_segment_idx;
            segmentMap[i][local_segment_idx].offset_adjust = offset_adjust[j];
        }
    }

    // combine into global symbol table
    for (size_t i = 0; i < num_files; i++) {
        for (int j = 0; j < objs[i].symbolTable.numSymbols; j++) {
            if (find_symbol(objs[i].symbolTable.symbols[j].name) != -1) {
                printf("nano8-ld: fatal error: Found duplicate symbol %s\n", objs[i].symbolTable.symbols[j].name);
                exit(EXIT_FAILURE);
            }
            struct GlobalSymbol sym;
            strcpy(sym.name, objs[i].symbolTable.symbols[j].name);

            uint32_t local_segment = objs[i].symbolTable.symbols[j].segment_index;

            if (objs[i].symbolTable.symbols[j].defined) {
                sym.defined = 1;
                sym.absolute_address = linkedSegments[segmentMap[i][local_segment].global_index].base_address
                                     + segmentMap[i][local_segment].offset_adjust + objs[i].symbolTable.symbols[j].segment_offset;
            } else {
                sym.absolute_address = 0xFFFFFFFF;
                sym.defined = 0;
            }
            append_symbol(sym);
        }
    }

    for (size_t i = 0; i < num_files; i++) {
        for (int j = 0; j < objs[i].relocationTable.numRelocations; j++) {
            uint32_t target_seg = segmentMap[i][objs[i].relocationTable.relocations[j].segment_index].global_index;
            uint32_t offset_adjust = segmentMap[i][objs[i].relocationTable.relocations[j].segment_index].offset_adjust;
            uint32_t offset = objs[i].relocationTable.relocations[j].segment_offset;
            uint32_t target_offset = offset_adjust + offset;
            int symbol_idx = find_symbol(objs[i].relocationTable.relocations[j].name);
            if (symbol_idx == -1) {
                printf("nano8-ld: fatal-error: symbol not found %s\n", objs[i].relocationTable.relocations[j].name);
                exit(EXIT_FAILURE);
            }
            linkedSegments[target_seg].data[target_offset] = symbolTable[symbol_idx].absolute_address & 0xFF;
            linkedSegments[target_seg].data[target_offset+1] = symbolTable[symbol_idx].absolute_address >> 8 & 0xFF;
        }
    }

    FILE *f = fopen(out, "wb");
    for (size_t i = 0; i < segment_table_count; i++) {
        fwrite(linkedSegments[i].data, linkedSegments[i].size, 1, f);
    }

    for (size_t i = 0; i < segment_table_count; i++) {
        free(linkedSegments[i].data);
    }
    free(linkedSegments);
}