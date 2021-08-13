#include "ast.h"
#include "common.h"

#include "ctu/util/str.h"

static void debug_list(list_t *nodes) {
    for (size_t i = 0; i < list_len(nodes); i++) {
        if (i != 0) {
            printf(" ");
        }
        debug_ast(list_at(nodes, i));
    }
}

static void debug_digit(node_t *node) {
    printf("digit %s", mpz_get_str(NULL, 10, node->num));
}

static void debug_unary(node_t *node) {
    printf("unary %s ", unary_name(node->unary));
    debug_ast(node->expr);
}

static void debug_binary(node_t *node) {
    printf("binary %s ", binary_name(node->binary));
    debug_ast(node->lhs);
    printf(" ");
    debug_ast(node->rhs);
}

static void debug_return(node_t *node) {
    printf("return ");
    if (node->expr) {
        debug_ast(node->expr);
    } else {
        printf("void");
    }
}

static void debug_stmts(node_t *node) {
    size_t len = list_len(node->stmts);
    printf("stmts[%zu]", len);
    if (len) {
        printf(" ");
    }
    debug_list(node->stmts);
}

static void debug_call(node_t *node) {
    size_t len = list_len(node->args);
    printf("call[%zu] ", len);
    debug_ast(node->expr);
    
    if (len) {
        printf(" ");
    }

    debug_list(node->args);
}

static void debug_symbol(node_t *node) {
    printf("symbol ");
    for (size_t i = 0; i < list_len(node->ident); i++) {
        if (i != 0) {
            printf("::");
        }
        printf("%s", LIST_AT(char*, node->ident, i));
    }
}

static void debug_func(node_t *node) {
    printf("def %s ", node->name);
    debug_ast(node->body);
}

static void debug_branch(node_t *node) {
    printf("branch ");
    debug_ast(node->cond);
    printf(" ");
    debug_ast(node->branch);
}

static void debug_var(node_t *node) {
    printf("var %s ", node->name);
    if (node->init) {
        debug_ast(node->init);
        if (raw_type(node)) {
            printf(" ");
        }
    }
    if (raw_type(node)) {
        debug_ast(node->type);
    }
}

static void debug_assign(node_t *node) {
    printf("assign ");
    debug_ast(node->dst);
    printf(" ");
    debug_ast(node->src);
}

static void debug_struct(node_t *node) {
    printf("struct %s ", node->name);
    debug_list(node->fields);
}

static void debug_field(node_t *node) {
    printf("field %s ", node->name);
    debug_ast(node->type);
}

static void debug_access(node_t *node) {
    printf("access ");
    debug_ast(node->target);
    printf(" %s %s", node->field, node->indirect ? "indirect" : "direct");
}

void debug_ast(node_t *node) {
    printf("(");
    switch (node->kind) {
    case AST_ACCESS: debug_access(node); break;
    case AST_DIGIT: debug_digit(node); break;
    case AST_UNARY: debug_unary(node); break;
    case AST_BINARY: debug_binary(node); break;
    case AST_RETURN: debug_return(node); break;
    case AST_STMTS: debug_stmts(node); break;
    case AST_CALL: debug_call(node); break;
    case AST_SYMBOL: debug_symbol(node); break;
    case AST_DECL_FUNC: debug_func(node); break;
    case AST_BRANCH: debug_branch(node); break;
    case AST_DECL_VAR: debug_var(node); break;
    case AST_ASSIGN: debug_assign(node); break;
    case AST_DECL_STRUCT: debug_struct(node); break;
    case AST_DECL_FIELD: debug_field(node); break;
    default: printf("error %d", node->kind); break;
    }
    printf(")");
}

