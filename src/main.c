#include "bison.h"
#include "flex.h"

#include "ir.h"
#include "sema.h"
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

static void emit_imm(int64_t num) {
    printf("$%ld", num);
}

static void emit_operand(operand_t it) {
    switch (it.type) {
    case REG:
        printf("%%%zu", it.reg);
        break;
    case IMM:
        printf("$%ld", it.num);
        break;
    case SYM:
        printf("`%s`", it.name);
        break;
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
    } else if (op.op == OP_NAME) {
        emit_operand(op.expr);
    } else if (op.op == OP_NEG) {
        printf("neg ");
        emit_operand(op.expr);
    } else if (op.op == OP_ABS) { 
        printf("abs ");
        emit_operand(op.expr);
    } else if (op.op == OP_CALL) {
        printf("call ");
        emit_operand(op.body);
        printf(" (");
        for (size_t i = 0; i < op.total; i++) {
            if (i) {
                printf(", ");
            }
            emit_operand(op.args[i]);
        }
        printf(")");
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

static void debug_ir(unit_t *unit) {
    printf("%s\n", unit->name);
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

static void optimize_unit(unit_t *unit, units_t *world) {
    if (optimize == O0) {
        /* no optimizing */
    } else if (optimize > O1) {
        bool dirty = true;

        while (dirty) {
            dirty = dirty && (ir_const_fold(unit) || ir_reduce(unit) || ir_inline(unit, world));
        }
    } else {
        ir_const_fold(unit);
        ir_reduce(unit);
    }

    if (emit_ir) {
        debug_ir(unit);
    }
}

int main(int argc, const char **argv) {
    output = stdout;

    for (int i = 1; i < argc; i++)
        parse_arg(i, argc, argv);

    if (!nodes) {
        fail_fast("input required");
    }

    sym_resolve(nodes);

    units_t world = symbol_table();

    for (size_t i = 0; i < nodes->len; i++) {
        unit_t *ir = ir_gen(ast_return(nodes->data + i), "main");
        add_symbol(&world, ir);
    }

    if (emit_ir) {
        for (size_t i = 0; i < world.len; i++)
            debug_ir(world.units[i]);
    }

    for (size_t i = 0; i < world.len; i++)
        optimize_unit(world.units[i], &world);

    for (size_t i = 0; i < world.len; i++)
        emit_asm(world.units[i], output);

    return 0;
}
