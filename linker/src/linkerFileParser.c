//
// Created by brego on 01/10/2025.
//

#include "../include/linkerFileParser.h"
#include "../include/flags.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct KeyValue {
    char key[16];
    enum { VAL_NUMBER, VAL_STRING, VAL_IDENT } type;
    union {
        uint32_t number;
        char string[32];
    };
};


enum TokenType {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_SYMBOL,
    TOKEN_EOF
};

typedef struct Token {
    enum TokenType type;
    char str_val[32];
    long int_val;
} Token;

Token *tokens = NULL;
int token_count = 0;
int token_capacity = 8;
int token_idx = 0;
Token current_token;

void consume_token() {
    if (current_token.type == TOKEN_EOF) {
        printf("Token EOF\n");
        return;
    }
    current_token = tokens[++token_idx];
}

void printToken(const Token tok) {
    char type[32];
    switch (tok.type) {
        case TOKEN_IDENTIFIER:
            strcpy(type, "T_IDENTIFIER");
            break;
        case TOKEN_NUMBER:
            strcpy(type, "T_NUMBER");
            break;
        case TOKEN_STRING:
            strcpy(type, "T_STRING");
            break;
        case TOKEN_SYMBOL:
            strcpy(type, "T_SYMBOL");
            break;
        case TOKEN_EOF:
            strcpy(type, "T_EOF");
            break;
    }

    if (tok.type == TOKEN_NUMBER) {
        printf("%s: %ld\n", type, tok.int_val);
        return;
    }
    printf("%s: %s\n", type, tok.str_val);
}

void tokenDump() {
    for (int i = 0; i < token_count; i++) {
        printToken(tokens[i]);
    }
}

int matches(const enum TokenType type, char *str_val, const long int_val) {
    if (type == TOKEN_NUMBER && current_token.type == TOKEN_NUMBER) {
        return current_token.int_val == int_val;
    }
    return current_token.type == type && strcmp(current_token.str_val, str_val) == 0;
}

void push_token(const Token token) {
    if (token_count >= token_capacity) {
        token_capacity *= 2;
        Token *tmp = realloc(tokens, token_capacity * sizeof(Token));
        if (!tmp) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        tokens = tmp;
    }

    tokens[token_count++] = token;
}

int get_base(const char c) {
    if (c == '$') {
        return 16;
    }
    if (c == '%') {
        return 2;
    }
    return 10;
}

int ishexdigit(char c) {
    if (c >= 'a' && c <= 'f') {
        return 1;
    }
    if (c >= 'A' && c <= 'F') {
        return 1;
    }
    if (isdigit(c)) {
        return 1;
    }
    return 0;
}

void tokenise(const char *input) {
    int i = 0;

    while (input[i] != '\0') {
        if (isspace(input[i])) {
            i++;
        }

        // symbols
        if (input[i] == ':' || input[i] == '=' || input[i] == ';' ||
            input[i] == '{' || input[i] == '}') {
            Token t = {0};
            t.type = TOKEN_SYMBOL;
            t.str_val[0] = input[i];
            t.str_val[1] = '\0';
            push_token(t);
            i++;
            continue;
        }

        // strings
        if (input[i] == '"') {
            i++;
            char buff[32]; int bi = 0;
            while ((isalnum(input[i]) || input[i] == '_') && input[i] != '"') {
                buff[bi++] = input[i++];
            }
            buff[bi] = '\0';
            i++;
            Token t = {0};
            t.type = TOKEN_STRING;
            strcpy(t.str_val, buff);
            push_token(t);
            continue;
        }

        // number
        if (isdigit(input[i]) || input[i] == '$' || input[i] == '%') {
            const int base = get_base(input[i]);
            if (base != 10) {
                i++;
            }
            char buff[32]; int bi = 0;
            while (ishexdigit(input[i])) {
                buff[bi++] = input[i++];
            }
            buff[bi] = '\0';
            const Token t = { .type = TOKEN_NUMBER, .int_val = strtol(buff, NULL, base) };
            push_token(t);
            continue;
        }

        // identifier
        if (isalpha(input[i])) {
            char buff[32]; int bi = 0;
            while (isalnum(input[i]) || input[i] == '_') {
                buff[bi++] = input[i++];
            }
            buff[bi] = '\0';

            Token t = {0};
            t.type = TOKEN_IDENTIFIER;
            strcpy(t.str_val, buff);
            push_token(t);
            continue;
        }

        // fallback
        i++;
    }

    const Token t = {TOKEN_EOF};
    push_token(t);
}

int memRegion_count = 0;
int memRegion_capacity = 8;
int segRule_count = 0;
int segRule_capacity = 8;

int keyValueCount = 0;
int keyValueCapacity = 8;

void push_memRegion(struct MemoryRegion **memRegions, char name[16], struct KeyValue *pairs) {
    if (memRegion_count >= memRegion_capacity) {
        memRegion_capacity *= 2;
        struct MemoryRegion *tmp = realloc(*memRegions, memRegion_capacity * sizeof(struct MemoryRegion));
        if (!tmp) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        *memRegions = tmp;
    }
    // optional field: fill

    strcpy((*memRegions)[memRegion_count].name, name);
    (*memRegions)[memRegion_count].size = 0xFFFFFFFF;
    (*memRegions)[memRegion_count].start = 0xFFFFFFFF;
    (*memRegions)[memRegion_count].fill = 0;
    (*memRegions)[memRegion_count].current_offset = 0;

    for (int i = 0; i < keyValueCount; i++) {
        if (strcmp(pairs[i].key, "start") == 0 && pairs[i].type == VAL_NUMBER) {
            (*memRegions)[memRegion_count].start = pairs[i].number;
        } else if (strcmp(pairs[i].key, "size") == 0 && pairs[i].type == VAL_NUMBER) {
            (*memRegions)[memRegion_count].size = pairs[i].number;
        } else if (strcmp(pairs[i].key, "fill") == 0 && pairs[i].type == VAL_IDENT) {
            (*memRegions)[memRegion_count].fill = (strcmp(pairs[i].string, "yes") == 0);
        } else {
            fprintf(stderr, "Unknown or invalid key %s for MEMORY\n", pairs[i].key);
            exit(EXIT_FAILURE);
        }
    }

    if ((*memRegions)[memRegion_count].start > 0xFFFF && (*memRegions)[memRegion_count].size <= 0xFFFF) {
        printf("Memory region \"%s\" requires a start address", (*memRegions)[memRegion_count].name);
        exit(EXIT_FAILURE);
    }
    if ((*memRegions)[memRegion_count].start <= 0xFFFF && (*memRegions)[memRegion_count].size > 0xFFFF) {
        printf("Memory region \"%s\" requires a size", (*memRegions)[memRegion_count].name);
        exit(EXIT_FAILURE);
    }
    if ((*memRegions)[memRegion_count].start > 0xFFFF && (*memRegions)[memRegion_count].size > 0xFFFF) {
        printf("Memory region \"%s\" requires a start address and size", (*memRegions)[memRegion_count].name);
        exit(EXIT_FAILURE);
    }

    if (verbose) {
        printf("Memory Region:\n");
        printf("\tname: %s\n", (*memRegions)[memRegion_count].name);
        printf("\tsize: %d\n", (*memRegions)[memRegion_count].size);
        printf("\tstart: %d\n", (*memRegions)[memRegion_count].start);
        printf("\tcurrent_offset: %d\n", (*memRegions)[memRegion_count].current_offset);
        printf("\tfill: %d\n", (*memRegions)[memRegion_count].fill);
    }

    memRegion_count++;
}

void push_segRule(struct SegmentRule **segRules, char name[16], struct KeyValue *pairs) {
    if (segRule_count >= segRule_capacity) {
        segRule_capacity *= 2;
        struct SegmentRule *tmp = realloc(*segRules, segRule_capacity * sizeof(struct SegmentRule));
        if (!tmp) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        *segRules = tmp;
    }
    // optional field: explicit_start

    strcpy((*segRules)[segRule_count].name, name);
    (*segRules)[segRule_count].load_to[0] = '\0';
    (*segRules)[segRule_count].explicit_start = 0xFFFFFFFF;

    for (int i = 0; i < keyValueCount; i++) {
        if (strcmp(pairs[i].key, "load") == 0 && pairs[i].type == VAL_STRING) {
            strcpy((*segRules)[segRule_count].load_to, pairs[i].string);
        } else if (strcmp(pairs[i].key, "start") == 0 && pairs[i].type == VAL_NUMBER) {
            (*segRules)[segRule_count].explicit_start = pairs[i].number;
        } else {
            fprintf(stderr, "Unknown or invalid key %s for SEGMENTS\n", pairs[i].key);
        }
    }

    if (strlen((*segRules)[segRule_count].load_to) == 0) {
        printf("Segment \"%s\" requires a memory chunk to load to", (*segRules)[segRule_count].name);
        exit(EXIT_FAILURE);
    }

    if (verbose) {
        printf("Segment Rule:\n");
        printf("\tname: %s\n", (*segRules)[segRule_count].name);
        printf("\tload_to: %s\n", (*segRules)[segRule_count].load_to);
        if ((*segRules)[segRule_count].explicit_start <= 0xFFFF) {
            printf("\texplicit_start: %d\n", (*segRules)[segRule_count].explicit_start);
        }
    }

    segRule_count++;
}

void push_KeyValue(struct KeyValue **pairs, const char *key, const int type, char str[32], const uint32_t int_val) {
    if (keyValueCount >= keyValueCapacity) {
        keyValueCapacity *= 2;
        struct KeyValue *tmp = realloc(*pairs, keyValueCapacity * sizeof(struct KeyValue));
        if (!tmp) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        *pairs = tmp;
    }
    strcpy((*pairs)[keyValueCount].key, key);

    if (type == VAL_NUMBER) {
        (*pairs)[keyValueCount].type = VAL_NUMBER;
        (*pairs)[keyValueCount].number = int_val;
        keyValueCount++;
        return;
    }
    (*pairs)[keyValueCount].type = type;
    strcpy((*pairs)[keyValueCount].string, str);
    keyValueCount++;
}

void parseKeyValue(struct KeyValue *pairs, char *key) {
    if (current_token.type == TOKEN_NUMBER) {
        push_KeyValue(&pairs, key, VAL_NUMBER, NULL, current_token.int_val);
    } else if (current_token.type == TOKEN_STRING) {
        push_KeyValue(&pairs, key, VAL_STRING, current_token.str_val, 0);
    } else if (current_token.type == TOKEN_IDENTIFIER) {
        push_KeyValue(&pairs, key, VAL_IDENT, current_token.str_val, 0);
    }
    consume_token();
}

void parseMemRegion(struct MemoryRegion *memRegions) {
    // NAME: key = value [, key = value];
    struct KeyValue *pairs = malloc(sizeof(struct KeyValue) * keyValueCapacity);
    char buff[16];
    char key[32];

    if (current_token.type != TOKEN_IDENTIFIER) {
        printf("Error: Expected identifier\n");
        printf("got: ");
        printToken(current_token);
        exit(EXIT_FAILURE);
    }
    strcpy(buff, current_token.str_val);
    consume_token();

    if (!matches(TOKEN_SYMBOL, ":\0",0)) {
        printf("Error: Expected \":\"\n");
        printf("got: ");
        printToken(current_token);
        exit(EXIT_FAILURE);
    }
    consume_token();

    if (current_token.type != TOKEN_IDENTIFIER) {
        printf("Error: Expected identifier\n");
        printf("got: ");
        printToken(current_token);
        exit(EXIT_FAILURE);
    }
    strcpy(key, current_token.str_val);
    consume_token();

    if (!matches(TOKEN_SYMBOL, "=\0",0)) {
        printf("Error: Expected \"=\"\n");
        printf("got: ");
        printToken(current_token);
        exit(EXIT_FAILURE);
    }
    consume_token();

    parseKeyValue(pairs, key);

    while (!matches(TOKEN_SYMBOL, ";\0", 0)) {
        if (current_token.type != TOKEN_IDENTIFIER) {
            printf("Error: Expected identifier\n");
            printf("got: ");
            printToken(current_token);
            exit(EXIT_FAILURE);
        }
        strcpy(key, current_token.str_val);
        consume_token();

        if (!matches(TOKEN_SYMBOL, "=\0",0)) {
            printf("Error: Expected \"=\"\n");
            printf("got: ");
            printToken(current_token);
            exit(EXIT_FAILURE);
        }
        consume_token();

        parseKeyValue(pairs, key);
    }
    consume_token();

    push_memRegion(&memRegions, buff, pairs);
    free(pairs);
    keyValueCount = 0;
    keyValueCapacity = 8;
}

void parseSegRule(struct SegmentRule *segRules) {
    struct KeyValue *pairs = malloc(sizeof(struct KeyValue) * keyValueCapacity);
    char buff[16];
    char key[32];

    if (current_token.type != TOKEN_IDENTIFIER) {
        printf("Error: Expected identifier\n");
        printf("got: ");
        printToken(current_token);
        exit(EXIT_FAILURE);
    }
    strcpy(buff, current_token.str_val);
    consume_token();

    if (!matches(TOKEN_SYMBOL, ":\0",0)) {
        printf("Error: Expected \":\"\n");
        printf("got: ");
        printToken(current_token);
        exit(EXIT_FAILURE);
    }
    consume_token();

    if (current_token.type != TOKEN_IDENTIFIER) {
        printf("Error: Expected identifier\n");
        printf("got: ");
        printToken(current_token);
        exit(EXIT_FAILURE);
    }
    strcpy(key, current_token.str_val);
    consume_token();

    if (!matches(TOKEN_SYMBOL, "=\0",0)) {
        printf("Error: Expected \"=\"\n");
        printf("got: ");
        printToken(current_token);
        exit(EXIT_FAILURE);
    }
    consume_token();

    parseKeyValue(pairs, key);

    while (!matches(TOKEN_SYMBOL, ";\0", 0)) {
        if (current_token.type != TOKEN_IDENTIFIER) {
            printf("Error: Expected identifier\n");
            printf("got: ");
            printToken(current_token);
            exit(EXIT_FAILURE);
        }
        strcpy(key, current_token.str_val);
        consume_token();

        if (!matches(TOKEN_SYMBOL, "=\0",0)) {
            printf("Error: Expected \"=\"\n");
            printf("got: ");
            printToken(current_token);
            exit(EXIT_FAILURE);
        }
        consume_token();

        parseKeyValue(pairs, key);
    }
    consume_token();

    push_segRule(&segRules, buff, pairs);
    free(pairs);
    keyValueCount = 0;
    keyValueCapacity = 8;
}

void parseBlock(struct MemoryRegion *memRegions, struct SegmentRule *rules) {
    // MEMORY "{" [ entry ] "}"
    // SEGMENTS "{" [ entry ] "}"

    if (matches(TOKEN_IDENTIFIER, "MEMORY\0", 0)) {
        consume_token();
        if (!matches(TOKEN_SYMBOL, "{\0", 0)) {
            printf("Error: Expected \"{\"\n");
            printf("got: ");
            printToken(current_token);
            exit(EXIT_FAILURE);
        }
        consume_token();
        while (!matches(TOKEN_SYMBOL, "}\0", 0)) {
            parseMemRegion(memRegions);
        }
        consume_token();
    } else if (matches(TOKEN_IDENTIFIER, "SEGMENTS\0", 0)) {
        consume_token();
        if (!matches(TOKEN_SYMBOL, "{\0", 0)) {
            printf("Error: Expected \"{\n");
            printf("got: ");
            printToken(current_token);
            exit(EXIT_FAILURE);
        }
        consume_token();
        while (!matches(TOKEN_SYMBOL, "}\0", 0)) {
            parseSegRule(rules);
        }
        consume_token();
    } else {
        printf("Error: Expected either \"MEMORY\" or \"SEGMENTS\" config blocks\n");
        printf("got: ");
        printToken(current_token);
        tokenDump();
        exit(EXIT_FAILURE);
    }
}

void parseFile(char *fileName, struct MemoryRegion **memRegions, struct SegmentRule **rules) {
    tokens = malloc(token_capacity * sizeof(Token));
    if (!tokens) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    *memRegions = malloc(memRegion_capacity * sizeof(struct MemoryRegion));
    if (!(*memRegions)) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    *rules = malloc(segRule_capacity * sizeof(struct SegmentRule));
    if (!(*rules)) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(fileName, "r");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char file_buff[2048] = {0};
    fread(file_buff, 1, sizeof(file_buff), file);
    fclose(file);

    tokenise(file_buff);

    current_token = tokens[0];

    while (tokens[token_idx].type != TOKEN_EOF) {
        parseBlock(*memRegions, *rules);
    }

    free(tokens);
}