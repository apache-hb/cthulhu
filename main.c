#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    int number = atoi(argv[1]);

    printf(
        ".globl main\n"
        "main:\n"
        "mov $%d, %%rax\n"
        "ret\n",
        number
    );
}
