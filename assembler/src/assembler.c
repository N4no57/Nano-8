//
// Created by Bernardo on 29/09/2025.

#include "../include/assembler.h"
#include "../include/instructionDef.h"
#include "../include/symbolTable.h"
#include "../include/tokeniser.h"
#include "../include/utils.h"
#include "../include/objectFileWriter.h"
#include "../include/flags.h"

#include <ctype.h>
#include <io.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYMBOL_TABLE_BASE_SIZE 16
#define MAX_LINE_LENGTH 1024
#define LINE_NUM_BASE_SIZE 16

bool isPassTwo = false;

InstructionDef instruction_table[] = {
	{"mov", 0x00, 2, encode_mov, get_size_mov },
	{"push", 0x20, 1, encode_push, get_size_push },
	{"pop", 0x40, 1, encode_pop, get_size_pop },
	{"inb", 0x60, 2, encode_inb, get_size_inb },
	{"outb", 0x80, 2, encode_outb, get_size_outb },
	{"add", 0x01, 2, encode_complexArith, get_size_add },
	{"sub", 0x11, 2, encode_complexArith, get_size_sub },
	{ "cmp", 0x21, 2, encode_complexArith, get_size_sub}, // just a fancy subtract that throws away the value
	{"inc", 0x31, 1, encode_simpleArith, get_size_inc },
	{"dec", 0x41, 1, encode_simpleArith, get_size_dec },
	{"mul", 0x51, 2, encode_complexArith, get_size_mul },
	{"div", 0x61, 2, encode_complexArith, get_size_div },
	{"and", 0x71, 2, encode_complexArith, get_size_and },
	{"or", 0x81, 2, encode_complexArith, get_size_or },
	{"xor", 0x91, 2, encode_complexArith, get_size_xor },
	{"not", 0xA1, 1, encode_simpleArith, get_size_not },
	{"shl", 0xB1, 1, encode_simpleArith, get_size_shl },
	{"shr", 0xC1, 1, encode_simpleArith, get_size_shr },
	{"jmp", 0x02, 1, encode_jmp, get_size_jmp },
	{"jz", 0x12, 1, encode_jmp, get_size_jz },
	{"jnz", 0x22, 1, encode_jmp, get_size_jnz },
	{"jc", 0x32, 1, encode_jmp, get_size_jc },
	{"jnc", 0x42, 1, encode_jmp, get_size_jnc },
	{"jo", 0x52, 1, encode_jmp, get_size_jo },
	{"jno", 0x62, 1, encode_jmp, get_size_jno },
	{"jn", 0x72, 1, encode_jmp, get_size_jn },
	{"jnn", 0x82, 1, encode_jmp, get_size_jnn },
	{"call", 0x92, 1, encode_jmp, get_size_call },
	{"ret", 0xA2, 0, encode_opcode, get_size_ret },
	{"hlt", 0x03, 0, encode_opcode, get_size_hlt },
	{"nop", 0x13, 0, encode_opcode, get_size_nop },
	{"cli", 0x23, 0, encode_opcode, get_size_cli },
	{"sti", 0x33, 0, encode_opcode, get_size_sti },
	{"iret", 0x43, 0, encode_opcode, get_size_iret },
};
int table_size = sizeof(instruction_table) / sizeof(InstructionDef);

void free_lines(char **lines, const int num_lines) {
	for (int i = 0; i < num_lines; i++) {
		free(lines[i]);
	}
	free(lines);
}

void get_lines(char ***lines, const char *filename, int *num_lines, int *line_capacity) {
	FILE *in = fopen(filename, "r");
	if (!in) {
		printf("Error opening input file\n");
		exit(1);
	}

	char buffer[MAX_LINE_LENGTH];
	while (fgets(buffer, MAX_LINE_LENGTH, in)) {
		if (*num_lines >= *line_capacity - 1) {
			char **tmp = realloc(*lines, *line_capacity * 2 * sizeof(char *));
			if (!tmp) {
				printf("Memory allocation error\n");
				free_lines(*lines, *num_lines);
				exit(1);
			}
			*lines = tmp;
			*line_capacity *= 2;
		}
		(*lines)[(*num_lines)++] = strdup(buffer);
	}
	(*lines)[*num_lines] = NULL;

	fclose(in);
}

void includeFile(const char *fileName, TokenList *tokenList, const int pos) {
	int line_capacity = LINE_NUM_BASE_SIZE;
	int num_lines = 0;
	char **lines = malloc(line_capacity * sizeof(char *));
	if (!lines) {
		printf("Memory allocation error\n");
		exit(1);
	}

	FILE *ftmp = fopen(fileName, "r");
	if (ftmp == NULL) {
		printf("File not found: %s\n", fileName);
		exit(1);
	}
	fclose(ftmp);

	get_lines(&lines, fileName, &num_lines, &line_capacity);

	const TokenList newTokens = tokenise(lines);

	size_t needed = tokenList->count + newTokens.count - 1;
	if (needed > tokenList->capacity) {
		Token *tmp = realloc(tokenList->data, needed * sizeof(Token));
		if (!tmp) {
			perror("Memory allocation error");
			exit(1);
		}
		tokenList->data = tmp;
		tokenList->capacity = needed;
	}

	// move tail upwards to make room
	memmove(tokenList->data + pos + newTokens.count-1,
		tokenList->data + pos, (tokenList->count - pos) * sizeof(Token));

	// splice in new data
	memcpy(tokenList->data + pos, newTokens.data, (newTokens.count-1) * sizeof(Token));

	tokenList->count += newTokens.count-1;
}

void first_pass(TokenList *tokens, SymbolTable *symbol_table, AssemblingSegmentTable *segment_table) {
	if (verbose) printf("First pass: Collecting Symbols\n");
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
				if (verbose) printf("Defining symbol: %s = 0x%02x\n", label, (uint32_t)current_segment->size);
				consume_token(&tok_idx, &current_token, tokens);
				continue;
			}
			printf("Invalid Label usage");
			exit(1);
		}

		if (current_token.type == TOKEN_DIRECTIVE) {
			if (strcmp(current_token.str_val, ".db") == 0) {
				consume_token(&tok_idx, &current_token, tokens);
				if (is_base_mod(current_token)) consume_token(&tok_idx, &current_token, tokens);
				consume_token(&tok_idx, &current_token, tokens);
				current_segment->size++;
				continue;
			}
			if (strcmp(current_token.str_val, ".dw") == 0) {
				consume_token(&tok_idx, &current_token, tokens);
				if (is_base_mod(current_token)) consume_token(&tok_idx, &current_token, tokens);
				consume_token(&tok_idx, &current_token, tokens);
				current_segment->size+=2;
				continue;
			}
			if (strcmp(current_token.str_val, ".dd") == 0) {
				consume_token(&tok_idx, &current_token, tokens);
				if (is_base_mod(current_token)) consume_token(&tok_idx, &current_token, tokens);
				consume_token(&tok_idx, &current_token, tokens);
				current_segment->size+=4;
				continue;
			}
			if (strcmp(current_token.str_val, ".dq") == 0) {
				consume_token(&tok_idx, &current_token, tokens);
				if (is_base_mod(current_token)) consume_token(&tok_idx, &current_token, tokens);
				consume_token(&tok_idx, &current_token, tokens);
				current_segment->size+=8;
				continue;
			}
			if (strcmp(current_token.str_val, ".segment") == 0) {
				consume_token(&tok_idx, &current_token, tokens);
				char segName[16];
				if (current_token.type != TOKEN_LABEL) {
					fprintf(stderr, "Unexpected token '%s'\n", current_token.str_val);
					exit(1);
				}
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
			}
			if (strcmp(current_token.str_val, ".include") == 0) {
				consume_token(&tok_idx, &current_token, tokens); // move past .include directive
				if (current_token.type != TOKEN_LABEL) {
					fprintf(stderr, "Unexpected token '%s'\n", current_token.str_val);
					exit(1);
				}
				includeFile(current_token.str_val, tokens, tok_idx);
				consume_token(&tok_idx, &current_token, tokens); // eat the filename token
				continue;
			}
			fprintf(stderr, "Illegal directive\n---->%s", current_token.str_val);
			exit(1);
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
	int reloc_index = reloc_table->numRelocations-1;
	if (operand_num == 2) {
		reloc_table->relocations[reloc_index].segment_offset += 2;
	} else if (operand_num == 1) {
		reloc_table->relocations[reloc_index].segment_offset += 1;
	}
	if (verbose) {
		char type[10];
		if (reloc_table->relocations[reloc_index].type == RELOC_ABSOLUTE) strcpy(type, "ABSOLUTE");
		else if (reloc_table->relocations[reloc_index].type == RELOC_RELAX) strcpy(type, "RELAXED");
		else strcpy(type, "RELATIVE");
		printf("Relocation: symbol %s at offset %02x type=%s\n",
		reloc_table->relocations[reloc_index].name,
		reloc_table->relocations[reloc_index].segment_offset,
		type);
	}
}

void second_pass(const TokenList *tokens, SymbolTable *table, const AssemblingSegmentTable *segment_table, struct RelocationTable *reloc_table) {
	if (verbose) printf("Pass 2: Encoding instructions\n");
	int tok_idx = 0;

	isPassTwo = true;

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

				current_segment->data[current_segment->size++] = current_token.int_value & 0xFF;
				consume_token(&tok_idx, &current_token, tokens);
				continue;
			}
			if (strcmp(current_token.str_val, ".dw") == 0) {
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

				current_segment->data[current_segment->size++] = current_token.int_value & 0xFF;
				current_segment->data[current_segment->size++] = current_token.int_value >> 8 & 0xFF;
				consume_token(&tok_idx, &current_token, tokens);
				continue;
			}
			if (strcmp(current_token.str_val, ".dd") == 0) {
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

				current_segment->data[current_segment->size++] = current_token.int_value & 0xFF;
				current_segment->data[current_segment->size++] = current_token.int_value >> 8 & 0xFF;
				current_segment->data[current_segment->size++] = current_token.int_value >> 16 & 0xFF;
				current_segment->data[current_segment->size++] = current_token.int_value >> 24 & 0xFF;
				consume_token(&tok_idx, &current_token, tokens);
				continue;
			}
			if (strcmp(current_token.str_val, ".dq") == 0) {
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

				current_segment->data[current_segment->size++] = current_token.int_value & 0xFF;
				current_segment->data[current_segment->size++] = current_token.int_value >> 8 & 0xFF;
				current_segment->data[current_segment->size++] = current_token.int_value >> 16 & 0xFF;
				current_segment->data[current_segment->size++] = current_token.int_value >> 24 & 0xFF;
				current_segment->data[current_segment->size++] = current_token.int_value >> 32 & 0xFF;
				current_segment->data[current_segment->size++] = current_token.int_value >> 40 & 0xFF;
				current_segment->data[current_segment->size++] = current_token.int_value >> 48 & 0xFF;
				current_segment->data[current_segment->size++] = current_token.int_value >> 56 & 0xFF;
				consume_token(&tok_idx, &current_token, tokens);
				continue;
			}
			if (strcmp(current_token.str_val, ".segment") == 0) {
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
			}
			if (strcmp(current_token.str_val, ".include") == 0) {
				consume_token(&tok_idx, &current_token, tokens);
				consume_token(&tok_idx, &current_token, tokens);
				continue;
			}
			fprintf(stderr, "Illegal directive\n---->%s", current_token.str_val);
			exit(1);
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

int assemble(const char *input, const char *output) {
	int retval = 0;

	SymbolTable symbol_table;
	init_table(&symbol_table);

	int line_capacity = LINE_NUM_BASE_SIZE;
	int num_lines = 0;
    char **lines = malloc(line_capacity * sizeof(char *));
	if (!lines) {
		printf("Memory allocation error\n");
		retval = 1;
		goto free_sTable;
	}

	get_lines(&lines, input, &num_lines, &line_capacity);

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

	TokenList tokens = tokenise(lines);

	first_pass(&tokens, &symbol_table, &segmentTable);

	second_pass(&tokens, &symbol_table, &segmentTable, &relocationTable);

	const struct ObjectFile obj = generateFileStruct(&symbol_table, &segmentTable, &relocationTable);
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
