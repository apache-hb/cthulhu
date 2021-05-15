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

static operand_t imm(size_t val) {
    operand_t op = { IMM, val };
    return op;
}

static operand_t reg(size_t idx) {
    operand_t op = { REG, idx };
    return op;
}

static size_t add_digit(char *text) {
    size_t val = strtoull(text, NULL, 10);
    opcode_t op = { OP_DIGIT, { imm(val) } };
    return unit_add(unit, op);
}

static opcode_t op_binary(optype_t type, operand_t lhs, operand_t rhs) {
    opcode_t op = { type, { .lhs = lhs, .rhs = rhs } };
    return op;
}

static size_t add_binary(int it, size_t lhs, size_t rhs) {
    operand_t l = reg(lhs), r = reg(rhs);

    switch (it) {
    case ADD: return unit_add(unit, op_binary(OP_ADD, l, r));
    case SUB: return unit_add(unit, op_binary(OP_SUB, l, r));
    case DIV: return unit_add(unit, op_binary(OP_DIV, l, r));
    case MUL: return unit_add(unit, op_binary(OP_MUL, l, r));
    case REM: return unit_add(unit, op_binary(OP_REM, l, r));

    default:
        fprintf(stderr, "unknown op %d\n", it);
        return 0;
    }
}

static size_t emit_ir(node_t *node) {
    if (node->type == NODE_DIGIT) {
        return add_digit(node->text);
    } else {
        size_t lhs = emit_ir(node->binary.lhs);
        size_t rhs = emit_ir(node->binary.rhs);
        return add_binary(node->binary.op, lhs, rhs);
    }
}

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

static void debug_ir() {
    for (size_t i = 0; i < unit->length; i++) {
        opcode_t op = unit->ops[i];
        emit_opcode(i, op);
    }
}

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

    unit = new_unit();
    emit_ir(node);

    debug_ir();

    return 0;
}