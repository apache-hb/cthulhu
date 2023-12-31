#include "cpp/scan.h"

#include "base/panic.h"
#include "core/macros.h"
#include "cpp/cpp.h"
#include "cthulhu/events/events.h"
#include "io/io.h"
#include "memory/arena.h"
#include "notify/notify.h"
#include "scan/scan.h"
#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"

#include "cpp/events.h"
#include "std/vector.h"

#include "cpp_bison.h" // IWYU pragma: keep
#include "cpp_flex.h" // IWYU pragma: keep

cpp_scan_t *cpp_scan_context(scan_t *scan)
{
    return scan_get_context(scan);
}

cpp_scan_t cpp_scan_new(cpp_instance_t *instance)
{
    CTASSERT(instance != NULL);
    CTASSERT(instance->include_depth > 0);

    typevec_t *result = typevec_new(sizeof(char), 0x1000, instance->arena);
    cpp_file_t **stack = ARENA_MALLOC(instance->arena, sizeof(cpp_file_t*) * instance->include_depth, "stack", NULL);

    cpp_scan_t scan = {
        .instance = instance,

        .result = result,

        .defines = map_optimal_arena(64, instance->arena),
        .files = map_optimal_arena(64, instance->arena),

        .stack_index = 0,
        .stack_size = instance->include_depth,
        .stack = stack
    };

    return scan;
}

void cpp_scan_consume(scan_t *scan, const char *text, size_t size)
{
    cpp_scan_t *self = cpp_scan_context(scan);
    typevec_append(self->result, text, size);
}

static bool verify_recursion_depth(yyscan_t yyscanner, const char *text)
{
    scan_t *scan = cppget_extra(yyscanner);
    cpp_scan_t *self = cpp_scan_context(scan);

    where_t *where = cppget_lloc(yyscanner);

    if (self->stack_index >= self->stack_size)
    {
        node_t *node = cpp_get_node(yyscanner, *where);
        msg_notify(self->instance->logger, &kEvent_IncludeDepthExceeded, node,
            "include depth exceeded when trying to include file `%s`", text);
        return false;
    }

    return true;
}

static void cpp_scan_push(scan_t *scan, cpp_file_t *file)
{
    cpp_scan_t *self = cpp_scan_context(scan);

    CTASSERTF(self->stack_index < self->stack_size, "include depth exceeded (stack_index=%zu, stack_size=%zu)", self->stack_index, self->stack_size);

    self->stack[self->stack_index++] = file;
}

static cpp_file_t *cpp_file_pop(cpp_scan_t *self)
{
    if (self->stack_index == 0)
        return NULL;

    return self->stack[--self->stack_index];
}

void cpp_set_current_file(cpp_scan_t *self, cpp_file_t *file)
{
    CTASSERT(self != NULL);
    CTASSERT(file != NULL);

    self->current_file = file;
}

static void set_current_buffer(yyscan_t yyscanner, cpp_file_t *file)
{
    scan_t *scan = cppget_extra(yyscanner);
    cpp_scan_t *self = cpp_scan_context(scan);

    //ctu_log("%s - \"%s\" (depth: %zu)", str_repeat("  ", self->stack_index), scan_path(file->scan), self->stack_index);

    // update the location to point to the current files location data
    where_t *where = cppget_lloc(yyscanner);
    *where = file->where;

    cpp_set_current_file(self, file);
    cpp_switch_to_buffer(file->buffer, yyscanner);
}

static cpp_file_t *file_new(void *yyscanner, scan_t *scan)
{
    CTASSERT(scan != NULL);
    CTASSERT(yyscanner != NULL);

    text_view_t source = scan_source(scan);

    cpp_file_t file = {
        .buffer = cpp_scan_bytes(source.text, (int)source.size, yyscanner),
        .scan = scan,
        .pragma_once = false,
    };

    return arena_memdup(&file, sizeof(cpp_file_t), scan_get_arena(scan));
}

cpp_file_t *cpp_file_from_scan(scan_t *scan, void *yyscanner)
{
    CTASSERT(scan != NULL);

    return file_new(yyscanner, scan);
}

cpp_file_t *cpp_file_from_io(arena_t *arena, void *yyscanner, io_t *io)
{
    CTASSERT(io != NULL);

    scan_t *scan = scan_io("C Header", io, arena);
    return file_new(yyscanner, scan);
}

static void reset_file_where(cpp_file_t *file)
{
    where_t start = {
        .first_line = 1,
        .last_line = 1,
        .first_column = 0,
        .last_column = 0,
    };
    file->where = start;
}

static void enter_included_file(yyscan_t yyscanner, scan_t *scan, cpp_file_t *file)
{
    CTASSERT(scan != NULL);

    if (file == NULL)
        return;

    if (file->pragma_once)
        return;

    cpp_scan_t *self = cpp_scan_context(scan);

    cpp_scan_push(scan, self->current_file);
    reset_file_where(file);
    set_current_buffer(yyscanner, file);
}

static cpp_file_t *try_include(void *yyscanner, const char *dir, const char *text)
{
    scan_t *scan = cppget_extra(yyscanner);
    cpp_scan_t *self = cpp_scan_context(scan);
    arena_t *arena = scan_get_arena(scan);

    char *path = str_format(arena, "%s" NATIVE_PATH_SEPARATOR "%s", dir, text);

    cpp_file_t *existing = map_get(self->files, path);
    if (existing != NULL)
    {
        return existing;
    }

    io_t *io = io_file_arena(path, eAccessRead | eAccessText, arena);
    os_error_t err = io_error(io);
    if (err != 0)
    {
        io_close(io);
        return NULL;
    }

    cpp_file_t *file = cpp_file_from_io(arena, yyscanner, io);

    io_close(io);
    map_set(self->files, path, file);

    return file;
}

void cpp_accept_include(void *yyscanner, const char *text)
{
    scan_t *scan = cppget_extra(yyscanner);
    if (!verify_recursion_depth(yyscanner, text))
        return;

    arena_t *arena = scan_get_arena(scan);
    cpp_scan_t *self = cpp_scan_context(scan);

    cpp_file_t *current = self->current_file;
    CTASSERT(current != NULL);

    char *dir = str_directory(arena, scan_path(current->scan));
    cpp_file_t *inc = try_include(yyscanner, dir, text);
    if (inc && inc != current)
    {
        enter_included_file(yyscanner, scan, inc);
        return;
    }

    vector_t *include_dirs = self->instance->include_directories;
    size_t len = vector_len(include_dirs);
    for (size_t i = 0; i < len; i++)
    {
        const char *incdir = vector_get(include_dirs, i);
        cpp_file_t *state = try_include(yyscanner, incdir, text);

        if (state != NULL && state != current)
        {
            enter_included_file(yyscanner, scan, state);
            return;
        }
    }

    enter_included_file(yyscanner, scan, NULL);
}

void cpp_accept_define_include(void *yyscanner, const char *text)
{
    CTU_UNUSED(yyscanner);
    CTU_UNUSED(text);
}

bool cpp_leave_file(void *yyscanner)
{
    scan_t *scan = cppget_extra(yyscanner);
    cpp_scan_t *self = cpp_scan_context(scan);
    cpp_file_t *file = cpp_file_pop(self);

    if (file == NULL)
        return true;

    set_current_buffer(yyscanner, file);
    return false;
}

node_t *cpp_get_node(void *yyscanner, where_t where)
{
    scan_t *scan = cppget_extra(yyscanner);
    cpp_scan_t *self = cpp_scan_context(scan);
    cpp_file_t *file = self->current_file;

    return node_new(file->scan, where);
}

void cpperror(where_t *where, void *yyscanner, scan_t *scan, const char *msg)
{
    CTU_UNUSED(yyscanner);
    CTU_UNUSED(where);

    cpp_scan_t *ctx = cpp_scan_context(scan);
    evt_scan_error(ctx->instance->logger, cpp_get_node(yyscanner, *where), msg);
}
