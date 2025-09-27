//
// Created by Bernardo on 27/09/2025.
//

#include <stdint.h>

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

typedef struct {
    char *label;
    uint16_t address;
} Symbol;

typedef struct {
    Symbol *data;
    size_t count;
    size_t capacity;
} SymbolTable;

void init_table(SymbolTable *table);
void add_symbol(SymbolTable *table, const char *label, uint16_t address);
int find_symbol(const SymbolTable *table, const char *label, uint16_t *out_addr);
void free_table(const SymbolTable *table);

#endif //SYMBOLTABLE_H
