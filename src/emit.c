#include "emit.h"

#include <stdio.h>

void emit_type(node_t *type)
{
    if (type->kind == NODE_BUILTIN_TYPE) {
        printf("%s", type->name);
    }
}

void emit_stmt(node_t *stmt)
{
    size_t i = 0;
    switch (stmt->kind) {
    case NODE_COMPOUND:
        printf("{");
        for (; i < stmt->compound->length; i++) {
            emit_stmt(stmt->compound->data + i);
            printf(";");
        }
        printf("}");
        break;
    case NODE_VAR:
        emit_type(stmt->decl.var.type);
        printf(" %s", stmt->decl.name);
        if (stmt->decl.var.init) {
            printf(" = ");
            emit_stmt(stmt->decl.var.init);
        }
        break;
    case NODE_BINARY:
        printf("(");
        emit_stmt(stmt->binary.lhs);
        switch (stmt->binary.op) {
        case BINARY_ADD: printf(" + "); break;
        case BINARY_SUB: printf(" - "); break;
        case BINARY_MUL: printf(" * "); break;
        case BINARY_DIV: printf(" / "); break;
        case BINARY_REM: printf(" %% "); break;
        default: printf("unknown binop\n"); break;
        }
        emit_stmt(stmt->binary.rhs);
        printf(")");
        break;
    case NODE_CALL:
        emit_stmt(stmt->call.body);
        printf("(");
        for (; i < stmt->call.args->length; i++) {
            if (i) {
                printf(", ");
            }
            emit_stmt(stmt->call.args->data + i);
        }
        printf(")");
        break;
    case NODE_RETURN:
        printf("return");
        if (stmt->expr) {
            printf(" ");
            emit_stmt(stmt->expr);
        }
        break;
    case NODE_DIGIT:
        printf("%s", stmt->digit);
        break;
    case NODE_NAME:
        printf("%s", stmt->name);
        break;
    default:
        printf("unknown node\n");
    }
}

void emit_param(node_t *decl)
{
    emit_type(decl->decl.param.type);
    printf(" %s", decl->decl.name);
}

void emit_func(node_t *func)
{
    emit_type(func->decl.func.result);
    printf(" %s(", func->decl.name);
    size_t len = func->decl.func.params->length;

    if (len == 0) {
        printf("void");
    } else {
        for (size_t i = 0; i < len; i++) {
            if (i) {
                printf(", ");
            }
            emit_param(func->decl.func.params->data + i);
        }
    }
    printf(")");

    printf("{");

    emit_stmt(func->decl.func.body);

    if (func->decl.func.body->kind != NODE_COMPOUND) {
        printf(";");
    }

    printf("}");
}

void emit_decl(node_t *decl)
{
    switch (decl->kind) {
    case NODE_FUNC: 
        emit_func(decl);
        break;
    default:
        printf("unknown decl to emit\n");
        break;
    }
    printf("\n");
}

void emit(node_t *prog)
{
    for (size_t i = 0; i < prog->compound->length; i++) {
        emit_decl(prog->compound->data + i);
    }
}
