#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, 
            "%s: invalid number of arguments\n"
            "   %s <number>\n"
            "   %s <number> <file>\n", 
            argv[0], argv[0], argv[0]
        );
        return 1;
    }

    FILE* output = stdout;

    if (argc >= 3) {
        output = fopen(argv[2], "w");
    }

    if (output == NULL) {
        fprintf(stderr, "failed to open output file\n");
        return 1;
    }

    long long number = atoll(argv[1]);

    fprintf(output,
        ".globl main\n"
        "main:\n"
        "mov $%lld, %%rax\n"
        "ret\n",
        number
    );
}
