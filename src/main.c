#include "bison.h"
#include "flex.h"

#include "ir.h"
#include "asm/x86.h"

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

static node_t *compile(const char *path, FILE *file) {
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

static node_t *compile_string(const char *text) {
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

unit_t *unit;

static void emit_imm(int64_t num) {
    printf("$%ld", num);
}

static void emit_operand(operand_t it) {
    if (it.type == REG) {
        printf("%%%zu", it.reg);
    } else {
        emit_imm(it.num);
    }
}

static void emit_opcode(size_t idx, opcode_t op) {
    if (op.op == OP_EMPTY)
        return;

    if (op.op == OP_RETURN) {
        printf("  ret ");
        emit_operand(op.expr);
        printf("\n");
        return;
    }

    printf("  %%%zu = ", idx);
    if (op.op == OP_DIGIT) {
        emit_imm(op.num);
    } else if (op.op == OP_NEG) {
        printf("neg ");
        emit_operand(op.expr);
    } else if (op.op == OP_ABS) { 
        printf("abs ");
        emit_operand(op.expr);
    } else {
        switch (op.op) {
        case OP_ADD: printf("add "); break;
        case OP_SUB: printf("sub "); break;
        case OP_DIV: printf("div "); break;
        case OP_MUL: printf("mul "); break;
        case OP_REM: printf("rem "); break;
        default: printf("err "); break;
        }
        emit_operand(op.lhs);
        printf(" ");
        emit_operand(op.rhs);
    }
    printf("\n");
}

static void debug_ir(void) {
    for (size_t i = 0; i < unit->length; i++) {
        opcode_t op = unit->ops[i];
        emit_opcode(i, op);
    }
}

enum {
    O0,
    O1,
    O2,
    O3
} optimize = O1;

node_t *node = NULL;

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
            node = compile_string(NEXT_ARG);
        }
    } else if (strcmp(arg, "-i") == 0) {
        printf(">>> ");
        node = compile("stdin", stdin);
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
    } else {
        if (node) {
            return;
        }
        fprintf(stderr, "unknown arg %s\n", arg);
        exit(1);
    }
}

int main(int argc, const char **argv) {
    for (int i = 1; i < argc; i++)
        parse_arg(i, argc, argv);

    if (!node) {
        fail_fast("input required");
    }

    unit = ir_gen(ast_return(node));

    debug_ir();

    if (optimize == O0) {
        /* no optimizing */
    } else if (optimize > O1) {
        size_t times = 0;
        bool dirty = true;

        while (dirty) {
            dirty = dirty && (ir_const_fold(unit) || ir_reduce(unit));

            printf("optimize: %zu\n", times++);
            debug_ir();
        }
    } else {
        ir_const_fold(unit);
        ir_reduce(unit);
    }

    emit_asm(unit);

    return 0;
}
