#pragma once

#include <stdbool.h>
#include <stdarg.h>

typedef struct node_t node_t;
typedef struct vector_t vector_t;

typedef struct rep_root_t rep_root_t;
typedef struct rep_message_t rep_message_t;
typedef struct rep_group_t rep_group_t;
typedef struct rep_entry_t rep_entry_t;

typedef enum rep_level_t {
#define REPORT_LEVEL(ID, STR, COLOUR) ID,
#include "report.inc"
    eLevelTotal
} rep_level_t;

rep_root_t *reports_new(void);

rep_group_t *add_group(rep_root_t *reports, const char *name, const char *desc);
rep_group_t *add_subgroup(rep_group_t *group, const char *name, const char *desc);
rep_entry_t *add_entry(rep_group_t *group, rep_level_t level, const char *name, const char *desc);

rep_message_t *reportv(const rep_entry_t *entry, const node_t *node, const char *fmt, va_list args);
rep_message_t *report(const rep_entry_t *entry, const node_t *node, const char *fmt, ...);

void logverbose(const char *fmt, ...);
