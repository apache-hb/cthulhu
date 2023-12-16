#include "common.h"

#include "notify/text.h"

#include "base/panic.h"

#include "core/macros.h"
#include "memory/memory.h"
#include "notify/notify.h"
#include "scan/node.h"

#include "io/io.h"

#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

#include <stdlib.h>
#include <string.h>

void print_file_contents(text_config_t config, sparse_text_t *text)
{
    CTU_UNUSED(config);
    CTASSERT(text != NULL);
}

void print_event_header(text_config_t config, const event_t *event)
{
    const diagnostic_t *diagnostic = event->diagnostic;
    const char *id = diagnostic->id;
    const char *sev = get_severity_name(diagnostic->severity);
    colour_t colour = get_severity_colour(diagnostic->severity);
    const char *lvl = fmt_coloured(config.colours, colour, "%s [%s]:", sev, id);

    io_printf(config.io, "%s %s\n", lvl, event->message);
}

void text_report_rich(text_config_t config, const event_t *event)
{
    CTASSERT(config.io != NULL);
    CTASSERT(event != NULL);

    sparse_report_t *report = sparse_report_new(event);
    vector_t *files = sparse_report_get_files(report);
    size_t len = vector_len(files);

    print_event_header(config, event);

    for (size_t i = 0; i < len; i++)
    {
        sparse_text_t *text = vector_get(files, i);
        print_file_contents(config, text);
    }
}
