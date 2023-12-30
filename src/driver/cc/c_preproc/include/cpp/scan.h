#pragma once

#include "scan/node.h" // IWYU pragma: export

typedef struct cpp_instance_t cpp_instance_t;
typedef struct typevec_t typevec_t;
typedef struct scan_t scan_t;
typedef struct map_t map_t;

#define CPPLTYPE where_t

typedef struct cpp_file_t
{
    void *buffer; // lex buffer
    bool pragma_once; // did this file have a #pragma once?
    char *path; // the path of the file
} cpp_file_t;

/// shared between all scanners
typedef struct cpp_scan_t
{
    cpp_instance_t *instance;

    /// @brief the preprocessing result
    typevec_t *result;

    map_t *defines;
    map_t *files;

    /// @brief are we currently inside a directive?
    bool inside_directive;

    /// the include stack
    size_t stack_index;
    size_t stack_size;
    cpp_file_t **stack;

    cpp_file_t *current_file;
} cpp_scan_t;

cpp_scan_t cpp_scan_new(cpp_instance_t *instance);

cpp_scan_t *cpp_scan_context(scan_t *scan);

void cpp_enter_directive(scan_t *scan);
void cpp_leave_directive(scan_t *scan);

void cpp_scan_consume(scan_t *scan, const char *text, size_t size);

bool cpp_check_recursion(scan_t *scan, const char *text);

cpp_file_t *cpp_file_from_io(arena_t *arena, void *yyscanner, io_t *io);
cpp_file_t *cpp_file_from_scan(scan_t *scan, void *yyscanner);

// get an include, either "" or <>
cpp_file_t *cpp_accept_include(void *yyscanner, const char *text, size_t size);

// get an include that requires preprocessing the provided name
cpp_file_t *cpp_accept_define_include(void *yyscanner, const char *text);

bool cpp_leave_file(void *yyscanner);
