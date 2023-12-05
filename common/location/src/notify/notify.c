#include "notify/notify.h"

#include "core/macros.h"

typedef struct reports_t
{
    alloc_t *alloc;
} reports_t;

reports_t *msg_new(alloc_t *alloc)
{
    reports_t *reports = arena_malloc(alloc, sizeof(reports_t), "reports");
    reports->alloc = alloc;
    return reports;
}

void msg_delete(reports_t *reports)
{
    CTU_UNUSED(reports);
    // empty for now
}

int msg_flush(reports_t *reports)
{
    return 0;
}

message_t *msg_diagnostic(reports_t *reports, const diagnostic_t *diagnostic)
{
    return NULL;
}

message_t *msg_notify(reports_t *reports, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, ...)
{
    return NULL;
}

message_t *msg_vnotify(reports_t *reports, const diagnostic_t *diagnostic, const node_t *node, const char *fmt, va_list args)
{
    return NULL;
}

void msg_append(message_t *message, const node_t *node, const char *fmt, ...)
{

}

void msg_underline(message_t *message, const node_t *node, const char *fmt, ...)
{

}

void msg_note(message_t *message, const char *fmt, ...)
{

}
