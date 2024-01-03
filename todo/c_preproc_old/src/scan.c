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

void cpp_scan_consume(scan_t *scan, const char *text, size_t size, bool is_comment)
{
    if (is_skipping(scan))
        return;

    cpp_scan_t *self = cpp_scan_context(scan);
    if (!self->instance->paste_comments && is_comment)
        return;

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

static node_t *get_scan_node(scan_t *scan, where_t where)
{
    cpp_scan_t *extra = cpp_scan_context(scan);
    cpp_file_t *file = extra->current_file;

    return node_new(file->scan, where);
}

node_t *cpp_get_node(void *yyscanner, where_t where)
{
    scan_t *scan = cppget_extra(yyscanner);

    return get_scan_node(scan, where);
}

static void check_old_definition(scan_t *scan, const char *name, where_t where)
{
    cpp_scan_t *self = cpp_scan_context(scan);

    cpp_ast_t *old = map_get(self->defines, name);
    if (old != NULL)
    {
        node_t *node = get_scan_node(scan, where);
        event_t *id = msg_notify(self->instance->logger, &kEvent_MacroRedefinition, node, "macro `%s` redefined", name);
        msg_append(id, old->node, "previous definition was here");
    }
}

void cpp_add_define(scan_t *scan, where_t where, const char *name, vector_t *body)
{
    CTASSERT(scan != NULL);
    CTASSERT(name != NULL);

    cpp_scan_t *self = cpp_scan_context(scan);
    check_old_definition(scan, name, where);

    cpp_ast_t *ast = cpp_define(scan, where, name, body);

    map_set(self->defines, name, ast);
}

void cpp_add_macro(scan_t *scan, where_t where, const char *name, cpp_params_t params, vector_t *body)
{
    CTASSERT(scan != NULL);
    CTASSERT(name != NULL);
    CTASSERT(body != NULL);

    cpp_scan_t *self = cpp_scan_context(scan);
    check_old_definition(scan, name, where);

    cpp_ast_t *ast = cpp_macro(scan, where, name, params, body);

    map_set(self->defines, name, ast);
}

cpp_params_t make_params(vector_t *names, bool variadic)
{
    CTASSERT(names != NULL);

    cpp_params_t params = {
        .names = names,
        .variadic = variadic,
    };

    return params;
}

void cpp_remove_define(scan_t *scan, where_t where, const char *name)
{
    CTASSERT(scan != NULL);
    CTASSERT(name != NULL);

    cpp_scan_t *self = cpp_scan_context(scan);
    cpp_ast_t *ast = map_get(self->defines, name);
    if (ast == NULL)
    {
        node_t *node = get_scan_node(scan, where);
        msg_notify(self->instance->logger, &kEvent_MacroNotDefined, node, "macro `%s` was not defined", name);
        return;
    }
    map_delete(self->defines, name);
}

cpp_ast_t *cpp_get_define(scan_t *scan, const char *name)
{
    CTASSERT(scan != NULL);
    CTASSERT(name != NULL);

    cpp_scan_t *self = cpp_scan_context(scan);

    return map_get(self->defines, name);
}

bool is_skipping(scan_t *scan)
{
    cpp_scan_t *self = cpp_scan_context(scan);
    return self->skipping;
}

static void do_branch_enter(scan_t *scan, bool truthy)
{
    cpp_scan_t *self = cpp_scan_context(scan);
    if (self->branch_depth == 0)
    {
        self->skipping = !truthy;
    }

    self->branch_depth += 1;
}

void enter_ifdef(scan_t *scan, where_t where, const char *name)
{
    CTU_UNUSED(where);

    bool truthy = cpp_get_define(scan, name) != NULL;

    do_branch_enter(scan, truthy);
}

void enter_ifndef(scan_t *scan, where_t where, const char *name)
{
    CTU_UNUSED(where);

    bool truthy = cpp_get_define(scan, name) == NULL;

    do_branch_enter(scan, truthy);
}

void enter_branch(scan_t *scan, where_t where, vector_t *condition)
{
    const node_t *node = get_scan_node(scan, where);
    bool truthy = eval_condition(scan, node, condition);

    do_branch_enter(scan, truthy);
}

void else_branch(scan_t *scan, where_t where)
{
    cpp_scan_t *self = cpp_scan_context(scan);
    if (self->branch_depth == 0)
    {
        node_t *node = get_scan_node(scan, where);
        msg_notify(self->instance->logger, &kEvent_UnexpectedElse, node, "unexpected #else");
    }
    else
    {
        self->skipping = !self->skipping;
    }
}

void elif_branch(scan_t *scan, where_t where, vector_t *condition)
{
    node_t *node = get_scan_node(scan, where);
    bool truthy = eval_condition(scan, node, condition);

    cpp_scan_t *self = cpp_scan_context(scan);
    if (self->branch_depth == 0)
    {
        msg_notify(self->instance->logger, &kEvent_UnexpectedElif, node, "unexpected #elif");
    }
    else
    {
        self->skipping = !truthy;
    }
}

void leave_branch(scan_t *scan, where_t where)
{
    cpp_scan_t *self = cpp_scan_context(scan);
    if (self->branch_depth == 0)
    {
        node_t *node = get_scan_node(scan, where);
        msg_notify(self->instance->logger, &kEvent_UnexpectedEndIf, node, "unexpected #endif");
    }
    else
    {
        self->branch_depth -= 1;
    }

    if (self->branch_depth == 0)
    {
        self->skipping = false;
    }
}

cpp_number_t make_number(scan_t *scan, const char *text, size_t len, int base)
{
    CTASSERT(scan != NULL);
    CTASSERT(text != NULL);

    cpp_number_t number = {
        .text = arena_strndup(text, len, scan_get_arena(scan)),
        .base = base,
    };

    return number;
}

void cpp_accept_pragma(scan_t *scan, where_t where, vector_t *tokens)
{
    CTU_UNUSED(where);

    cpp_scan_t *self = cpp_scan_context(scan);
    if (vector_len(tokens) == 0)
        return;

    cpp_ast_t *first = vector_get(tokens, 0);
    if (cpp_ast_is_not(first, eCppIdent))
        return;

    const char *name = first->text;
    if (str_equal(name, "once"))
    {
        self->current_file->pragma_once = true;
    }
}

static void paste_token(scan_t *scan, const cpp_ast_t *node)
{
    switch (node->kind)
    {
    case eCppPaste:
    case eCppString:
    case eCppIdent:
        cpp_scan_consume(scan, node->text, strlen(node->text), false);
        break;

    case eCppNumber:
        cpp_scan_consume(scan, node->original, strlen(node->original), false);
        break;

    case eCppLParen:
        cpp_scan_consume(scan, "(", 1, false);
        break;
    case eCppRParen:
        cpp_scan_consume(scan, ")", 1, false);
        break;

    case eCppComma:
        cpp_scan_consume(scan, ",", 1, false);
        break;

    case eCppColon:
        cpp_scan_consume(scan, ":", 1, false);
        break;

    case eCppCompare: {
        const char *cmp = compare_symbol(node->compare);
        cpp_scan_consume(scan, cmp, strlen(cmp), false);
        break;
    }
    case eCppBinary: {
        const char *bin = binary_symbol(node->binary);
        cpp_scan_consume(scan, bin, strlen(bin), false);
        break;
    }
    case eCppUnary: {
        const char *un = unary_symbol(node->unary);
        cpp_scan_consume(scan, un, strlen(un), false);
        break;
    }

    default:
        NEVER("unexpected token %d in paste", node->kind);
    }
}

void cpp_expand_ident(scan_t *scan, where_t where, const char *name, size_t size)
{
    CTU_UNUSED(where);

    cpp_scan_t *self = cpp_scan_context(scan);
    cpp_ast_t *ast = map_get(self->defines, name);
    if (ast == NULL)
    {
        cpp_scan_consume(scan, name, size, false);
        return;
    }

    if (ast->kind == eCppDefine)
    {
        vector_t *body = ast->body;
        size_t len = vector_len(body);
        for (size_t i = 0; i < len; i++)
        {
            cpp_ast_t *node = vector_get(body, i);
            paste_token(scan, node);
        }
    }
}

void cpp_expand_macro(scan_t *scan, where_t where, const char *name, size_t size, vector_t *args)
{
    CTU_UNUSED(where);

    cpp_scan_t *self = cpp_scan_context(scan);
    cpp_ast_t *ast = map_get(self->defines, name);
    if (ast == NULL)
    {
        cpp_scan_consume(scan, name, size, false);
        // also expand the arguments
        size_t len = vector_len(args);
        for (size_t i = 0; i < len; i++)
        {
            char *arg = vector_get(args, i);
            cpp_scan_consume(scan, arg, strlen(arg), false);
        }
        return;
    }
}

void cpperror(where_t *where, void *yyscanner, scan_t *scan, const char *msg)
{
    CTU_UNUSED(yyscanner);
    CTU_UNUSED(where);

    cpp_scan_t *ctx = cpp_scan_context(scan);

    evt_scan_error(ctx->instance->logger, cpp_get_node(yyscanner, *where), msg);
}
