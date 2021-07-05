#include "compile.h"

#include "ctu/util/report.h"
#include "ctu/util/str.h"

#include "scanner.h"
#include "bison.h"
#include "flex.h"

static size_t file_size(FILE *fd) {
    fseek(fd, 0, SEEK_END);
    size_t size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    return size;
}

static scanner_t *new_scanner(const char *path, size_t size) {
    scanner_t *scanner = malloc(sizeof(scanner_t));

    scanner->path = path;

    scanner->text = malloc(size + 1);
    scanner->offset = 0;
    scanner->size = size;

    return scanner;
}

static const char *yyerror_str(int err) {
    switch (err) {
    case ENOMEM: return "out of memory";
    case EINVAL: return "invalid argument";
    default: return format("error %d", err);
    }
}

nodes_t *compile_file(const char *path, FILE *stream, scanner_t **scanout) {
    int err;
    yyscan_t scanner;
    size_t size = file_size(stream);
    scanner_t *extra = new_scanner(path, file_size(stream));
    fread(extra->text, 1, size, stream);

    if ((err = yylex_init_extra(extra, &scanner))) {
        assert("yylex_init_extra -> %s", yyerror_str(err));
        return NULL;
    }

    yyset_in(stream, scanner);
    
    if ((err = yyparse(scanner, extra))) {
        return NULL;
    }

    yylex_destroy(scanner);

    if (scanout) {
        *scanout = extra;
    }

    return extra->ast;
}

nodes_t *compile_string(const char *path, const char *text, scanner_t **scanout) {
    int err;
    yyscan_t scanner;
    scanner_t *extra = new_scanner(path, strlen(text));
    extra->text = strdup(text);
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

    if (scanout) {
        *scanout = extra;
    }

    return extra->ast;
}

void yyerror(where_t *where, void *state, scanner_t *scanner, const char *msg) {
    (void)state;
    
    report(LEVEL_ERROR, scanner, *where, msg);
}
