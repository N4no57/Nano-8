#include "../include/objectFileReader.h"
#include "../include/linker.h"
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#define DEFAULT_FILENAME "o.bin"

int main(int argc, char **argv) {
    char *output = DEFAULT_FILENAME;

    static struct option long_options[] = {
        {NULL, 0, 0, 0}
    };

    opterr = 0;

    int option;
    int option_index = 0;
    while ((option = getopt_long(argc, argv, "ho:", long_options, &option_index)) != -1) {
        switch (option) {
            case 'h':
                printf("Usage: nano8-ld [options]\n");
                printf("Options:\n");
                printf("  -h                 Display this help screen\n");
				printf("  -o <file>          Place the output into <file>\n");
                return 0;
            case 'o':
                output = optarg;
                break;
            default:
                break;
        }
    }

    const struct ObjectFile obj = readObjectFile("test.o");
    dumpObjectFile(&obj);

    linker(&obj, 1, output);

    return 0;
}