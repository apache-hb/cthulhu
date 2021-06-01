#include "bison.h"
#include "flex.h"

#include "ir.h"

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

static void emit_operand(operand_t it) {
    if (it.type == REG) {
        printf("%%%zu", it.val);
    } else {
        printf("$%zu", it.val);
    }
} 

static void emit_opcode(size_t idx, opcode_t op) {
    printf("%%%zu = ", idx);
    if (op.op == OP_DIGIT) {
        emit_operand(op.lhs);
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