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

    dump_node(ast);
    printf("\n");

    sema(ast);

    return err;
}

int yyerror(yyscan_t scan, const char *msg) {
    (void)scan;
    fprintf(stderr, "error: %s\n", msg);
    return 1;
}
