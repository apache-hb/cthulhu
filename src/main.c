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

#if 0
static void assemble(void) {
    printf("global _start\nsection .text\n  _start:\n");
}
#endif

int main(int argc, const char **argv) {
    FILE *in;
    const char *path;

    if (argc > 1) {
        path = argv[1];
        in = fopen(path, "r");
    } else {
        path = "stdin";
        in = stdin;
        printf(">>> ");
    }

    node_t *node = compile(path, in);
    if (!node) {
        return 1;
    }

    unit = ir_gen(node);

    debug_ir();

    size_t times = 0;
    bool dirty = true;

    while (dirty) {
        dirty = dirty && (ir_const_fold(unit) || ir_reduce(unit));

        printf("optimize: %zu\n", times++);
        debug_ir();
    }

    live_graph_t graph = build_graph(unit);

    for (size_t i = 0; i < graph.len; i++) {
        if (i) {
            printf("\n");
        }
        live_range_t range = graph.ranges[i];
        printf("%zu -> %zu", range.first, range.last);
    }
    printf("\n");

#if 0
    printf(
        "\n"
        "global _start\n"
        "section .text\n\n"
        "_start:\n"
        "  mov rax, 60\n"
        "  xor edi, edi\n"
        "  syscall\n"
    );

    assemble();
#endif

    return 0;
}

#if 0
5 ? 7 : 8;

  %0 = 5
if %0 goto 2
  %1 = 7
  jmp 3
2: 
  %1 = 8
3: 
  %3 = %1
#endif 