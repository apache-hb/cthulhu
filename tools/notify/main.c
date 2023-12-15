#include "io/io.h"

#include "memory/memory.h"

#include "notify/notify.h"
#include "notify/text.h"

#include "report/report.h"

#include "scan/node.h"
#include "std/vector.h"
#include <stdio.h>

const char *kSampleSource =
    "module main;\n"        // 1
    "\n"                    // 2
    "import multi.lhs,\n"   // 3
    "       multi.rhs;\n"   // 4
    "\n"                    // 5
    "const LIMIT = 25;\n"   // 6
    "var x;\n"              // 7
    "\n"                    // 8
    "procedure entry;\n"    // 9
    "begin\n"               // 10
    "    x := 0;\n"         // 11
    "    call lhs;\n"       // 12
    "    ! x\n"             // 13
    "end;\n"                // 14
    "\n"                    // 15
    "call entry\n"          // 16
    ".\n";                  // 17

static const diagnostic_t kInfoDiagnostic = {
    .severity = eSeverityInfo,
    .id = "T0001",
    .brief = "Test diagnostic at info level",
    .description =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n"
        "Sed non risus. Suspendisse lectus tortor, dignissim sit amet,\n"
        "adipiscing nec, ultricies sed, dolor. Cras elementum ultrices diam.\n"
        "Maecenas ligula massa, varius a, semper congue, euismod non, mi.\n"
        "Proin porttitor, orci nec nonummy molestie, enim est eleifend mi,\n"
        "non fermentum diam nisl sit amet erat. Duis semper. Duis arcu massa,\n"
        "scelerisque vitae, consequat in, pretium a, enim. Pellentesque congue.\n"
};

static const diagnostic_t kUndefinedFunctionName = {
    .severity = eSeverityFatal,
    .id = "G0001",
    .brief = "Undefined function name",
    .description =
        "A function name must be visible in the current scope\n"
        "to be used in a call expression or statement.\n"
};

static const diagnostic_t kUnresolvedImport = {
    .severity = eSeverityFatal,
    .id = "G0002",
    .brief = "Unresolved import",
    .description =
        "An import statement must refer to a valid module.\n"
        "The module must be visible in the current scope.\n"
};

void event_simple(logger_t *logs)
{
    event_t *event = msg_notify(logs, &kInfoDiagnostic, node_builtin(), "test");
    msg_append(event, node_builtin(), "hello %s", "world");
}

void event_missing_call(logger_t *logs, scan_t *scan)
{
    where_t where = {
        .first_line = 11,
        .last_line = 11,
        .first_column = 4,
        .last_column = 12
    };

    node_t *node = node_new(scan, where);

    event_t *event = msg_notify(logs, &kUndefinedFunctionName, node, "undefined function name `%s`", "lhs");
    msg_note(event, "did you mean `%s`?", "rhs");
    msg_underline(event, "function called here");
}

void event_invalid_import(logger_t *logs, scan_t *scan)
{
    where_t where = {
        .first_line = 2,
        .last_line = 2,
        .first_column = 7,
        .last_column = 7 + 9
    };

    node_t *node = node_new(scan, where);

    event_t *event = msg_notify(logs, &kUnresolvedImport, node, "unresolved import `%s`", "multi.lhs");
    msg_note(event, "did you mean `%s`?", "multi.rhs");
    msg_note(event, "did you mean `%s`?", "multi.rhx");
    msg_underline(event, "import statement here");
}

int main()
{
    init_global_alloc(ctu_default_alloc());
    init_gmp_alloc(ctu_default_alloc());

    logger_t *logs = log_new();
    msg_diagnostic(logs, &kInfoDiagnostic);
    msg_diagnostic(logs, &kUndefinedFunctionName);
    msg_diagnostic(logs, &kUnresolvedImport);

    reports_t *reports = begin_reports();

    io_t *source = io_string("sample.pl0", kSampleSource);
    scan_t *scan = scan_io(reports, "PL/0", source, ctu_default_alloc());

    event_simple(logs);
    event_missing_call(logs, scan);
    event_invalid_import(logs, scan);

    io_t *io = io_blob("test", 0x1000, eAccessWrite);

    text_config_t config = {
        .zero_line = true,
        .colour = true,
        .io = io
    };

    vector_t *events = log_events(logs);
    size_t count = vector_len(events);

    for (size_t i = 0; i < count; i++)
    {
        event_t *event = vector_get(events, i);
        text_report(config, event);
    }

    const void *data = io_map(io);
    size_t size = io_size(io);

    fwrite(data, size, 1, stdout);
}
