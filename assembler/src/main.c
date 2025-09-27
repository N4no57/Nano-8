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

enum registers {
	R0 = 0x0,
	R1 = 0x1,
	R2 = 0x2,
	R3 = 0x3,
	PC = 0xB,
	SP = 0xC,
	BP = 0xD,
	H = 0xE,
	L = 0xF
};

typedef struct {
	enum instruction instruction;
	int operand_count;
	int instruction_size;
} InstructionDef;

void first_pass(char **lines, SymbolTable *symbol_table) {
	uint16_t current_address = 0;
	int current_line = 0;
	int buff_index = 0;
	char buff[MAX_LINE_LENGTH] = {0};

	while (lines[current_line] != NULL) {
		for (int i = 0; i < strlen(lines[current_line]); i++) {
			if (lines[current_line][i] == ' ') i++;
			buff[buff_index] = lines[current_line][i];

			if (strcmp(buff, "hlt") == 0) {
				memset(buff, 0, MAX_LINE_LENGTH);
				buff_index = -1;
				current_address += 1;
			} else if (strcmp(buff, "nop") == 0) {
				memset(buff, 0, MAX_LINE_LENGTH);
				buff_index = -1;
				current_address += 1;
			}

			if (buff[buff_index] == ':') {
				buff[buff_index] = '\0';
				add_symbol(symbol_table, buff, current_address);
				memset(buff, 0, MAX_LINE_LENGTH);
				buff_index = -1;
			}

			buff_index++;
		}
		current_line++;
	}
}

uint8_t *second_pass(char **lines, SymbolTable *table) {
	uint16_t current_address = 0;
	int current_line = 0;
	int buff_index = 0;
	char buff[MAX_LINE_LENGTH] = {0};
	size_t binary_index = 0;
	uint8_t *binary = malloc(1024 * sizeof(uint8_t));

	while (lines[current_line] != NULL) {
		for (int i = 0; i < strlen(lines[current_line]); i++) {
			if (lines[current_line][i] == ' ') i++;
			buff[buff_index] = lines[current_line][i];

			if (strcmp(buff, "hlt") == 0) {
				binary[binary_index++] = HLT;
				current_address += 1;
			}

			buff_index++;
		}
		current_line++;
	}

	return binary;
}

void free_lines(char **lines, const int num_lines) {
	for (int i = 0; i < num_lines; i++) {
		free(lines[i]);
	}
	free(lines);
}

int main() {
	SymbolTable symbol_table;
	init_table(&symbol_table);

	int line_capacity = LINE_NUM_BASE_SIZE;
    char **lines = malloc(line_capacity * sizeof(char *));
	if (!lines) {
		printf("Memory allocation error\n");
		free_table(&symbol_table);
		return 1;
	}

	FILE *input = fopen("test.asm", "r");
	if (!input) {
		free_table(&symbol_table);
		free_lines(lines, 0);
		printf("Error opening input file\n");
		return 1;
	}

	int num_lines = 0;
	char buffer[MAX_LINE_LENGTH];
	while (fgets(buffer, line_capacity, input)) {
		if (num_lines >= line_capacity - 1) {
			char **tmp = realloc(lines, line_capacity * 2 * sizeof(char *));
			if (!tmp) {
				printf("Memory allocation error\n");
				free_lines(lines, num_lines);
				free_table(&symbol_table);
				return 1;
			}
			lines = tmp;
			line_capacity *= 2;
		}
		lines[num_lines++] = strdup(buffer);
	}
	lines[num_lines] = NULL;

	fclose(input);

	first_pass(lines, &symbol_table);

	// free everything
	free_lines(lines, num_lines);
	free_table(&symbol_table);

	return 0;
}
