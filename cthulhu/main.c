#include "ast/compile.h"

#include <stdbool.h>
#include <stdio.h>


static const char *name = NULL;

static const char *output = "a.out";

#define HELP_ARG "--help"
#define OUTPUT_ARG "--output"

static void print_help(void) {
    printf("%s: cthulhu compiler\n", name);
    puts("\t" HELP_ARG ": print this message");
    puts("\t" OUTPUT_ARG " <name>: set output name (default a.out)")
}

int main(int argc, const char **argv) {
    name = argv[0];
    
    if (argc == 1) {
        print_help();
    }

    return 0;
}
