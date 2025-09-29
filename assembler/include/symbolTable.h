//
// Created by Bernardo on 27/09/2025.
//

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdint.h>
#include "segments.h"

typedef struct {
    char *label;
    AssemblingSegment *segment;
    uint16_t offset;
    uint8_t defined;
} Symbol;

typedef struct {
    Symbol *data;
    size_t count;
    size_t capacity;
} SymbolTable;

void init_table(SymbolTable *table);
void add_symbol(SymbolTable *table, AssemblingSegment *seg, const char *label, uint16_t offset);
int find_symbol(const SymbolTable *table, const char *label, Symbol *result);
void free_table(const SymbolTable *table);

#endif //SYMBOLTABLE_H
