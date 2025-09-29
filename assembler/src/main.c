#include "../include/assembler.h"
#include "../include/flags.h"
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

#define OUTPUT_DEFAULT "a.o"
#define ASSEMBLER_VERSION "1.1.0"

// flags
bool objDump = 0;
bool verbose = 0;

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

int hasExtension(const char *string) {
	for (int i = 0; i < strlen(string); i++) {
		if (string[i] == '.') {
			return 1;
		}
	}
	return 0;
}

int main(const int argc, char **argv) {
	char *output = OUTPUT_DEFAULT;

	static struct option long_options[] = {
		{"objdump", no_argument, 0, 0},
		{"version", no_argument, 0, 0},
		{0, 0, 0, 0}
	};

	opterr = 0;

	int option;
	int option_index = 0;
	while ((option = getopt_long(argc, argv, "ho:v", long_options, &option_index)) != -1) {
		switch (option) {
			case 'h':
				printf("Usage: nano8-as [options] file...\n");
				printf("Options:\n");
				printf("  --version          Display current version\n");
				printf("  -h                 Display this help screen\n");
				printf("  -o <file>          Place the output into <file>\n");
				printf("  --objdump          Prints the object file to stdout\n");
				return 0;
			case 'o':
				output = optarg;
				break;
			case 'v':
				verbose = true;
				break;
			case 0:
				if (strcmp(long_options[option_index].name, "objdump") == 0) {
					objDump = true;
				} else if (strcmp(long_options[option_index].name, "version") == 0) {
					printf("nano8-as.exe (built by Bernardo Oliveira) %s\n", ASSEMBLER_VERSION);
					return 0;
				}
				break;
			case '?':
				printf("nano8-as: error: unrecognised command-line option '%s'\n", argv[optind-1]);
				break;
			default:
				printf("nano8-as: error: unrecognised command-line option '-%c'\n", option);
				break;
		}
	}


	const int length = (int)strlen(output);
	char o[length+3];
	if (output != OUTPUT_DEFAULT) {
		if (!hasExtension(output)) {
			strcpy(o, output);
			o[length] = '.';
			o[length+1] = 'o';
			o[length+2] = 0;
			output = o;
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
			if (verbose) printf("Assembling file: %s\n", input[i]);
			assemble(input[i], changeFileExt(input[i]));
		}
		return 0;
	}
	if (verbose) printf("Assembling file: %s\n", input[0]);
	return assemble(input[0], output);
}
