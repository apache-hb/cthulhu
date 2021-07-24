/* we need to conditionally set this and mmap for windows */
#define _POSIX_SOURCE

#include "compile.h"

#ifndef _WIN32
#   include <sys/mman.h>
#endif

#include <stdio.h>

#include "ctu/util/report.h"
#include "ctu/util/str.h"
#include "ctu/util/util.h"

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
    scanner_t *scanner = ctu_malloc(sizeof(scanner_t));

    scanner->path = path;
    scanner->offset = 0;
    scanner->size = size;

    return scanner;
}

static scanner_t *new_str_scanner(const char *path, const char *str) {
    scanner_t *scanner = new_scanner(path, strlen(str));
    scanner->text = str;
    return scanner;
}

static scanner_t *new_file_scanner(const char *path, FILE *file) {
    size_t size = file_size(file);
    scanner_t *scanner = new_scanner(path, size);

#ifndef _WIN32
    int fd = fileno(file);
    scanner->text = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
#else
    char *text = ctu_malloc(size + 1);
    fread(text, 1, size, file);
    text[size] = '\0';
    scanner->text = text;
#endif

    return scanner;
}

static const char *yyerror_str(int err) {
    switch (err) {
    case ENOMEM: return "out of memory";
    case EINVAL: return "invalid argument";
    default: return format("error %d", err);
    }
}

node_t *compile_file(const char *path, FILE *stream, scanner_t **scanout) {
    int err;
    yyscan_t scanner;
    scanner_t *extra = new_file_scanner(path, stream);

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

node_t *compile_string(const char *path, const char *text, scanner_t **scanout) {
    int err;
    yyscan_t scanner;
    scanner_t *extra = new_str_scanner(path, text);
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
