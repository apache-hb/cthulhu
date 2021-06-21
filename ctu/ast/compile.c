#include "compile.h"

#include "ctu/util/report.h"
#include "ctu/util/str.h"

#include "scanner.h"
#include "bison.h"
#include "flex.h"

static int flex_file_next(void *handle) {
    FILE *file = handle;
    int letter = fgetc(file);
    return letter == EOF ? 0 : letter;
}

static scanner_t *new_scanner(const char *path, void *handle, int(*next)(void*)) {
    scanner_t *scanner = malloc(sizeof(scanner_t));

    scanner->path = path;
    scanner->handle = handle;
    scanner->next = next;

    scanner->text = malloc(0x1000);
    scanner->len = 0;
    scanner->size = 0x1000;

    return scanner;
}

static const char *yyerror_str(int err) {
    switch (err) {
    case ENOMEM: return "out of memory";
    case EINVAL: return "invalid argument";
    default: return format("error %d", err);
    }
}

nodes_t *compile_file(const char *path, FILE *stream) {
    int err;
    yyscan_t scanner;
    scanner_t *extra = new_scanner(path, stream, flex_file_next);

    if ((err = yylex_init_extra(extra, &scanner))) {
        assert("yylex_init_extra -> %s", yyerror_str(err));
        return NULL;
    }

    yyset_in(stream, scanner);
    
    if ((err = yyparse(scanner, extra))) {
        return NULL;
    }

    yylex_destroy(scanner);

    return extra->ast;
}

nodes_t *compile_string(const char *path, const char *text) {
    int err;
    yyscan_t scanner;
    scanner_t *extra = new_scanner(path, NULL, NULL);
    extra->text = strdup(text);
    extra->len = strlen(text);
    YY_BUFFER_STATE buffer;

    if ((err = yylex_init_extra(extra, &scanner))) {
        assert("yylex_init_extra -> %s", yyerror_str(err));
        return NULL;
    }

    if (!(buffer = yy_scan_string(text, scanner))) {
        assert("yy_scan_string -> NULL");
        return NULL;
    }

    if ((err = yyparse(scanner, extra))) {
        yy_delete_buffer(buffer, scanner);
        return NULL;
    }

    yylex_destroy(scanner);

    return extra->ast;
}

void yyerror(where_t *where, void *state, scanner_t *scanner, const char *msg) {
    (void)state;
    
    report(LEVEL_ERROR, scanner, *where, msg);
}
