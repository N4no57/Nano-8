//
// Created by Bernardo on 27/09/2025.
//

#include "../include/symbolTable.h"
#include <stdlib.h>
#include <string.h>

void init_table(SymbolTable *table) {
    table->count = 0;
    table->capacity = 8;
    table->data = malloc(table->capacity * sizeof(Symbol));
}

void add_symbol(SymbolTable *table, const char *label, uint16_t address) {
    if (table->count >= table->capacity) {
      table->capacity *= 2;
      table->data = realloc(table->data, table->capacity * sizeof(Symbol));
    }

    table->data[table->count].label = strdup(label);
    table->data[table->count].address = address;

    table->count++;
}

int find_symbol(SymbolTable *table, const char *label, uint16_t *out_addr) {
    for (size_t i = 0; i < table->count; i++) {
        if (strcmp(table->data[i].label, label) == 0) {
            *out_addr = table->data[i].address;
            return 1; // found
        }
    }
    return 0; // not found
}

void free_table(SymbolTable *table) {
    for (size_t i = 0; i < table->count; i++) {
      free(table->data[i].label);
    }
    free(table->data);
}
