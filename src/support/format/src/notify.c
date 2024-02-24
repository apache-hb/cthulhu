// SPDX-License-Identifier: LGPL-3.0-only

#include "common_extra.h"

#include "format/notify2.h"

#include "io/io.h"

#include "std/typed/vector.h"
#include "std/vector.h"

#include "base/panic.h"

typedef struct notify_config_t
{
    print_notify_t config;
} notify_config_t;

// TODO: all this

#define COLOUR_PATH eColourBlue

static void print_notify_segment_simple(notify_config_t *config, const segment_t *segment)
{
    print_notify_t options = config->config;
    print_options_t base = options.options;

    format_context_t format_context = format_context_make(base);

    source_config_t source_config = {
        .context = format_context,
        .colour = COLOUR_PATH,
        .heading_style = options.heading_style,
        .zero_indexed_lines = options.zero_indexed_lines,
    };

    char *path = fmt_node_location(source_config, segment->node);

    io_printf(base.io, "%s: %s\n", path, segment->message);
}

static void print_notify_note_simple(notify_config_t *config, const char *path, const char *note)
{
    print_notify_t options = config->config;
    print_options_t base = options.options;

    io_printf(base.io, "%s note: %s\n", path, note);
}

static void print_notify_simple(notify_config_t *config, const event_t *event)
{
    print_notify_t options = config->config;
    print_options_t base = options.options;

    format_context_t format_context = format_context_make(base);

    source_config_t source_config = {
        .context = format_context,
        .colour = COLOUR_PATH,
        .heading_style = options.heading_style,
        .zero_indexed_lines = options.zero_indexed_lines,
    };

    const diagnostic_t *diagnostic = event->diagnostic;
    severity_t severity = diagnostic->severity;
    const char *id = diagnostic->id;

    const char *severity_name = get_severity_name(severity);
    colour_t severity_colour = get_severity_colour(severity);

    char *path = fmt_node_location(source_config, event->node);

    char *level = colour_format(format_context, severity_colour, "%s %s:", severity_name, id);

    io_printf(base.io, "%s: %s %s\n", path, level, event->message);

    if (event->segments != NULL)
    {
        size_t len = typevec_len(event->segments);
        for (size_t i = 0; i < len; i++)
        {
            const segment_t *segment = typevec_offset(event->segments, i);
            print_notify_segment_simple(config, segment);
        }
    }

    if (event->notes != NULL)
    {
        size_t len = vector_len(event->notes);
        for (size_t i = 0; i < len; i++)
        {
            const char *note = vector_get(event->notes, i);
            print_notify_note_simple(config, path, note);
        }
    }
}

USE_DECL
void print_notify(print_notify_t config, const event_t *event)
{
    CTASSERT(event != NULL);

    notify_config_t notify_config = {
        .config = config,
    };

    print_notify_simple(&notify_config, event);
}

USE_DECL
void print_notify_many(print_notify_t config, const typevec_t *events)
{
    CTASSERT(events != NULL);

    size_t len = typevec_len(events);
    for (size_t i = 0; i < len; i++)
    {
        const event_t *event = typevec_offset(events, i);
        print_notify(config, event);
    }
}
