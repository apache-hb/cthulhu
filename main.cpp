#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

struct token {
    enum { DIGIT, OP, END } type;
    enum op_t { ADD, SUB };

    union {
        long digit;
        op_t op;
    } data;
};

struct lexer {
    char* text;

    char next() {
        char c = *text;
        if (c != 0) {
            text++;
        }
        return c;
    }

    char peek() {
        return *(text + 1);
    }

    char skip() {
        char c = next();

        while (isspace(c) && c != 0)
            c = next();

        return c;
    }

    token read() {
        char c = skip();

        if (c == '+') {
            return { token::OP, { token::ADD } };
        } else if (c == '-') {
            return { token::OP, { token::SUB } };
        } else if (isdigit(c)) {
            return { token::DIGIT, { strtol(text - 1, &text, 10) } };
        } else if (c == 0 || c == -1) {
            return { token::END, {} };
        } else {
            fprintf(stderr, "unknown character `%c`\n", c);
            exit(1);
        }
    }
};

int compile(FILE *out, char *source) {
    /* emit prelude */
    fprintf(out,
        ".globl _start\n"
        ".section .text\n"
        "_start:\n"
    );

    lexer lex = { source };

    token c = lex.read();

    fprintf(out, "  mov $%ld, %%rdi\n", c.data.digit);

    while (c.type != token::END) {
        token o = lex.read();
        c = lex.read();

        switch (o.data.op) {
        case token::ADD:
            fprintf(out, "  add $%ld, %%rdi\n", c.data.digit);
            break;
        case token::SUB:
            fprintf(out, "  sub $%ld, %%rdi\n", c.data.digit);
            break;
        default:
            fprintf(stderr, "unknown token\n");
            exit(1);
        }
    }

    fprintf(out, 
        "  mov $60, %%rax\n"
        "  syscall\n"
    );

    return 0;
}

void help(const char* name) {
    fprintf(stderr, 
        "%s: invalid number of arguments\n"
        "   <expr>\n"
        "   -h: display this message\n"
        "   -o: output file name\n", 
        name
    );

    exit(1);
}

int main(int argc, char **argv) {
    FILE *output;
    const char *self = argv[0];
    char *source;
    int result = 0;

    if (argc < 2) {
        help(self);
    }

    source = argv[1];

    output = fopen("temp.s", "w");

    if (output == NULL) {
        fprintf(stderr, "failed to open output file\n");
        return 1;
    }

    result = compile(output, source);
    fclose(output);

    if (result != 0) {
        return result;   
    }

    system("as temp.s -o temp.o");
    system("ld temp.o -o a.out");

    remove("temp.s");
    remove("temp.o");
}
