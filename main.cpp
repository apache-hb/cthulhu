#include <stdio.h>
#include <stdlib.h>

int compile(FILE *out, char *source) {
    int line = 0;
    int col = 0;
    char c;

    /* emit prelude */
    fprintf(out,
        ".globl _start\n"
        ".section .text\n"
        "_start:\n"
    );

    fprintf(out, "  mov $%ld, %%rdi\n", strtol(source, &source, 10));

    while ((c = *source++)) {
        line++;
        switch (c) {
        case '+':
            fprintf(out, "  add $%ld, %%rdi\n", strtol(source, &source, 10));
            break;
        case '-':
            fprintf(out, "  sub $%ld, %%rdi\n", strtol(source, &source, 10));
            break;
        case ' ': case '\t': case '\r':
            /* skip whitespace */
            continue;
        case '\n':
            line = 0;
            col++;
            continue;
        default:
            fprintf(stderr, "unexpected character `%c` at %d:%d \n", c, line, col);
            return 1;
        }
    }

    fprintf(out, 
        "  mov $60, %%rax\n"
        "  syscall\n"
    );

    return 0;
}

int main(int argc, char **argv) {
    FILE *output = stdout;
    const char *name = "stdout";
    const char *self = argv[0];
    char *source;
    int result = 0;

    if (argc < 2) {
        fprintf(stderr, 
            "%s: invalid number of arguments\n"
            "   %s <number>\n"
            "   %s <number> <file>\n", 
            self, self, self
        );
        return 1;
    }

    source = argv[1];

    if (argc >= 3) {
        name = argv[2];
        output = fopen(argv[2], "w");
    }

    if (output == NULL) {
        fprintf(stderr, "failed to open output file `%s`\n", name);
        return 1;
    }

    result = compile(output, source);

    if (result != 0) {
        if (output != stdout) {
            fclose(output);
            remove(name);
        }
    }

    return result;
}
