#include "ast.h"
#include "bison.h"
#include "flex.h"
#include "sema.h"
#include "emit.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char **argv) {
    if (argc < 2) {
        fprintf(stderr, "%s: provide input file\n", argv[0]);
        return 1;
    }
    
    FILE* file = fopen(argv[1], "r");
    void *scanner;
    node_t *ast;

    yylex_init(&scanner);
    yyset_in(file, scanner);

    int err = yyparse(scanner, &ast);

    yylex_destroy(scanner);

    if (err) {
        return err;
    }

    dump_node(ast);
    printf("\n");

    state_t state;

    state.inttype = new_builtin_type("int", 4, S_SIGNED);
    state.voidtype = new_builtin_type("void", 0, S_SIGNED);
    state.booltype = new_builtin_type("bool", 0, S_SIGNED);
    state.chartype = new_builtin_type("char", 1, S_SIGNED);

    nodes_t *types = empty_node_list();
    types = node_append(types, state.inttype);
    types = node_append(types, state.voidtype);
    types = node_append(types, state.booltype);
    types = node_append(types, state.chartype);

    state.decls = types;

    sema(&state, ast);

    if (state.errors > 0) {
        fprintf(stderr, "%d compilation error(s), aborting\n", state.errors);
        return 1;
    }

    dump_node(ast);
    printf("\n");

    emit(ast);

    return err;
}

int yyerror(YYLTYPE *loc, yyscan_t scan, node_t **node, const char *msg) {
    (void)scan;
    (void)node;
    fprintf(stderr, "error[%d:%d]: %s\n", loc->first_line, loc->first_column, msg);
    return 1;
}
