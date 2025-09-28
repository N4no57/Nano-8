//
// Created by brego on 28/09/2025.
//

#include "../include/segments.h"
#include <stdlib.h>
#include <string.h>

void initSegmentTable(AssemblingSegmentTable *table) {
    table->capacity = 8;
    table->count = 0;
    table->segments = malloc(table->capacity * sizeof(AssemblingSegment));
    if (!table->segments) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
}

void appendSegment(AssemblingSegmentTable *table, const AssemblingSegment segment) {
    if (table->count >= table->capacity) {
        table->capacity *= 2;
        AssemblingSegment *tmp = realloc(table->segments, table->capacity * sizeof(AssemblingSegment));
        if (tmp == NULL) {
            perror("realloc");
            freeSegmentTable(table);
            exit(EXIT_FAILURE);
        }
        table->segments = tmp;
    }

    table->segments[table->count] = segment;
    table->count++;
}

int find_segment(const AssemblingSegmentTable *table, AssemblingSegment **seg, const char *name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->segments[i].name, name) == 0) {
            *seg = &table->segments[i];
            return 1;
        }
    }
    return 0;
}

int get_segment_index(const AssemblingSegmentTable *table, AssemblingSegment *segment) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->segments[i].name, segment->name) == 0) {
            return i;
        }
    }
    return -1;
}

void freeSegmentTable(const AssemblingSegmentTable *table) {
    for (int i = 0; i < table->count; i++) {
        free(table->segments[i].data);
    }
    free(table->segments);
}
