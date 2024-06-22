#include "cpp/scan.h"
#include "core/macros.h"
#include "cthulhu/events/events.h"
#include "io/io.h"
#include "std/fifo.h"
#include "std/map.h"
#include "std/set.h"
#include "std/str.h"
#include "std/typed/vector.h"

#include "cpp_bison.h" // IWYU pragma: keep
#include "cpp_flex.h" // IWYU pragma: keep
#include "std/vector.h"

typedef struct synthetic_token_t
{
    int token;
    YYSTYPE value;
    YYLTYPE where;
} synthetic_token_t;

int cpp_extra_init(cpp_config_t config, cpp_extra_t *extra)
{
    typevec_t *result = typevec_new(sizeof(char), 0x1000, config.arena);
    typevec_t *comment = typevec_new(sizeof(char), 256, config.arena);

    fifo_t *tokens = fifo_new(sizeof(synthetic_token_t), 256, config.arena);

    map_t *defines = map_optimal_arena(1024, config.arena);
    map_t *include_cache = map_optimal_arena(1024, config.arena);

    set_t *nonexistent_files = set_new_arena(64, config.arena);

    size_t stack_size = config.max_include_depth;

    cpp_file_t **include_stack = ARENA_MALLOC(config.arena, sizeof(cpp_file_t *) * stack_size, "cpp_include_stack", NULL);

    cpp_extra_t builder = {
        .config = config,

        .yypstate = cpppstate_new(),

        .result = result,
        .comment = comment,
        .tokens = tokens,

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

static int get_next_token(cpp_extra_t *extra, CPPSTYPE *value, where_t *where)
{
    if (fifo_is_empty(extra->tokens))
    {
        return cpplex(value, where, extra->yyscanner);
    }

    synthetic_token_t token = { 0 };
    fifo_remove(extra->tokens, &token);

    *value = token.value;
    *where = token.where;

    return token.token;
}

int cpp_parse(cpp_extra_t *extra)
{
    int status = 0;
    CPPSTYPE value = { 0 };
    do {
        cpp_file_t *file = extra->current_file;
        int tok = get_next_token(extra, &value, &file->where);
        status = cpppush_parse(extra->yypstate, tok, &value, &file->where, extra->yyscanner, extra);
    } while (status == YYPUSH_MORE);

    return status;
}

static bool output_disabled(cpp_extra_t *extra)
{
    CTASSERT(extra != NULL);

    return extra->branch_disable_index != SIZE_MAX;
}

static bool inside_directive(cpp_extra_t *extra)
{
    CTASSERT(extra != NULL);

    return extra->inside_directive;
}

static void push_inner(cpp_extra_t *extra, text_t text)
{
    CTASSERT(extra != NULL);

    if (output_disabled(extra) || inside_directive(extra))
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

void cpp_push_token(cpp_extra_t *extra, int token, void *value, where_t where)
{
    synthetic_token_t synthetic = {
        .token = token,
        .where = where,
    };

    memcpy(&synthetic.value, value, sizeof(YYSTYPE));

    fifo_insert(extra->tokens, &synthetic);
}

void cpp_push_ident(cpp_extra_t *extra, text_t text)
{
    CTASSERT(extra != NULL);
    CTASSERT(text.text != NULL);

    cpp_config_t config = extra->config;

    char *data = arena_strndup(text.text, text.size, config.arena);
    cpp_define_t *define = map_get(extra->defines, data);

    if (define == NULL)
    {
        push_inner(extra, text);
        return;
    }

    size_t len = vector_len(define->body);
    for (size_t i = 0; i < len; i++)
    {
        synthetic_token_t *token = vector_get(define->body, i);
        cpp_push_token(extra, token->token, &token->value, token->where);
    }
}

cpp_ast_t *cpp_expand_ident(cpp_extra_t *extra, where_t where, text_t text)
{
    CTU_UNUSED(where);

    CTASSERT(extra != NULL);
    CTASSERT(text.text != NULL);

    cpp_config_t config = extra->config;

    char *data = arena_strndup(text.text, text.size, config.arena);
    cpp_define_t *define = map_get(extra->defines, data);

    if (define == NULL)
    {
        cpp_file_t *file = extra->current_file;
        return cpp_ast_ident(file->scan, where, text);
    }

    size_t len = vector_len(define->body);
    for (size_t i = 0; i < len; i++)
    {
        synthetic_token_t *token = vector_get(define->body, i);
        cpp_push_token(extra, token->token, &token->value, token->where);
    }

    // TODO: return the next token
    return NULL;
}

cpp_ast_t *cpp_expand_macro(cpp_extra_t *extra, where_t where, text_t text, vector_t *args)
{
    // TODO: expand macro nicely

    CTU_UNUSED(extra);
    CTU_UNUSED(where);
    CTU_UNUSED(text);
    CTU_UNUSED(args);

    return NULL;
}

void cpp_push_comment(cpp_extra_t *extra, const char *text, size_t size)
{
    CTASSERT(extra != NULL);
    CTASSERT(text != NULL);

    // TODO: do we want to emit comments inside falsey branches?
    if (inside_directive(extra))
        return;

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

void cpp_enter_directive(cpp_extra_t *extra)
{
    CTASSERT(extra != NULL);
    CTASSERT(!extra->inside_directive);

    extra->inside_directive = true;
}

void cpp_leave_directive(cpp_extra_t *extra)
{
    CTASSERT(extra != NULL);
    CTASSERT(extra->inside_directive);

    extra->inside_directive = false;
}

synthetic_token_t *cpp_token_new(cpp_extra_t *extra, int token, void *value, where_t where)
{
    CTASSERT(extra != NULL);

    cpp_config_t config = extra->config;

    synthetic_token_t synthetic = {
        .token = token,
        .where = where,
    };

    memcpy(&synthetic.value, value, sizeof(YYSTYPE));

    return arena_memdup(&synthetic, sizeof(synthetic_token_t), config.arena);
}

cpp_number_t *cpp_number_new(cpp_extra_t *extra, const char *text, size_t len, int base)
{
    CTASSERT(extra != NULL);
    CTASSERT(text != NULL);

    cpp_config_t config = extra->config;

    char *data = arena_strndup(text, len, config.arena);
    cpp_number_t *number = ARENA_MALLOC(config.arena, sizeof(cpp_number_t), "cpp_number_t", NULL);
    number->text = cpp_text_new(extra, data, len);
    mpz_init_set_str(number->value, data, base);

    return number;
}

cpp_define_t *cpp_define_new(cpp_extra_t *extra, where_t where, vector_t *body)
{
    CTASSERT(extra != NULL);
    CTASSERT(body != NULL);

    cpp_config_t config = extra->config;
    cpp_file_t *file = extra->current_file;

    node_t *node = node_new(file->scan, where);

    cpp_define_t define = {
        .node = node,
        .body = body,
    };

    return arena_memdup(&define, sizeof(cpp_define_t), config.arena);
}

void cpp_add_define(cpp_extra_t *extra, where_t where, text_t name, vector_t *body)
{
    CTASSERT(extra != NULL);
    CTASSERT(name.text != NULL);

    cpp_config_t config = extra->config;

    cpp_define_t *define = cpp_define_new(extra, where, body);

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
    if (extra->branch_depth == 0)
        return;

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

void cpp_if(cpp_extra_t *extra, where_t where, cpp_ast_t *ast)
{
    CTU_UNUSED(where);
    CTU_UNUSED(ast);

    CTASSERT(extra != NULL);

    enter_branch(extra, true);
}

void cpp_elif(cpp_extra_t *extra, where_t where, cpp_ast_t *ast)
{
    CTU_UNUSED(where);
    CTU_UNUSED(ast);

    CTASSERT(extra != NULL);

    leave_branch(extra);
    enter_branch(extra, false);
}

void cpp_else(cpp_extra_t *extra, where_t where)
{
    CTU_UNUSED(where);

    CTASSERT(extra != NULL);

    bool enabled = output_disabled(extra);

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

    CPPLTYPE *loc = cppget_lloc(extra->yyscanner);
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

    if (extra->include_depth >= extra->max_include_depth)
    {
        ctu_log("max include depth reached");
        return;
    }

    if (output_disabled(extra))
        return;

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

    ctu_log("%s:%zu:%zu: %s", scan_path(file->scan), where->first_line, where->first_column, msg);

    evt_scan_error(config.logger, node_new(file->scan, *where), msg);
}
