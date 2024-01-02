#pragma once

#include "cpp/ast.h"
#include "scan/node.h" // IWYU pragma: export
#include "std/vector.h"

#define CPPLTYPE where_t

typedef struct cpp_instance_t cpp_instance_t;
typedef struct typevec_t typevec_t;
typedef struct scan_t scan_t;
typedef struct map_t map_t;
typedef struct vector_t vector_t;

typedef struct cpp_file_t
{
    // cthulhu runtime
    scan_t *scan; // the associated scanner
    where_t where; // the current location

    // flex/bison state
    void *buffer; // lex buffer

    // C language
    bool pragma_once; // did this file have a #pragma once?
} cpp_file_t;

/// shared between all scanners
typedef struct cpp_scan_t
{
    cpp_instance_t *instance;

    /// @brief the preprocessing result
    typevec_t *result;

    /// @brief are we currently skipping a preprocessor group?
    bool skipping;

    /// how deep are we in a chain of #if/#elif/#else/#endif?
    size_t branch_depth;

    map_t *defines;
    map_t *files;

    /// the include stack
    size_t stack_index;
    size_t stack_size;
    cpp_file_t **stack;

    cpp_file_t *current_file;
} cpp_scan_t;

cpp_scan_t cpp_scan_new(cpp_instance_t *instance);

cpp_scan_t *cpp_scan_context(scan_t *scan);

void cpp_scan_consume(scan_t *scan, const char *text, size_t size, bool is_comment);

cpp_file_t *cpp_file_from_io(arena_t *arena, void *yyscanner, io_t *io);
cpp_file_t *cpp_file_from_scan(scan_t *scan, void *yyscanner);

void cpp_set_current_file(cpp_scan_t *extra, cpp_file_t *file);

// get an include, either "" or <>
void cpp_accept_include(void *yyscanner, const char *text);

// get an include that requires preprocessing the provided name
void cpp_accept_define_include(void *yyscanner, const char *text);

bool cpp_leave_file(void *yyscanner);

node_t *cpp_get_node(void *yyscanner, where_t where);

void cpp_add_define(scan_t *scan, where_t where, const char *name, vector_t *body);
void cpp_add_macro(scan_t *scan, where_t where, const char *name, cpp_params_t params, vector_t *body);

cpp_params_t make_params(vector_t *names, bool variadic);

void cpp_remove_define(scan_t *scan, where_t where, const char *name);

cpp_ast_t *cpp_get_define(scan_t *scan, const char *name);

bool is_skipping(scan_t *scan);

void enter_ifdef(scan_t *scan, where_t where, const char *name);
void enter_ifndef(scan_t *scan, where_t where, const char *name);

void enter_branch(scan_t *scan, where_t where, vector_t *condition);
void elif_branch(scan_t *scan, where_t where, vector_t *condition);
void else_branch(scan_t *scan, where_t where);
void leave_branch(scan_t *scan, where_t where);

bool eval_condition(scan_t *scan, const node_t *node, vector_t *condition);
vector_t *expand_macro(scan_t *scan, const char *name, vector_t *args);

cpp_number_t make_number(scan_t *scan, const char *text, size_t len, int base);

void cpp_accept_pragma(scan_t *scan, where_t where, vector_t *tokens);
void cpp_expand_ident(scan_t *scan, where_t where, const char *name, size_t size);
void cpp_expand_macro(scan_t *scan, where_t where, const char *name, size_t size, vector_t *args);
