#include "ast.h"
#include "bison.h"
#include "flex.h"
#include "sema.h"

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

    state.inttype = new_builtin_type("int");
    state.voidtype = new_builtin_type("void");
    state.booltype = new_builtin_type("bool");

    nodes_t *types = empty_node_list();
    types = node_append(types, state.inttype);
    types = node_append(types, state.voidtype);
    types = node_append(types, state.booltype);

    state.decls = types;

    sema(&state, ast);

    if (state.errors > 0) {
        fprintf(stderr, "%d compilation error(s), aborting\n", state.errors);
        return 1;
    }

    dump_node(ast);
    printf("\n");

    return err;
}

int yyerror(YYLTYPE *loc, yyscan_t scan, node_t **node, const char *msg) {
    (void)scan;
    (void)node;
    fprintf(stderr, "error[%d:%d]: %s\n", loc->first_line, loc->first_column, msg);
    return 1;
}
