#include "bison.h"
#include "flex.h"

#include <stdbool.h>

#include "ir.h"
#include "asm/x64.h"
#include "asm/aarch64.h"

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

const char *prog;

static void fail_fast(const char *message) {
    fprintf(stderr, "error: %s\n", message);
    fprintf(stderr, "try `%s --help`\n", prog);
    exit(1);
}

static void print_help() {
    printf("%s: cthulhu compiler\n", prog);

    printf("\t-h,--help: print this message\n");
    printf("\t-e: compile expression from string `-e \"5 ? 10 : 20 + 30;\"\n");
    printf("\t-O: set optimization level `-O0`, `-O1`, `-O2`, `-O3`\n");
    printf("\t-o: set output file (defaults to stdout) `-o test.asm`\n");
    printf("\t-i: run in interactive mode\n");
    printf("\t-E,--emit-ir: emit ir bytecode\n");

    exit(0);
}

#define NEXT_ARG (argv[idx + 1])

static void parse_arg(int idx, int argc, const char **argv) {
    const char *arg = argv[idx];
    bool has_next = argc - 1 > idx;

    if (strcmp(arg, "-e") == 0) {
        if (!has_next) {
            fail_fast("`-e` argument requires an expression");
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
    } else if (strcmp(arg, "-E") == 0 || strcmp(arg, "--emit-ir") == 0) {
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
    } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
        print_help();
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
    }
}

static void ir_emit_unary(const char *name, operand_t op) {
    printf("%s ", name);
    ir_emit_operand(op);
}

static void ir_emit_jmp(operand_t cond, operand_t dst) {
    printf("  jmp ");
    ir_emit_operand(dst);
    printf(" when ");
    ir_emit_operand(cond);
    printf("\n");
}

static void ir_emit_phi(operand_t lhs, operand_t rhs) {
    printf("phi [");
    ir_emit_operand(lhs);
    printf(", ");
    ir_emit_operand(rhs);
    printf("]");
}

static void ir_emit_binary(const char *op, operand_t lhs, operand_t rhs) {
    printf("%s ", op);
    ir_emit_operand(lhs);
    printf(" ");
    ir_emit_operand(rhs);
}

static void ir_emit_ret(operand_t op) {
    printf("  ret ");
    ir_emit_operand(op);
    printf("\n");
}

static void ir_debug_op(size_t idx, opcode_t *op) {
    switch (op->type) {
    case OP_JMP: ir_emit_jmp(op->cond, op->label); return;
    case OP_LABEL: printf(".%zu:\n", idx); return;
    case OP_RETURN: ir_emit_ret(op->expr); return;
    default: break;
    }

    printf("  %%%zu = ", idx);
    switch (op->type) {
    case OP_VALUE: ir_emit_operand(op->expr); break;
    case OP_NEG: ir_emit_unary("neg", op->expr); break;
    case OP_ABS: ir_emit_unary("abs", op->expr); break;
    case OP_PHI: ir_emit_phi(op->lhs, op->rhs); break;

    case OP_ADD: ir_emit_binary("add", op->lhs, op->rhs); break;
    case OP_SUB: ir_emit_binary("sub", op->lhs, op->rhs); break;
    case OP_DIV: ir_emit_binary("div", op->lhs, op->rhs); break;
    case OP_MUL: ir_emit_binary("mul", op->lhs, op->rhs); break;
    case OP_REM: ir_emit_binary("rem", op->lhs, op->rhs); break;

    default:
        printf("ir_debug_op(%d)\n", op->type);
        break;
    }
    printf("\n");
}

static void ir_debug(unit_t *unit, size_t idx) {
    printf("define %zu {\n", idx);
    for (size_t i = 0; i < unit->len; i++) {
        ir_debug_op(i, unit->data + i);
    }
    printf("}\n");
}

int main(int argc, const char **argv) {
    output = stdout;
    prog = argv[0];

    for (int i = 1; i < argc; i++)
        parse_arg(i, argc, argv);

    if (!nodes) {
        fail_fast("input required");
    }

    for (size_t i = 0; i < nodes->len; i++) {
        if (i) {
            printf("\n");
        }
        unit_t unit = ir_emit_node(ast_return(nodes->data + i));
        ir_debug(&unit, i);

        x64_emit_asm(&unit, output);
    }

    return 0;
}
