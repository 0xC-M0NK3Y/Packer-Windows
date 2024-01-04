/*#include <stdio.h>
#include <stdlib.h>
#include <argp.h>

#include "struct.h"

struct arguments {
    char *compressor;
    char *algorithm;
    char *level;
    char *other_argument;Q
};

static struct argp_option options[] = {
    {"compressor", 'c', "COMPRESSOR", 0, "Specify a compressor dll containing compress and decompress function, overrides --algorithm\n"},
    {"algorithm",  'a', "ALGORITHM", 0,  "Specify the algorithm, default xz"},
    {"level",      'l', "LEVEL", 0,      "Specify the level, default 1"},
    {0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
    case 'c':
        arguments->compressor = arg;
        break;
    case 'a':
        arguments->algorithm = arg;
        break;
    case 'l':
        if (!isdigit(arg[0]))
            argp_usage(state);
        arguments->level = arg;
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num == 0)
            arguments->other_argument = arg;
        else
            argp_usage(state);
        break;
    case ARGP_KEY_END:
        if (state->arg_num < 1)
            argp_usage(state);
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, "EXECUTABLE", "Packer supporting multiple compression algorithms."};

int parse_arguments(int argc, char **argv, packer_info_t *infos) {
    struct arguments arguments;

    arguments.compressor = NULL;
    arguments.algorithm = NULL;
    arguments.level = NULL;
    arguments.other_argument = NULL;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    printf("Compressor: %s\n", arguments.compressor);
    printf("Algorithm: %s\n", arguments.algorithm);
    printf("Level: %s\n", arguments.level);
    printf("Other Argument: %s\n", arguments.other_argument);

    infos->executable   = arguments.other_argument;
    infos->compress_dll = arguments.compressor;
    infos->level        = atoi(arguments.level);
    infos->algorithm    = arguments.algorithm;

    return 0;
}
*/