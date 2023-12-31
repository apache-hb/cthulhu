#include "cpp/cpp.h"
#include "cpp/events.h"

#include "base/log.h"
#include "base/panic.h"

#include "interop/compile.h"

#include "cpp/scan.h"
#include "io/io.h"
#include "memory/arena.h"
#include "scan/scan.h"

#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"
#include <string.h>

#include "cpp_bison.h"
#include "cpp_flex.h"

#define NEW_EVENT(id, ...) const diagnostic_t kEvent_##id = __VA_ARGS__;
#include "cpp/events.inc"

cpp_define_t *cpp_define_new(arena_t *arena, const char *name, const char *value)
{
    CTASSERT(arena != NULL);
    CTASSERT(name != NULL);
    CTASSERT(value != NULL);

    cpp_define_t define = {
        .name = name,
        .value = value,
    };

    return arena_memdup(&define, sizeof(cpp_define_t), arena);
}

cpp_instance_t cpp_instance_new(arena_t *arena, logger_t *logger)
{
    CTASSERT(arena != NULL);
    CTASSERT(logger != NULL);

    cpp_instance_t instance = {
        .arena = arena,
        .logger = logger,

        .include_directories = vector_new_arena(4, arena),
        .defines = map_optimal_arena(64, arena)
    };

    return instance;
}

io_t *cpp_process_file(cpp_instance_t *instance, scan_t *source)
{
    CTASSERT(instance != NULL);
    CTASSERT(source != NULL);

    yyscan_t scanner = NULL;
    int err = 0;

    cpp_scan_t extra = cpp_scan_new(instance);
    scan_set_context(source, &extra);

    err = cpplex_init_extra(source, &scanner);
    CTASSERTF(err == 0, "failed to init scanner: %d", err);

    // do the intial setup for the first file
    const char *initial_path = scan_path(source);
    cpp_file_t *initial = cpp_file_from_scan(source, scanner);
    map_set(extra.files, initial_path, initial);
    extra.current_file = initial;

    ctu_log("start: %s", initial_path);

    err = cppparse(scanner, source);
    if (err != 0)
    {
        ctu_log("failed to parse: %d", err);
    }

    cpplex_destroy(scanner);

    const char *result_text = typevec_data(extra.result);
    size_t result_size = typevec_len(extra.result);

    io_t *io = io_view("out", result_text, result_size, instance->arena);

    return io;
}
