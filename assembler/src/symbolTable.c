//
// Created by Bernardo on 27/09/2025.
//

#include "../include/symbolTable.h"
#include "../include/objectFileWriter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void init_table(SymbolTable *table) {
    table->count = 0;
    table->capacity = 8;
    table->data = malloc(table->capacity * sizeof(Symbol));
}

void add_symbol(SymbolTable *table, AssemblingSegment *seg, const char *label, const uint16_t offset) {
    if (table->count >= table->capacity) {
        table->capacity *= 2;
        Symbol *tmp = realloc(table->data, table->capacity * sizeof(Symbol));
        if (!tmp) {
            printf("Failed to allocate memory for symbol table\n");
            exit(1);
        }
        table->data = tmp;
    }

    table->data[table->count].label = strdup(label);
    table->data[table->count].offset = offset;
    table->data[table->count].segment = seg;
    table->data[table->count].defined = DEFINED_TRUE;

    table->count++;
}

int find_symbol(const SymbolTable *table, const char *label, Symbol *result) {
    for (size_t i = 0; i < table->count; i++) {
        if (strcmp(table->data[i].label, label) == 0) {
            *result = table->data[i];
            return 1; // found
        }
    }
    return 0; // not found
}

void free_table(const SymbolTable *table) {
    for (size_t i = 0; i < table->count; i++) {
      free(table->data[i].label);
    }
    free(table->data);
}
