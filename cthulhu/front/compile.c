#include "compile.h"

#include "bison.h"
#include "flex.h"

#include "cthulhu/report/report.h"

nodes_t *compile_file(const char *path, FILE *stream) {
    int err;
    yyscan_t scan;
    scanner_t extra = { path, NULL };

    if ((err = yylex_init_extra(&extra, &scan))) {
        reportf("yylex_init_extra -> %d", err);
        return NULL;
    }

    yyset_in(stream, scan);

    if ((err = yyparse(scan, &extra))) {
        reportf("yyparse -> %d", err);
        return NULL;
    }

    yylex_destroy(scan);

    return extra.ast;
}

nodes_t *compile_string(const char *path, const char *text) {
    int err;
    yyscan_t scan;
    scanner_t extra = { path, NULL };
    YY_BUFFER_STATE buffer;

    if ((err = yylex_init_extra(&extra, &scan))) {
        reportf("yylex_init_extra -> %d", err);
        return NULL;
    }

    if (!(buffer = yy_scan_string(text, scan))) {
        reportf("yy_scan_string -> NULL");
        return NULL;
    }

    if ((err = yyparse(scan, &extra))) {
        reportf("yyparse -> %d", err);
        yy_delete_buffer(buffer, scan);
        return NULL;
    }

    yylex_destroy(scan);

    return extra.ast;
}

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
