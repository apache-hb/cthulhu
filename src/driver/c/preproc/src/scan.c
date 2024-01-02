#include "cpp/scan.h"
#include "core/macros.h"
#include "cthulhu/events/events.h"
#include "io/io.h"
#include "std/map.h"
#include "std/set.h"
#include "std/str.h"
#include "std/typed/vector.h"

#include "cpp_bison.h" // IWYU pragma: keep
#include "cpp_flex.h" // IWYU pragma: keep
#include "std/vector.h"

int cpp_extra_init(cpp_config_t config, cpp_extra_t *extra)
{
    typevec_t *result = typevec_new(sizeof(char), 0x1000, config.arena);
    typevec_t *comment = typevec_new(sizeof(char), 256, config.arena);

    map_t *defines = map_optimal_arena(1024, config.arena);
    map_t *include_cache = map_optimal_arena(1024, config.arena);

    set_t *nonexistent_files = set_new_arena(64, config.arena);

    size_t stack_size = config.max_include_depth;

    cpp_file_t **include_stack = ARENA_MALLOC(config.arena, sizeof(cpp_file_t *) * stack_size, "cpp_include_stack", NULL);

    cpp_extra_t builder = {
        .config = config,

        .result = result,
        .comment = comment,

        .branch_disable_index = SIZE_MAX,

        .defines = defines,
        .include_cache = include_cache,

        .nonexistent_files = nonexistent_files,

        .max_include_depth = stack_size,
        .include_stack = include_stack,
    };

    *extra = builder;

    int err = cpplex_init_extra(extra, &extra->yyscanner);
    if (err != 0) return err;

    return 0;
}

int cpp_parse(cpp_extra_t *extra)
{
    return cppparse(extra->yyscanner, extra);
}

static bool output_enabled(cpp_extra_t *extra)
{
    CTASSERT(extra != NULL);

    return extra->branch_disable_index == SIZE_MAX;
}

static void push_inner(cpp_extra_t *extra, text_t text)
{
    CTASSERT(extra != NULL);

    if (!output_enabled(extra))
        return;

    typevec_append(extra->result, text.text, text.size);
}

void cpp_push_output_single(cpp_extra_t *extra, char c)
{
    char data[1] = { c };
    text_t text = text_make(data, 1);
    push_inner(extra, text);
}

void cpp_push_output(cpp_extra_t *extra, text_t text)
{
    push_inner(extra, text);
}

void cpp_push_comment(cpp_extra_t *extra, const char *text, size_t size)
{
    CTASSERT(extra != NULL);
    CTASSERT(text != NULL);

    typevec_append(extra->comment, text, size);
}

text_t cpp_reset_comment(cpp_extra_t *extra)
{
    CTASSERT(extra != NULL);

    cpp_config_t config = extra->config;

    char *data = typevec_data(extra->comment);
    size_t size = typevec_len(extra->comment);

    text_t text = {
        .text = arena_strndup(data, size, config.arena),
        .size = size,
    };

    typevec_reset(extra->comment);

    return text;
}

text_t cpp_text_new(cpp_extra_t *extra, const char *text, size_t size)
{
    CTASSERT(extra != NULL);
    CTASSERT(text != NULL);

    cpp_config_t config = extra->config;

    text_t result = {
        .text = arena_strndup(text, size, config.arena),
        .size = size,
    };

    return result;
}

cpp_define_t *cpp_define_new(cpp_extra_t *extra, where_t where, text_t body)
{
    CTASSERT(extra != NULL);
    CTASSERT(body.text != NULL);

    cpp_config_t config = extra->config;
    cpp_file_t *file = extra->current_file;

    node_t *node = node_new(file->scan, where);

    cpp_define_t define = {
        .node = node,
        .body = body,
    };

    return arena_memdup(&define, sizeof(cpp_define_t), config.arena);
}

void cpp_add_define(cpp_extra_t *extra, where_t where, text_t name)
{
    CTASSERT(extra != NULL);
    CTASSERT(name.text != NULL);

    cpp_config_t config = extra->config;

    text_t text = cpp_text_new(extra, "", 0);
    cpp_define_t *define = cpp_define_new(extra, where, text);

    char *data = arena_strndup(name.text, name.size, config.arena);
    map_set(extra->defines, data, define);
}

void cpp_remove_define(cpp_extra_t *extra, where_t where, text_t name)
{
    CTU_UNUSED(where);

    CTASSERT(extra != NULL);
    CTASSERT(name.text != NULL);

    cpp_config_t config = extra->config;

    char *data = arena_strndup(name.text, name.size, config.arena);
    map_delete(extra->defines, data);
}

static void enter_branch(cpp_extra_t *extra, bool disable)
{
    CTASSERT(extra != NULL);

    if (disable && extra->branch_disable_index == SIZE_MAX)
    {
        extra->branch_disable_index = extra->branch_depth;
    }

    extra->branch_depth += 1;
}

static void leave_branch(cpp_extra_t *extra)
{
    CTASSERT(extra != NULL);
    CTASSERT(extra->branch_depth > 0);

    extra->branch_depth -= 1;

    if (extra->branch_disable_index == extra->branch_depth)
    {
        extra->branch_disable_index = SIZE_MAX;
    }
}

void cpp_ifdef(cpp_extra_t *extra, where_t where, text_t name)
{
    CTU_UNUSED(where);

    CTASSERT(extra != NULL);
    CTASSERT(name.text != NULL);

    cpp_config_t config = extra->config;

    char *data = arena_strndup(name.text, name.size, config.arena);
    cpp_define_t *define = map_get(extra->defines, data);

    bool disable = define == NULL;

    enter_branch(extra, disable);
}

void cpp_ifndef(cpp_extra_t *extra, where_t where, text_t name)
{
    CTU_UNUSED(where);

    CTASSERT(extra != NULL);
    CTASSERT(name.text != NULL);

    cpp_config_t config = extra->config;

    char *data = arena_strndup(name.text, name.size, config.arena);
    cpp_define_t *define = map_get(extra->defines, data);

    bool disable = define != NULL;

    enter_branch(extra, disable);
}

void cpp_if(cpp_extra_t *extra, where_t where)
{
    CTU_UNUSED(where);

    CTASSERT(extra != NULL);

    enter_branch(extra, true);
}

void cpp_elif(cpp_extra_t *extra, where_t where)
{
    CTU_UNUSED(where);

    CTASSERT(extra != NULL);

    leave_branch(extra);
    enter_branch(extra, false);
}

void cpp_else(cpp_extra_t *extra, where_t where)
{
    CTU_UNUSED(where);

    CTASSERT(extra != NULL);

    bool enabled = output_enabled(extra);

    leave_branch(extra);
    enter_branch(extra, !enabled);
}

void cpp_endif(cpp_extra_t *extra, where_t where)
{
    CTU_UNUSED(where);

    CTASSERT(extra != NULL);

    leave_branch(extra);
}

static cpp_file_t *cpp_file_new(cpp_extra_t *extra, scan_t *scan)
{
    CTASSERT(extra != NULL);
    CTASSERT(scan != NULL);

    text_view_t text = scan_source(scan);
    void *buffer = cpp_scan_bytes(text.text, (int)text.size, extra->yyscanner);

    cpp_file_t file = {
        .buffer = buffer,
        .scan = scan,
    };

    cpp_config_t config = extra->config;

    return arena_memdup(&file, sizeof(cpp_file_t), config.arena);
}

cpp_file_t *cpp_file_from_scan(cpp_extra_t *extra, scan_t *scan)
{
    return cpp_file_new(extra, scan);
}

cpp_file_t *cpp_file_from_io(cpp_extra_t *extra, io_t *io)
{
    CTASSERT(extra != NULL);
    CTASSERT(io != NULL);

    cpp_config_t config = extra->config;

    scan_t *scan = scan_io("C Header", io, config.arena);
    return cpp_file_new(extra, scan);
}

void update_flex_location(cpp_extra_t *extra, where_t where)
{
    CTASSERT(extra != NULL);

    where_t *loc = cppget_lloc(extra->yyscanner);
    *loc = where;
}

void cpp_set_current_file(cpp_extra_t *extra, cpp_file_t *file)
{
    CTASSERT(extra != NULL);
    CTASSERT(file != NULL);

    ctu_log("%s - %s (depth: %zu)", str_repeat("  ", extra->include_depth), scan_path(file->scan), extra->include_depth);
    extra->current_file = file;

    cpp_switch_to_buffer(file->buffer, extra->yyscanner);
}

static void rewind_file_where(cpp_file_t *file)
{
    CTASSERT(file != NULL);

    where_t start = { 0 };
    file->where = start;
}

static void cpp_push_file(cpp_extra_t *extra, cpp_file_t *file)
{
    CTASSERT(extra != NULL);
    CTASSERT(file != NULL);

    extra->include_stack[extra->include_depth++] = extra->current_file;
    rewind_file_where(file);
    cpp_set_current_file(extra, file);
    update_flex_location(extra, file->where);
}

static void cpp_pop_file(cpp_extra_t *extra)
{
    CTASSERT(extra != NULL);
    CTASSERT(extra->include_depth > 0);

    cpp_file_t *file = extra->include_stack[--extra->include_depth];
    cpp_set_current_file(extra, file);
    update_flex_location(extra, file->where);
}

static cpp_file_t *find_include(cpp_extra_t *extra, const char *base, const char *path, size_t length)
{
    cpp_config_t config = extra->config;
    char *joined = str_format(config.arena, "%s" NATIVE_PATH_SEPARATOR "%.*s", base, (int)length, path);

    if (set_contains(extra->nonexistent_files, joined))
    {
        return NULL;
    }

    cpp_file_t *cached = map_get(extra->include_cache, joined);
    if (cached != NULL)
    {
        return cached;
    }

    io_t *io = io_file_arena(joined, eAccessRead | eAccessText, config.arena);
    os_error_t err = io_error(io);
    if (err != 0)
    {
        set_add(extra->nonexistent_files, joined);
        io_close(io);
        return NULL;
    }

    scan_t *scan = scan_io("C Header", io, config.arena);
    cpp_file_t *file = cpp_file_new(extra, scan);
    map_set(extra->include_cache, joined, file);

    return file;
}

static bool is_valid_include(cpp_extra_t *extra, cpp_file_t *file)
{
    return file != NULL && file != extra->current_file;
}

static void setup_new_file(cpp_extra_t *extra, cpp_file_t *file)
{
    CTASSERT(extra != NULL);
    CTASSERT(file != NULL);

    // we have this for once we support pragma once

    cpp_push_file(extra, file);
}

static bool include_using_extra(cpp_extra_t *extra, const char *path, size_t length)
{
    cpp_config_t config = extra->config;

    size_t len = vector_len(config.include_directories);
    for (size_t i = 0; i < len; i++)
    {
        const char *base = vector_get(config.include_directories, i);
        cpp_file_t *inner = find_include(extra, base, path, length);
        if (is_valid_include(extra, inner))
        {
            setup_new_file(extra, inner);
            return true;
        }
    }

    return false;
}

void cpp_include_local(cpp_extra_t *extra, const char *path, size_t length)
{
    CTASSERT(extra != NULL);
    CTASSERT(path != NULL);

    cpp_config_t config = extra->config;
    cpp_file_t *current = extra->current_file;
    const char *current_dir = str_directory(config.arena, scan_path(current->scan));
    cpp_file_t *file = find_include(extra, current_dir, path, length);
    if (is_valid_include(extra, file))
    {
        setup_new_file(extra, file);
    }

    if (include_using_extra(extra, path, length))
    {
        return;
    }


    ctu_log("failed to find include: %.*s", (int)length, path);
}

void cpp_include_system(cpp_extra_t *extra, const char *path, size_t length)
{
    CTASSERT(extra != NULL);
    CTASSERT(path != NULL);

    if (include_using_extra(extra, path, length))
    {
        return;
    }

    ctu_log("failed to find include: %.*s", (int)length, path);
}

void cpp_include_define(cpp_extra_t *extra, const char *name, size_t length)
{
    CTASSERT(extra != NULL);
    CTASSERT(name != NULL);

    ctu_log("include define: %.*s", (int)length, name);
}

bool cpp_leave_file(cpp_extra_t *extra)
{
    CTASSERT(extra != NULL);

    if (extra->include_depth == 0)
        return false;

    cpp_pop_file(extra);
    return true;
}

int cpp_input(cpp_extra_t *extra, char *out, int size)
{
    cpp_file_t *file = extra->current_file;
    return flex_input(file->scan, out, size);
}

cpp_extra_t *cpp_get_scanner_extra(void *yyscanner)
{
    CTASSERT(yyscanner != NULL);

    return (cpp_extra_t*)cppget_extra(yyscanner);
}

arena_t *cpp_get_scanner_arena(void *yyscanner)
{
    cpp_extra_t *extra = cpp_get_scanner_extra(yyscanner);
    cpp_config_t config = extra->config;
    return config.arena;
}

logger_t *cpp_get_scanner_logger(void *yyscanner)
{
    cpp_extra_t *extra = cpp_get_scanner_extra(yyscanner);
    cpp_config_t config = extra->config;
    return config.logger;
}

void cpperror(where_t *where, void *yyscanner, cpp_extra_t *extra, const char *msg)
{
    CTU_UNUSED(yyscanner);
    CTU_UNUSED(where);

    cpp_config_t config = extra->config;
    cpp_file_t *file = extra->current_file;

    evt_scan_error(config.logger, node_new(file->scan, *where), msg);
}
