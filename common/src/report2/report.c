#include "report2/report.h"

#include "std/vector.h"
#include "std/str.h"

#include "base/memory.h"
#include "base/panic.h"

typedef struct rep_root_t
{
    vector_t *entries;
} rep_root_t;

typedef struct rep_group_t
{
    rep_root_t *reports;
    rep_group_t *parent;

    const char *name;
    const char *desc;
} rep_group_t;

typedef struct rep_entry_t 
{
    rep_group_t *parent;

    rep_level_t level;
    const char *name;
    const char *desc;
} rep_entry_t;

typedef struct rep_message_t
{
    const rep_entry_t *entry;

    const node_t *node;
    char *message;
} rep_message_t;

static void push_entry(rep_root_t *reports, rep_entry_t *entry)
{
    vector_push(&reports->entries, entry);
}

rep_root_t *reports_new(void)
{
    rep_root_t *reports = ctu_malloc(sizeof(rep_root_t));

    reports->entries = vector_new(128);

    return reports;
}

rep_entry_t *add_entry(rep_group_t *group, rep_level_t level, const char *name, const char *desc)
{   
    CTASSERT(group != NULL);
    CTASSERT(name != NULL);
    CTASSERT(desc != NULL);

    rep_entry_t *entry = ctu_malloc(sizeof(rep_entry_t));

    entry->parent = group;
    entry->level = level;
    entry->name = name;
    entry->desc = desc;

    push_entry(group->reports, entry);

    return entry;
}

rep_message_t *reportv(const rep_entry_t *entry, const node_t *node, const char *fmt, va_list args)
{
    CTASSERT(entry != NULL);
    CTASSERT(fmt != NULL);

    rep_message_t *message = ctu_malloc(sizeof(rep_message_t));
    message->entry = entry;
    message->node = node;
    message->message = formatv(fmt, args);

    return message;
}

rep_message_t *report(const rep_entry_t *entry, const node_t *node, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    rep_message_t *message = reportv(entry, node, fmt, args);
    va_end(args);

    return message;
}
