#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/instructionDef.h"
#include "../include/symbolTable.h"
 #include "../include/tokeniser.h"

#define SYMBOL_TABLE_BASE_SIZE 16
#define MAX_LINE_LENGTH 1024
#define LINE_NUM_BASE_SIZE 16

// enum instruction {
// 	MOV_REG_REG = 0x00,
// 	MOV_REG_IMM = 0x04,
// 	MOV_REG_ABS = 0x08,
// 	MOV_REG_IND = 0x0C,
// 	MOV_REG_IDX = 0x10,
// 	MOV_ABS_REG = 0x14,
// 	MOV_IND_REG = 0x18,
// 	MOV_IDX_REG = 0x1C,
// 	ADD_REG_REG = 0x01,
// 	ADD_REG_IMM = 0x05,
// 	HLT = 0x03,
// 	NOP = 0x13,
// };
//
// enum registers {
// 	R0 = 0x0,
// 	R1 = 0x1,
// 	R2 = 0x2,
// 	R3 = 0x3,
// 	PC = 0xB,
// 	SP = 0xC,
// 	BP = 0xD,
// 	H = 0xE,
// 	L = 0xF
// };

InstructionDef instruction_table[] = {
	{ "hlt", 0x03, 0, encode_hlt, get_size_hlt },
	{ "nop", 0x13, 0, encode_nop, get_size_nop }
};
int table_size = sizeof(instruction_table) / sizeof(InstructionDef);

void consume_token(int *tok_idx, Token *t, const TokenList *tok_list) {
	if (*tok_idx >= tok_list->count) {
		printf("Out of tokens\n");
		exit(1);
	}
	*t = tok_list->data[*tok_idx];
	(*tok_idx)++;
}

void first_pass(const TokenList *tokens, SymbolTable *symbol_table, uint16_t *num_bytes) {
	uint16_t current_address = 0;
	int tok_idx = 0;
	Token current_token;
	consume_token(&tok_idx, &current_token, tokens);

	while (current_token.type != TOKEN_EOF) {
		if (current_token.type == TOKEN_LABEL) {
			// locally store string and consume token
			char label[strlen(current_token.str_val) + 1];
			strcpy(label, current_token.str_val);
			consume_token(&tok_idx, &current_token, tokens);
			if (current_token.type == TOKEN_SYMBOL && current_token.str_val[0] == ':') {
				add_symbol(symbol_table, label, current_address);
				consume_token(&tok_idx, &current_token, tokens);
				continue;
			}
			printf("bad child");
			exit(1);
		}

		if (current_token.type == TOKEN_MNEMONIC) {
			const InstructionDef *inst = find_instruction(instruction_table, table_size, current_token.str_val);
			consume_token(&tok_idx, &current_token, tokens);

			Token operands[32];
			int operand_count = 0;

			while (current_token.type != TOKEN_MNEMONIC && current_token.type != TOKEN_EOF) {
				operands[operand_count++] = current_token;
				consume_token(&tok_idx, &current_token, tokens);
				if (current_token.type == TOKEN_EOF) break;
				if (operand_count >= 32) {
					fprintf(stderr, "Too many operands\n");
					exit(1);
				}
			}

			current_address += inst->get_size(operands, operand_count, inst->operand_count);

			continue;
		}

		// fallback - consume token
		consume_token(&tok_idx, &current_token, tokens);
	}

	*num_bytes = current_address;
}

uint8_t *second_pass(char **lines, SymbolTable *table) {

uint8_t *second_pass(const TokenList *tokens, SymbolTable *table, const uint16_t *num_bytes) {
	uint16_t current_address = 0;
	int current_line = 0;
	int buff_index = 0;
	char buff[MAX_LINE_LENGTH] = {0};
	size_t binary_index = 0;
	uint8_t *binary = malloc(1024 * sizeof(uint8_t));

	while (lines[current_line] != NULL) {
		buff_index = 0;
		memset(buff, 0, MAX_LINE_LENGTH);
		for (int i = 0; i < strlen(lines[current_line]); i++) {
			if (isspace(lines[current_line][i])) {
				while (isspace(lines[current_line][i])) {
					i++;
				}
			}
			buff[buff_index] = lines[current_line][i];

			if (strcmp(buff, "hlt") == 0) {
				binary[binary_index++] = HLT;
				current_address += 1;
				memset(buff, 0, buff_index + 1);
				buff_index = -1;
			} else if (strcmp(buff, "nop") == 0) {
				binary[binary_index++] = NOP;
				current_address += 1;
				memset(buff, 0, buff_index + 1);
				buff_index = -1;
			}

			if (buff[buff_index] == ':') { // if is symbol skip
				memset(buff, 0, buff_index + 1);
				buff_index = -1;
			}

			buff_index++;
		}
		current_line++;
	}

	printf("Assembled Binary:\n");
	for (int i = 0; i < binary_index; i++) {
		printf("%02x ", binary[i]);
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

	InstructionDef instruction_table[] = {
		{ "hlt", 0x03, 0, encode_hlt },
		{ "nop", 0x13, 0, encode_nop }
	};

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
	uint8_t *binary = second_pass(lines, &symbol_table);

	// free everything
	free(binary);
	free_lines(lines, num_lines);
	free_table(&symbol_table);

	return 0;
}
