#include "ast.h"
#include "bison.h"
#include "flex.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char **argv) {
    if (argc < 2) {
        fprintf(stderr, "%s: provide input file\n", argv[0]);
        return 1;
    }
    
    FILE* file = stdin; //fopen(argv[1], "r");
    void *scanner;

    yylex_init(&scanner);
    yyset_in(file, scanner);

    int err = yyparse(scanner);

    yylex_destroy(scanner);

    return err;
}

int yyerror(yyscan_t scan, const char *msg) {
    (void)scan;
    fprintf(stderr, "error: %s\n", msg);
    return 1;
}
