//
// Created by Bernardo on 29/09/2025.
//

#include "../include/assembler.h"
#include "../include/instructionDef.h"
#include "../include/symbolTable.h"
#include "../include/tokeniser.h"
#include "../include/utils.h"
#include "../include/objectFileWriter.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYMBOL_TABLE_BASE_SIZE 16
#define MAX_LINE_LENGTH 1024
#define LINE_NUM_BASE_SIZE 16

// flags
bool objDump = 0;

// assembler code

InstructionDef instruction_table[] = {
	{ "mov", 0x00, 2, encode_mov, get_size_mov },
	{ "hlt", 0x03, 0, encode_hlt, get_size_hlt },
	{ "nop", 0x13, 0, encode_nop, get_size_nop }
};
int table_size = sizeof(instruction_table) / sizeof(InstructionDef);

void first_pass(const TokenList *tokens, SymbolTable *symbol_table, AssemblingSegmentTable *segment_table) {
	int tok_idx = 0;
	Token current_token;
	consume_token(&tok_idx, &current_token, tokens);

	AssemblingSegment *current_segment = 0;
	find_segment(segment_table, &current_segment, "code");

	while (current_token.type != TOKEN_EOF) {
		if (current_token.type == TOKEN_LABEL) {
			// locally store string and consume token
			char label[strlen(current_token.str_val) + 1];
			strcpy(label, current_token.str_val);
			consume_token(&tok_idx, &current_token, tokens);
			if (current_token.type == TOKEN_SYMBOL && current_token.str_val[0] == ':') {
				add_symbol(symbol_table, current_segment, label, current_segment->size);
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
				current_segment->size++;
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
			const Token mnemonic = current_token;
			consume_token(&tok_idx, &current_token, tokens);

			ParsedOperand operands[32];
			int operand_count = 0;

			while (operand_count < inst->operand_count) {
				operands[operand_count++] = operand_parser(tokens, symbol_table, &tok_idx, &current_token, NULL,
					segment_table, *current_segment, mnemonic);
				if (current_token.type == TOKEN_SYMBOL && current_token.str_val[0] == ',') consume_token(&tok_idx, &current_token, tokens);
				if (current_token.type == TOKEN_EOF) break;
				if (operand_count >= 32) {
					fprintf(stderr, "Too many operands\n");
					exit(1);
				}
			}

			const uint8_t size = inst->get_size(operand_count, inst->operand_count, operands);
			current_segment->size += size;

			continue;
		}

		// fallback - consume token
		consume_token(&tok_idx, &current_token, tokens);
	}

	for (int i = 0; i < segment_table->count; i++) {
		segment_table->segments[i].size = 0;
	}

}

void correct_reloc_offset(const struct RelocationTable *reloc_table, const int operand_num) {
	if (operand_num == 2) {
		reloc_table->relocations[reloc_table->numRelocations-1].segment_offset += 2;
	} else if (operand_num == 1) {
		reloc_table->relocations[reloc_table->numRelocations-1].segment_offset += 1;
	}
}

void second_pass(const TokenList *tokens, SymbolTable *table, const AssemblingSegmentTable *segment_table, struct RelocationTable *reloc_table) {
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
				fprintf(stderr, "segmentation fault");
				exit(1);
			} else {
				fprintf(stderr, "Illegal directive\n---->%s", current_token.str_val);
				exit(1);
			}
		}

		if (current_token.type == TOKEN_MNEMONIC) {
			const InstructionDef *inst = find_instruction(instruction_table, table_size, current_token.str_val);
			const Token mnemonic = current_token;
			consume_token(&tok_idx, &current_token, tokens);

			ParsedOperand operands[2];
			int operand_count = 0;

			while (operand_count < inst->operand_count) {
				operands[operand_count++] = operand_parser(tokens, table, &tok_idx, &current_token, reloc_table,
					segment_table, *current_segment, mnemonic);
				if (operands[operand_count-1].imm == 0x7FFFFFFF) correct_reloc_offset(reloc_table, operand_count);
				if (current_token.type == TOKEN_SYMBOL && current_token.str_val[0] == ',') consume_token(&tok_idx, &current_token, tokens);
				if (current_token.type == TOKEN_EOF) break;
				if (operand_count >= 3) {
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
}

void free_lines(char **lines, const int num_lines) {
	for (int i = 0; i < num_lines; i++) {
		free(lines[i]);
	}
	free(lines);
}

int assemble(const char *input, const char *output) {
	int retval = 0;

	SymbolTable symbol_table;
	init_table(&symbol_table);

	int line_capacity = LINE_NUM_BASE_SIZE;
    char **lines = malloc(line_capacity * sizeof(char *));
	if (!lines) {
		printf("Memory allocation error\n");
		retval = 1;
		goto free_sTable;
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
		free(segmentTable.segments);
		retval = 1;
		goto lines_free;
	}
	appendSegment(&segmentTable, codeSegment);

	struct RelocationTable relocationTable;
	relocationTable.capacity = 8;
	relocationTable.numRelocations = 0;
	relocationTable.relocations = malloc(relocationTable.capacity * sizeof(struct RelocationEntry));
	if (!relocationTable.relocations) {
		perror("malloc");
		retval = 1;
		goto segment_free;
	}

	FILE *in = fopen(input, "r");
	if (!in) {
		printf("Error opening input file\n");
		retval = 1;
		goto segment_free;
	}

	int num_lines = 0;
	char buffer[MAX_LINE_LENGTH];
	while (fgets(buffer, MAX_LINE_LENGTH, in)) {
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

	fclose(in);

	const TokenList tokens = tokenise(lines);

	first_pass(&tokens, &symbol_table, &segmentTable);

	second_pass(&tokens, &symbol_table, &segmentTable, &relocationTable);

	struct ObjectFile obj = generateFileStruct(&symbol_table, &segmentTable, &relocationTable);
	if (objDump) dumpObjectFile(&obj);
	writeObjectFile(&obj, output);

	// free everything
	free_obj:
		freeObjectFile(&obj);
	tokens_free:
		freeTokenList(&tokens);
	reloc_free:
		free(relocationTable.relocations);
	segment_free:
		freeSegmentTable(&segmentTable);
	lines_free:
		free_lines(lines, num_lines);
	free_sTable:
		free_table(&symbol_table);

	return retval;
}
