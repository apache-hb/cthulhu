#pragma once

#include <stddef.h>
#include <stdarg.h>

#include "ctu/ast/ast.h"

typedef struct {
    char *message;
    const node_t *node;
} part_t;

typedef struct {
    level_t level;

    char *message;
    char *underline;

    vector_t *parts;

    const node_t *node;

    char *note;
} message_t;

typedef struct {
    size_t total;
    size_t used;

    message_t *messages;
} reports_t;

reports_t begin_report(size_t limit);
int end_report(reports_t *reports, const char *name);

message_t *assertf(reports_t *reports, const char *fmt, ...);
#define ASSERTF(ctx, expr, ...) ((!(expr)) ? assertf(reports, __VA_ARGS__)) : NULL

message_t *report();
message_t *reportf(const char *fmt, ...);
message_t *reportv(const char *fmt, va_list args);

void report_appendf(message_t *message, const char *fmt, ...);
void report_appendv(message_t *message, const char *fmt, va_list args);
void report_note(message_t *message, const char *fmt, ...);
