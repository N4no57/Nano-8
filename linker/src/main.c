#include "../include/objectFileReader.h"
#include "../include/linker.h"
#include "../include/linkerFileParser.h"
#include "../include/flags.h"
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>

#define DEFAULT_FILENAME "o.bin"
#define LINKER_VERSION "1.0.1"

bool configFile = false;
bool verbose = false;

int main(const int argc, char **argv) {
    struct MemoryRegion *a = NULL;
    struct SegmentRule *b = NULL;

    char *output = DEFAULT_FILENAME;

    static struct option long_options[] = {
        {"version", no_argument, 0, 0},
        {NULL, 0, 0, 0}
    };

    opterr = 0;

    int option;
    int option_index = 0;
    while ((option = getopt_long(argc, argv, "ho:C:v", long_options, &option_index)) != -1) {
        switch (option) {
            case 'h':
                printf("Usage: nano8-ld [options]\n");
                printf("Options:\n");
				printf("  --version          Display current version\n");
                printf("  -h                 Display this help screen\n");
				printf("  -o <file>          Place the output into <file>\n");
                printf("  -C <file>.cfg      Specify a linker config file\n");
                return 0;
            case 'o':
                output = optarg;
                break;
            case 'C':
                configFile = true;
                if (verbose) printf("Using config file %s\n", optarg);
                parseFile(optarg, &a, &b);
                break;
            case 'v':
                verbose = true;
                break;
            case 0:
                if (strcmp(long_options[option_index].name, "version") == 0) {
					printf("nano8-ld.exe (built by Bernardo Oliveira) %s\n", LINKER_VERSION);
                } else {
                    printf("nano8-ld: error: unrecognised command-line option '%s'\n", argv[optind-1]);
                    break;
                }
            default:
                printf("nano8-ld: error: unrecognised command-line option '-%c'\n", option);
                break;
        }
    }

    const size_t num_files = argc - optind;

    struct ObjectFile *o = malloc(num_files * sizeof(struct ObjectFile));

    if (num_files == 0) {
        printf("nano8-ld: fatal-error: no input files\n");
        printf("linking terminated");
        free(o);
        return 1;
    }

    for (int i = optind; i < argc; i++) {
        if (verbose) printf("Reading file: %s\n", argv[i]);
        o[i-optind] = readObjectFile(argv[i]);
    }

    linker(o, num_files, output, a, b);

    free(o);
    return 0;
}