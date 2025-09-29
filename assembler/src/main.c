#include "../include/assembler.h"
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

#define OUTPUT_DEFAULT "a.o"

const char *changeFileExt(char *string) {
	for (int i = 0; i < strlen(string); i++) {
		if (string[i] == '.') {
			string[i+1] = 'o';
			string[i+2] = '\0';
			break;
		}
	}
	return string;
}

int main(const int argc, char **argv) {
	const char *output = OUTPUT_DEFAULT;

	static struct option long_options[] = {
		{"objdump", no_argument, 0, 0},
		{0, 0, 0, 0}
	};

	opterr = 0;

	int option;
	int option_index = 0;
	while ((option = getopt_long(argc, argv, "ho:", long_options, &option_index)) != -1) {
		switch (option) {
			case 'h':
				printf("Usage: nano8-as [options] file...\n");
				printf("Options:\n");
				printf("  -h                 Display this help screen\n");
				printf("  -o <file>          Place the output into <file>\n");
				return 0;
			case 0:
				if (strcmp(long_options[option].name, "objdump") == 0) {
					objDump = true;
				}
				break;
			case '?':
				printf("nano8-as: error: unrecognised command-line option '%s'\n", argv[optind-1]);
				break;
			default:
				break;
		}
	}

	if (optind == argc) {
		printf("nano8-as: fatal error: no input files\n");
		printf("assembly terminated\n");
		return -1;
	}

	if (argc - optind != 1 && output != OUTPUT_DEFAULT) {
		printf("nano8-as: fatal error: cannot use '-o' when assembling multiple files\n");
		printf("assembly terminated\n");
		return -1;
	}

	char *input[argc - optind];

	for (int i = optind; i < argc; i++) {
		input[i-optind] = argv[i];
	}

	if (argc - optind > 1) {
		for (int i = 0; i < argc - optind; i++) {
			assemble(input[i], changeFileExt(input[i]));
		}
		return 0;
	}
	return assemble(input[0], output);
}
