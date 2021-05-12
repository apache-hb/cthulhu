#include "ast.h"
#include "bison.h"
#include "flex.h"
#include "sema.h"
#include "emit.h"
#include "scanner.h"

int main(int argc, const char **argv) {
    FILE* source;
    const char *path;
    int err = 0;

    if (argc > 1) {
        source = fopen(argv[1], "r");
        path = argv[1];
    } else {
        source = stdin;
        path = "stdin";
    }

    scan_extra_t extra = { NULL, path, NULL };
    
    err = yylex_init(&extra.scanner);

    if (err) {
        fprintf(stderr, "yylex_init failed %d %s\n", err, strerror(errno));
        return err;
    }    

    yyset_in(source, extra.scanner);
    yyset_extra(&extra, extra.scanner);

    err = yyparse(extra.scanner);

    if (err) {
        fprintf(stderr, "yyparse failed\n");
        return err;
    }

    yylex_destroy(extra.scanner);

    return 0;
}

/* for syntax errors */
int yyerror(YYLTYPE *yylloc, void *scanner, const char *msg) {
    scan_extra_t *extra = yyget_extra(scanner);

    fprintf(stderr,
        "error[%s:%d:%d]: %s\n", 
        extra->path,
        yylloc->first_line, 
        yylloc->first_column, 
        msg
    );
    return 1;
}
