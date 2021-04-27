#include <stdio.h>
#include <stdlib.h>

int emit(FILE *out, char *source) {
    char c;

    /* emit prelude */
    fprintf(out,
        "  .globl main\n"
        "main:\n"
    );

    fprintf(out, "  mov $%ld, %%rax\n", strtol(source, &source, 10));

    while ((c = *source++)) {
        switch (c) {
        case '+':
            fprintf(out, "  add $%ld, %%rax\n", strtol(source, &source, 10));
            break;
        case '-':
            fprintf(out, "  sub $%ld, %%rax\n", strtol(source, &source, 10));
            break;
        case ' ': case '\t': case '\r': case '\n':
            continue;
            
        default:
            fprintf(stderr, "unexpected character `%c`\n", c);
            return 1;
        }
    }

    fprintf(out, "  ret\n");

    return 0;
}

int main(int argc, char **argv) {
    FILE *output = stdout;
    const char *name = "stdout";
    const char *self = argv[0];
    char *source;

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

    return emit(output, source);
}
