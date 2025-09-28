#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/instructionDef.h"
#include "../include/symbolTable.h"
#include "../include/tokeniser.h"
#include "../include/utils.h"

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
	{ "mov", 0x00, 2, encode_mov, get_size_mov },
	{ "hlt", 0x03, 0, encode_hlt, get_size_hlt },
	{ "nop", 0x13, 0, encode_nop, get_size_nop }
};
int table_size = sizeof(instruction_table) / sizeof(InstructionDef);

void first_pass(const TokenList *tokens, SymbolTable *symbol_table, AssemblingSegmentTable *segment_table, uint16_t *num_bytes) {
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

		if (current_token.type == TOKEN_DIRECTIVE) {
			if (strcmp(current_token.str_val, ".db") == 0) {
				consume_token(&tok_idx, &current_token, tokens);
				if (is_base_mod(current_token)) consume_token(&tok_idx, &current_token, tokens);
				consume_token(&tok_idx, &current_token, tokens);
				current_address++;
				continue;
			} else if (strcmp(current_token.str_val, ".segment") == 0) {
				consume_token(&tok_idx, &current_token, tokens);
				consume_token(&tok_idx, &current_token, tokens);
				continue;
			} else {
				fprintf(stderr, "Illegal directive\n---->%s", current_token.str_val);
				exit(1);
			}
		}

		if (current_token.type == TOKEN_MNEMONIC) {
			const InstructionDef *inst = find_instruction(instruction_table, table_size, current_token.str_val);
			consume_token(&tok_idx, &current_token, tokens);

			ParsedOperand operands[32];
			int operand_count = 0;

			while (operand_count < inst->operand_count) {
				operands[operand_count++] = operand_parser(tokens, symbol_table, &tok_idx, &current_token);
				if (current_token.type == TOKEN_SYMBOL && current_token.str_val[0] == ',') consume_token(&tok_idx, &current_token, tokens);
				if (current_token.type == TOKEN_EOF) break;
				if (operand_count >= 32) {
					fprintf(stderr, "Too many operands\n");
					exit(1);
				}
			}

			const uint8_t size = inst->get_size(operand_count, inst->operand_count, operands);
			current_address += size;

			continue;
		}

		// fallback - consume token
		consume_token(&tok_idx, &current_token, tokens);
	}

	*num_bytes = current_address;
}

void second_pass(const TokenList *tokens, SymbolTable *table, AssemblingSegmentTable *segment_table, const uint16_t *num_bytes) {
	int tok_idx = 0;

	AssemblingSegment *current_segment = 0;
	find_segment(segment_table, &current_segment, "code");

	Token current_token;
	consume_token(&tok_idx, &current_token, tokens);

	while (current_token.type != TOKEN_EOF) {
		if (current_token.type == TOKEN_LABEL) {
			consume_token(&tok_idx, &current_token, tokens);
			if (current_token.type == TOKEN_SYMBOL && current_token.str_val[0] == ':') {
				consume_token(&tok_idx, &current_token, tokens);
				continue;
			}
			printf("bad child");
			exit(1);
		}

		if (current_token.type == TOKEN_DIRECTIVE) {
			if (strcmp(current_token.str_val, ".db") == 0) {
				consume_token(&tok_idx, &current_token, tokens);
				if (is_base_mod(current_token)) consume_token(&tok_idx, &current_token, tokens);

				if (current_segment->size >= current_segment->capacity) {
					current_segment->capacity *= 2;
					uint8_t *temp = realloc(current_segment->data, current_segment->capacity);
					if (!temp) {
						fprintf(stderr, "Error: out of memory\n");
						exit(1);
					}
					current_segment->data = temp;
				}

				current_segment->data[current_segment->size++] = current_token.int_value;
				consume_token(&tok_idx, &current_token, tokens);
				continue;
			} else if (strcmp(current_token.str_val, ".segment") == 0) {
				consume_token(&tok_idx, &current_token, tokens);
				char segName[16];
				strcpy(segName, current_token.str_val);
				consume_token(&tok_idx, &current_token, tokens);
				AssemblingSegment *tmp = 0;
				if (find_segment(segment_table, &tmp, segName)) { // check if desired segment exists
					current_segment = tmp;
					continue;
				}
				// create new segment and continue
				AssemblingSegment s;
				s.capacity = 8;
				strcpy(s.name, segName);
				s.size = 0;
				s.data = malloc(s.capacity * sizeof(char *));
				if (!s.data) {
					perror("malloc");
					exit(1);
				}
				appendSegment(segment_table, s);
				current_segment = &segment_table->segments[segment_table->count-1];
				continue;
			} else {
				fprintf(stderr, "Illegal directive\n---->%s", current_token.str_val);
				exit(1);
			}
		}

		if (current_token.type == TOKEN_MNEMONIC) {
			const InstructionDef *inst = find_instruction(instruction_table, table_size, current_token.str_val);
			consume_token(&tok_idx, &current_token, tokens);

			ParsedOperand operands[32];
			int operand_count = 0;

			while (operand_count < inst->operand_count) {
				operands[operand_count++] = operand_parser(tokens, table, &tok_idx, &current_token);
				if (current_token.type == TOKEN_SYMBOL && current_token.str_val[0] == ',') consume_token(&tok_idx, &current_token, tokens);
				if (current_token.type == TOKEN_EOF) break;
				if (operand_count >= 32) {
					fprintf(stderr, "Too many operands\n");
					exit(1);
				}
			}

			inst->encode(inst->base_opcode, operand_count, current_segment, operands);

			continue;
		}

		// fallback - consume
		consume_token(&tok_idx, &current_token, tokens);
	}

	for (size_t i = 0; i < segment_table->count; i++) {
		printf("Segment %s hexdump:", segment_table->segments[i].name);
		for (size_t j = 0; j < segment_table->segments[i].size; j++) {
			printf(" %02x", segment_table->segments[i].data[j]);
		}
		printf("\n");
	}
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

	AssemblingSegmentTable segmentTable;
	initSegmentTable(&segmentTable);
	AssemblingSegment codeSegment;
	codeSegment.capacity = 8;
	strcpy(codeSegment.name, "code");
	codeSegment.size = 0;
	codeSegment.data = malloc(codeSegment.capacity * sizeof(uint8_t *));
	if (!codeSegment.data) {
		perror("malloc");
		free_lines(lines, 0);
		freeSegmentTable(&segmentTable);
		free_table(&symbol_table);
		return 1;
	}
	appendSegment(&segmentTable, codeSegment);

	FILE *input = fopen("test.asm", "r");
	if (!input) {
		free_table(&symbol_table);
		free_lines(lines, 0);
		printf("Error opening input file\n");
		return 1;
	}

	int num_lines = 0;
	char buffer[MAX_LINE_LENGTH];
	while (fgets(buffer, MAX_LINE_LENGTH, input)) {
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

	const TokenList tokens = tokenise(lines);

	free_lines(lines, num_lines);

	uint16_t binary_size = 0;

	first_pass(&tokens, &symbol_table, &segmentTable, &binary_size);

	if (binary_size == 0) {
		printf("Nothing to compile\n");
		freeTokenList(&tokens);
		free_table(&symbol_table);
		return 1;
	}

	second_pass(&tokens, &symbol_table, &segmentTable, &binary_size);

	// free everything
	freeTokenList(&tokens);
	freeSegmentTable(&segmentTable);
	free_table(&symbol_table);

	return 0;
}
