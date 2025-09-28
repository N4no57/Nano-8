//
// Created by brego on 28/09/2025.
//

#ifndef SEGMENTS_H
#define SEGMENTS_H

#include <stdint.h>

typedef struct {
    char name[16];
    uint8_t *data;
    size_t size;
    size_t capacity;
} AssemblingSegment;

typedef struct {
    AssemblingSegment *segments;
    size_t count;
    size_t capacity;
} AssemblingSegmentTable;

void initSegmentTable(AssemblingSegmentTable *table);
void appendSegment(AssemblingSegmentTable *table, AssemblingSegment segment);
int find_segment(const AssemblingSegmentTable *table, AssemblingSegment **seg, const char *name);
void freeSegmentTable(const AssemblingSegmentTable *table);

#endif //SEGMENTS_H
