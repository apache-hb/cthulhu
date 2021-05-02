#include "ast.h"

#include <stdlib.h>
#include <stdio.h>

static node_t*
new_node(node_kind_t kind) 
{
    node_t *node = malloc(sizeof(node_t));
    node->kind = kind;
    return node;
}

node_t*
new_digit(char* digit) 
{
    node_t *node = new_node(NODE_DIGIT);
    node->digit = digit;
    return node;
}

node_t*
new_unary(unary_op_t op, node_t *expr) 
{
    node_t *node = new_node(NODE_UNARY);
    node->unary.op = op;
    node->unary.expr = expr;
    return node;
}

node_t*
new_binary(binary_op_t op, node_t *lhs, node_t *rhs) 
{
    node_t *node = new_node(NODE_BINARY);
    node->binary.op = op;
    node->binary.lhs = lhs;
    node->binary.rhs = rhs;
    return node;
}

node_t*
new_ternary(node_t *cond, node_t *yes, node_t *no) 
{
    node_t *node = new_node(NODE_TERNARY);
    node->ternary.cond = cond;
    node->ternary.yes = yes;
    node->ternary.no = no;
    return node;
}

void
dump_node(node_t *node)
{
    switch (node->kind) {
    case NODE_DIGIT: 
        printf("%s", node->digit); 
        break;
    case NODE_UNARY:    
        printf("("); 
        switch (node->unary.op) {
        case UNARY_ABS: printf("abs "); break;
        case UNARY_NEG: printf("neg "); break;
        } 
        dump_node(node->unary.expr);
        printf(")");
        break;
    case NODE_BINARY: 
        printf("(");
        switch (node->binary.op) {
        case BINARY_ADD: printf("add "); break;
        case BINARY_SUB: printf("sub "); break;
        case BINARY_DIV: printf("div "); break;
        case BINARY_MUL: printf("mul "); break;
        case BINARY_REM: printf("rem "); break;
        }
        dump_node(node->binary.lhs);
        printf(" ");
        dump_node(node->binary.rhs);
        printf(")");
        break;
    case NODE_TERNARY:
        printf("(if ");
        dump_node(node->ternary.cond);
        printf(" then ");
        dump_node(node->ternary.yes);
        printf(" else ");
        dump_node(node->ternary.no);
        printf(")");
        break;
    }
}
