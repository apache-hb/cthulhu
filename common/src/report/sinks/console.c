#include "report/sink.h"

#include "base/util.h"

typedef struct
{
    bool colour;
    vector_t *reports;
} console_sink_t;

static sink_t *sink_new(void)
{
    return ctu_malloc(sizeof(sink_t) + sizeof(console_sink_t));
}

static bool console_begin(void *user)
{
    return true;
}

static void console_end(void *user, const char *name, const report_config_t *config)
{

}

static void console_add(void *user, const message_t *message)
{

}

sink_t *sink_console(bool colour)
{
    sink_t *sink = sink_new();

    sink->begin = console_begin;
    sink->end = console_end;
    sink->add = console_add;

    console_sink_t *console = (void*)sink->user;

    console->colour = colour;
    console->reports = vector_new(32);

    return sink;
}
