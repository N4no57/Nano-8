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

int find_MemoryRegion(const struct MemoryRegion *mem, const char *name) {
    for (int i = 0; i < memRegion_count; i++) {
        if (strcmp(name, mem[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

int find_SegmentRule(const struct SegmentRule *rules, const char *name) {
    for (int i = 0; i < segRule_count; i++) {
        if (strcmp(name, rules[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

void append_segment(struct LinkedSegment seg, struct SegmentRule *rules, struct MemoryRegion *mem) {
    if (segment_table_count >= segment_table_capacity) {
        segment_table_capacity *= 2;
        struct LinkedSegment *tmp = realloc(linkedSegments, sizeof(struct LinkedSegment) * segment_table_capacity);
        if (!tmp) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        linkedSegments = tmp;
    }

    if (!configFile) {
        for (int i = 0; i < segment_table_count; i++) {
            seg.base_address += linkedSegments[i].size;
        }
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

int find_localSegment(struct SegmentTable *segTable, char name[16]) {
    for (int i = 0; i < segTable->numSegments; i++) {
        if (strcmp(segTable->entries[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int correct_relax(const struct ObjectFile *objs, const size_t num_files, const uint32_t target_seg, const uint32_t target_offset) {
    for (int i = 0; i < num_files; i++) {
        for (int j = 0; j < objs[i].relocationTable.numRelocations; j++) {
            struct RelocationEntry *entry = &objs[i].relocationTable.relocations[j];
            if (segmentMap[i][entry->segment_index].global_index != target_seg) {
                continue;
            }

            if (entry->segment_offset > target_offset) {
                entry->segment_offset--;
            }
        }
    }

    for (int i = 0; i < symbol_table_count; i++) {
        if (symbolTable[i].absolute_address > linkedSegments[target_seg].base_address + target_offset) {
            symbolTable[i].absolute_address--;
        }
    }

    return 0;
}

void handle_duplicate_symbol(size_t symbol_idx, const struct ObjectFile *objs, size_t file_idx, size_t symbol_table_idx) {
    struct Symbol *sym = &objs[file_idx].symbolTable.symbols[symbol_table_idx];

    if (symbolTable[symbol_idx].defined == 0) {
        struct GlobalSymbol *global = &symbolTable[symbol_idx];
        global->defined = 1;

        uint32_t local_segment = sym->segment_index;

        global->absolute_address = linkedSegments[segmentMap[file_idx][local_segment].global_index].base_address
                             + segmentMap[file_idx][local_segment].offset_adjust + sym->segment_offset;
        return;
    }
    if (sym->defined == 0) {
        return;
    }

    printf("nano8-ld: fatal error: Found duplicate symbol %s\n", sym->name);
    exit(EXIT_FAILURE);
}

void linker(const struct ObjectFile *objs, const size_t num_files, char *out, struct MemoryRegion *mem, struct SegmentRule *rules) {
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
            struct MemoryRegion *region = NULL;
            if (configFile) {
                const int segRule_idx = find_SegmentRule(rules, segName);
                if (segRule_idx == -1) {
                    printf("Segment %s not specified on config file", segName);
                    exit(EXIT_FAILURE);
                }
                const int memRegion_idx = find_MemoryRegion(mem, rules[segRule_idx].load_to);
                region = &mem[memRegion_idx];
            }
            const int seg_idx = find_segment(segName);
            if (seg_idx != -1) {
                const uint8_t *data = objs[i].Data + data_offset;
                offset_adjust[j] = linkedSegments[seg_idx].size;
                append_segment_data(data, objs[i].header.segmentTable.entries[j].size, seg_idx);
                update_segment_base_addresses();
            } else {
                struct LinkedSegment seg;
                seg.size = objs[i].header.segmentTable.entries[j].size;
                seg.data_cap = configFile == 0 ? seg.size : region->size;
                seg.data = malloc(seg.data_cap);
                if (!seg.data) {
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                memcpy(seg.data, objs[i].Data + data_offset, seg.size);
                strcpy(seg.name, segName);
                seg.base_address = configFile == 0 ? 0 : region->start + region->current_offset; // placeholder, corrected in append_segment()
                append_segment(seg, rules, mem);
                if (configFile) region->current_offset += seg.size;
                offset_adjust[j] = 0;
            }
            data_offset += objs[i].header.segmentTable.entries[j].size;
        }

        struct SegmentTable local_table = objs[i].header.segmentTable;
        for (int j = 0; j < local_table.numSegments; j++) {
            char *segName = local_table.entries[j].name;
            const int local_segment_idx = find_localSegment(&local_table, segName);
            const int global_segment_idx = find_segment(segName);
            segmentMap[i][local_segment_idx].global_index = global_segment_idx;
            segmentMap[i][local_segment_idx].offset_adjust = offset_adjust[local_segment_idx];
        }
    }

    // combine into global symbol table
    for (size_t i = 0; i < num_files; i++) {
        for (size_t j = 0; j < objs[i].symbolTable.numSymbols; j++) {
            int symbol_idx = find_symbol(objs[i].symbolTable.symbols[j].name);
            if (symbol_idx != -1) {
                handle_duplicate_symbol(symbol_idx, objs, i, j);
                continue;
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
            const int16_t addend = objs[i].relocationTable.relocations[j].addend;
            const uint32_t target_seg = segmentMap[i][objs[i].relocationTable.relocations[j].segment_index].global_index;
            const uint32_t offset_adjust = segmentMap[i][objs[i].relocationTable.relocations[j].segment_index].offset_adjust;
            const uint32_t offset = objs[i].relocationTable.relocations[j].segment_offset;
            const uint32_t target_offset = offset_adjust + offset;
            const uint8_t type = objs[i].relocationTable.relocations[j].type;
            const int symbol_idx = find_symbol(objs[i].relocationTable.relocations[j].name);
            if (symbol_idx == -1) {
                printf("nano8-ld: fatal-error: symbol not found %s\n", objs[i].relocationTable.relocations[j].name);
                exit(EXIT_FAILURE);
            }
            const uint32_t address = symbolTable[symbol_idx].absolute_address + addend;
            if (type == 0) { // absolute
                linkedSegments[target_seg].data[target_offset] = address & 0xFF;
                linkedSegments[target_seg].data[target_offset+1] = address >> 8 & 0xFF;
            } else if (type == 1) { // relative
                // abs - current = rel
                int8_t relative_address = (int8_t)(address - (linkedSegments[target_seg].base_address + target_offset + 1));
                linkedSegments[target_seg].data[target_offset] = relative_address;
            } else if (type == 2) { // relax
                // the idea of relax is when the assembler doesn't know if it should do
                // relative of absolute relocation it asks the linker please do it for me

                // big thing is all relocations after than need to have their
                // addresses shifted back to match the new location

                int relative_address = (int)(address - (linkedSegments[target_seg].base_address + target_offset + 1));
                if (relative_address < -128 || relative_address > 127) { // has to be absolute, real simple
                    linkedSegments[target_seg].data[target_offset] = address & 0xFF;
                    linkedSegments[target_seg].data[target_offset+1] = address >> 8 & 0xFF;
                } else { // is relative
                    linkedSegments[target_seg].data[target_offset] = (int8_t)relative_address;
                    // shift all bytes back
                    memcpy(&linkedSegments[target_seg].data[target_offset+1],
                        &linkedSegments[target_seg].data[target_offset+2],
                        linkedSegments[target_seg].size-(target_offset+2));
                    // update all positional information after the instruction
                    linkedSegments[target_seg].size--;
                    correct_relax(objs, num_files, target_seg, offset);
                }
            }
        }
    }

    FILE *f = fopen(out, "wb");
    if (!f) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    if (configFile) {
        for (int r = 0; r < memRegion_count; r++) {
            struct MemoryRegion *region = &mem[r];

            uint32_t used_space = 0;
            for (int s = 0; s < segment_table_count; s++) {
                struct LinkedSegment *seg = &linkedSegments[s];
                const int segRule_idx = find_SegmentRule(rules, seg->name);
                if (strcmp(rules[segRule_idx].load_to, region->name) == 0) {
                    uint32_t end = seg->base_address - region->start + seg->size;
                    if (end > used_space) used_space = end;
                }
            }

            uint8_t buffer[region->size];
            memset(buffer, 0, region->size);

            for (int s = 0; s < segment_table_count; s++) {
                struct LinkedSegment *seg = &linkedSegments[s];
                const int segRule_idx = find_SegmentRule(rules, seg->name);
                if (strcmp(rules[segRule_idx].load_to, region->name) == 0) {
                    memcpy(buffer + (seg->base_address - region->start), seg->data, seg->size);
                }
            }

            fwrite(buffer, region->fill == 1 ? region->size : used_space, 1, f);
        }
    } else {
        for (size_t i = 0; i < segment_table_count; i++) {
            fwrite(linkedSegments[i].data, linkedSegments[i].size, 1, f);
        }
    }

    fclose(f);

    for (size_t i = 0; i < segment_table_count; i++) {
        free(linkedSegments[i].data);
    }
    free(linkedSegments);
}