#pragma once

#include <gmp.h>

#include "cpp/cpp.h"
#include "cpp/ast.h"
#include "scan/node.h" // IWYU pragma: export

#define YY_INPUT(buffer, result, size)         \
    result = cpp_input(yyextra, buffer, size); \
    if ((result) <= 0)                         \
    {                                          \
        (result) = YY_NULL;                    \
    }

// include this after defining YY_INPUT so we override it
#include "interop/flex.h" // IWYU pragma: keep

typedef struct synthetic_token_t synthetic_token_t;
typedef struct typevec_t typevec_t;
typedef struct fifo_t fifo_t;
typedef struct set_t set_t;

#define CPPLTYPE where_t

typedef struct cpp_file_t
{
    void *buffer;
    scan_t *scan;
    where_t where;
} cpp_file_t;

typedef struct cpp_define_t
{
    const node_t *node;
    vector_t *body;
} cpp_define_t;

typedef struct cpp_number_t
{
    text_t text;
    mpz_t value;
} cpp_number_t;

typedef struct cpp_extra_t
{
    cpp_config_t config;

    void *yyscanner;
    void *yypstate;

    typevec_t *result;

    typevec_t *comment;

    fifo_t *tokens;

    // how many branches deep we are
    size_t branch_depth;

    // the index of the branch that disabled output
    // SIZE_MAX if output is enabled
    size_t branch_disable_index;

    bool inside_directive;

    map_t *defines;
    map_t *include_cache;

    // keep a set of files that dont exist so we dont keep querying the filesystem
    set_t *nonexistent_files;

    size_t include_depth;
    size_t max_include_depth;
    cpp_file_t **include_stack;

    cpp_file_t *current_file;
} cpp_extra_t;

///
/// internal api
///

int cpp_extra_init(cpp_config_t config, cpp_extra_t *extra);
int cpp_parse(cpp_extra_t *extra);

///
/// scanner api
///

void cpp_push_output_single(cpp_extra_t *extra, char c);
void cpp_push_output(cpp_extra_t *extra, text_t text);

void cpp_push_ident(cpp_extra_t *extra, text_t text);

cpp_ast_t *cpp_expand_ident(cpp_extra_t *extra, where_t where, text_t text);
cpp_ast_t *cpp_expand_macro(cpp_extra_t *extra, where_t where, text_t text, vector_t *args);

void cpp_push_comment(cpp_extra_t *extra, const char *text, size_t size);
text_t cpp_reset_comment(cpp_extra_t *extra);

text_t cpp_text_new(cpp_extra_t *extra, const char *text, size_t size);

void cpp_enter_directive(cpp_extra_t *extra);
void cpp_leave_directive(cpp_extra_t *extra);

synthetic_token_t *cpp_token_new(cpp_extra_t *extra, int token, void *value, where_t where);

cpp_number_t *cpp_number_new(cpp_extra_t *extra, const char *text, size_t len, int base);

///
/// defines
///

cpp_define_t *cpp_define_new(cpp_extra_t *extra, where_t where, vector_t *body);

void cpp_add_define(cpp_extra_t *extra, where_t where, text_t name, vector_t *body);
void cpp_remove_define(cpp_extra_t *extra, where_t where, text_t name);

void cpp_ifdef(cpp_extra_t *extra, where_t where, text_t name);
void cpp_ifndef(cpp_extra_t *extra, where_t where, text_t name);

void cpp_if(cpp_extra_t *extra, where_t where, cpp_ast_t *ast);
void cpp_elif(cpp_extra_t *extra, where_t where, cpp_ast_t *ast);

void cpp_else(cpp_extra_t *extra, where_t where);
void cpp_endif(cpp_extra_t *extra, where_t where);

///
/// getters
///

cpp_extra_t *cpp_get_scanner_extra(void *yyscanner);

arena_t *cpp_get_scanner_arena(void *yyscanner);
logger_t *cpp_get_scanner_logger(void *yyscanner);


///
/// include handling
///

cpp_file_t *cpp_file_from_scan(cpp_extra_t *extra, scan_t *scan);
cpp_file_t *cpp_file_from_io(cpp_extra_t *extra, io_t *io);

void cpp_set_current_file(cpp_extra_t *extra, cpp_file_t *file);

void cpp_include_local(cpp_extra_t *extra, const char *path, size_t length);
void cpp_include_system(cpp_extra_t *extra, const char *path, size_t length);
void cpp_include_define(cpp_extra_t *extra, const char *name, size_t length);

// returns true if theres a file above this one
bool cpp_leave_file(cpp_extra_t *extra);

int cpp_input(cpp_extra_t *extra, char *out, int size);
