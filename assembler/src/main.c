#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/symbolTable.h"

#define SYMBOL_TABLE_BASE_SIZE 16
#define MAX_LINE_LENGTH 1024
#define LINE_NUM_BASE_SIZE 16

enum instruction {
	MOV_REG_REG = 0x00,
	MOV_REG_IMM = 0x04,
	MOV_REG_ABS = 0x08,
	MOV_REG_IND = 0x0C,
	MOV_REG_IDX = 0x10,
	MOV_ABS_REG = 0x14,
	MOV_IND_REG = 0x18,
	MOV_IDX_REG = 0x1C,
	ADD_REG_REG = 0x01,
	ADD_REG_IMM = 0x05,
	HLT = 0x03,
	NOP = 0x13,
};

typedef struct {
	enum instruction instruction;
	int operand_count;
	int instruction_size;
} InstructionDef;

char *get_line(FILE *in) {
	char *line = malloc(MAX_LINE_LENGTH);
	if (line == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	fgets(line, MAX_LINE_LENGTH, in);
	return line;
}

void first_pass(char **lines, SymbolTable *symbol_table) {
	// Assumes that there is at least 1 line in the file
	uint16_t current_address = 0;
	int current_line = 0;

	do {
		for (int i = 0; i < strlen(lines[current_line]); i++) {
			if (lines[current_line][i] == ':') {
				Symbol *symbol = malloc(sizeof(Symbol));
				strcpy_s(symbol->label, i+1, lines[current_line]);
				symbol->address = current_address;
				free(symbol);
			}
		}
	} while (lines[current_line++] != NULL);
}

void second_pass(char **lines, Symbol *symbols, FILE *out);

int main() {
	SymbolTable symbol_table;

	int line_capacity = LINE_NUM_BASE_SIZE;
    char **lines = malloc(line_capacity * sizeof(char *));
	if (lines == NULL) {
		printf("Memory allocation error\n");
		return 1;
	}

	FILE *input = fopen("test.asm", "r");
	if (input == NULL) {
		printf("Error opening input file\n");
		return 1;
	}

	int line_num = 0;
	while (!feof(input)) {
		if (line_num >= line_capacity - 1) {
			lines = realloc(lines, (line_capacity*=2) * sizeof(char *));
			if (lines == NULL) {
				printf("Memory allocation error\n");
				return 1;
			}
		}
		lines[line_num++] = get_line(input);
	}
	lines[line_num] = NULL;

	fclose(input);

	first_pass(lines, &symbol_table);

	// free everything
	for (int i = 0; i < line_num; i++) {
		free(lines[i]);
	}
	free(lines);

	return 0;
}
