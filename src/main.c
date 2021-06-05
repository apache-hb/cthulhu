#include "bison.h"
#include "flex.h"

#include <stdbool.h>

#include "ir.h"
#include "asm/x64.h"

int yyerror(YYLTYPE *yylloc, void *scanner, scanner_t *x, const char *msg) {
    (void)scanner;

    fprintf(stderr, "[%s:%d:%d]: %s\n",
        x->path, 
        yylloc->first_line,
        yylloc->first_column,
        msg
    );

    return 1;
}

static nodes_t *compile(const char *path, FILE *file) {
    int err;
    yyscan_t scan;
    scanner_t extra = { path, NULL };

    if ((err = yylex_init_extra(&extra, &scan))) {
        fprintf(stderr, "yylex_init_extra = %d\n", err);
        return NULL;
    }

    yyset_in(file, scan);

    if ((err = yyparse(scan, &extra))) {
        return NULL;
    }

    yylex_destroy(scan);
    
    return extra.ast;
}

static nodes_t *compile_string(const char *text) {
    int err;
    yyscan_t scan;
    scanner_t extra = { "cmd", NULL };
    YY_BUFFER_STATE buffer;

    if ((err = yylex_init_extra(&extra, &scan))) {
        fprintf(stderr, "yylex_init_extra = %d\n", err);
        return NULL;
    }

    if ((buffer = yy_scan_string(text, scan))) {
        if ((err = yyparse(scan, &extra))) {
            return NULL;
        }

        yy_delete_buffer(buffer, scan);
    }

    yylex_destroy(scan);

    return extra.ast;
}

enum {
    O0,
    O1,
    O2,
    O3
} optimize = O1;

nodes_t *nodes = NULL;

bool emit_ir = false;

FILE *output;

static void fail_fast(const char *message) {
    fprintf(stderr, "error: %s\n", message);
    exit(1);
}

#define NEXT_ARG (argv[idx + 1])

static void parse_arg(int idx, int argc, const char **argv) {
    const char *arg = argv[idx];
    bool has_next = argc > idx;

    if (strcmp(arg, "-e") == 0) {
        if (!has_next) {
            fail_fast("-e argument requires an expression");
        } else {
            nodes = compile_string(NEXT_ARG);
        }
    } else if (strcmp(arg, "-i") == 0) {
        printf(">>> ");
        nodes = compile("stdin", stdin);
    } else if (strncmp(arg, "-O", 2) == 0) {
        char level = '1';
        if (strlen(arg) >= 3) {
            level = arg[2];
        } else if (has_next) {
            level = NEXT_ARG[0];
        }

        switch (level) {
        case '0':
            optimize = O0;
            break;
        
        default:
        case '1':
            optimize = O1;
            break;
        case '2':
            optimize = O2;
            break;
        case '3':
            optimize = O3;
            break;
        }
    } else if (strcmp(arg, "--emit-ir") == 0) {
        emit_ir = true;
    } else if (strncmp(arg, "-o", 2) == 0) {
        const char *dst;
        if (strlen(arg) > 2) {
            dst = arg + 2;
        } else if (has_next) {
            dst = NEXT_ARG;
        } else {
            fail_fast("-o requires output file");
        }

        output = fopen(dst, "wb");
        if (!output) {
            fail_fast("failed to open output file");
        }
    } else {
        if (nodes) {
            return;
        }
        fprintf(stderr, "unknown arg %s\n", arg);
        exit(1);
    }
}

static void ir_emit_operand(operand_t op) {
    switch (op.type) {
    case IMM: printf("$%ld", op.num); break;
    case REG: printf("%%%zu", op.reg); break;
    case SYM: printf("`%zu`", op.reg); break;
    }
}

static void ir_emit_unary(const char *name, operand_t op) {
    printf("%s ", name);
    ir_emit_operand(op);
}

static void ir_debug_op(opcode_t *op) {
    printf("  %%%zu = ", op->dst);
    switch (op->type) {
    case OP_VALUE:
        ir_emit_operand(op->expr);
        break;

    case OP_NEG:
        ir_emit_unary("neg", op->expr);
        break;
    case OP_ABS:
        ir_emit_unary("abs", op->expr);
        break;

    default:
        printf("ir_debug_op(%d)\n", op->type);
        break;
    }
    printf("\n");
}

static void ir_debug(unit_t *unit) {
    for (size_t i = 0; i < unit->len; i++) 
        ir_debug_op(unit->ops + i);
}

int main(int argc, const char **argv) {
    output = stdout;

    for (int i = 1; i < argc; i++)
        parse_arg(i, argc, argv);

    if (!nodes) {
        fail_fast("input required");
    }

    for (size_t i = 0; i < nodes->len; i++) {
        unit_t unit = ir_emit_node(nodes->data + i);
        ir_debug(&unit);
    }

    return 0;
}
