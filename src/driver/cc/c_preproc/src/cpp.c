#include "cpp/cpp.h"

#include "base/log.h"
#include "base/panic.h"
#include "core/text.h"

#include "io/io.h"
#include "memory/arena.h"
#include "scan/scan.h"

#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"
#include <string.h>

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
        .defines = map_optimal_arena(64, arena),
        .files = map_optimal_arena(64, arena),
    };

    return instance;
}

void cpp_add_include_dir(cpp_instance_t *instance, const char *path)
{
    CTASSERT(instance != NULL);
    CTASSERT(path != NULL);

    vector_push(&instance->include_directories, (char*)path);
}

typedef enum cpp_state_t
{
    /// initial state
    eStateInitial,

    /// actual source code
    eStateSource,

    /// beginning of a directive, found `#` and expecting a directive name
    eStateDirective,

    // inside a directive name
    eStateDirectiveName,

    /// found the name, now looking for arguments
    eStateDirectiveExtra,
} cpp_state_t;

/// @brief state machine
typedef struct cpp_machine_t
{
    /// @brief global instance
    cpp_instance_t *instance;

    /// @brief resulting scan text
    typevec_t *result;

    const char *path;

    /// @brief current text being processed
    text_view_t text;

    /// @brief current offset
    size_t offset;

    /// @brief current state
    cpp_state_t state;

    /// @brief current directive name
    text_view_t directive_name;

    /// @brief current directive arguments
    text_view_t directive_extra;

    bool pragma_once;
    size_t skip_depth;
} cpp_machine_t;

static bool machine_step(cpp_machine_t *machine);

static cpp_machine_t machine_init(cpp_instance_t *instance, scan_t *source)
{
    text_view_t text = scan_source(source);
    typevec_t *result = typevec_new(sizeof(char), text.size, instance->arena);

    cpp_machine_t machine = {
        .instance = instance,
        .result = result,
        .path = scan_path(source),
        .text = text,
        .offset = 0,
        .state = eStateInitial,
        .directive_name = { 0 },
        .directive_extra = { 0 },
    };

    return machine;
}

static cpp_machine_t machine_child(cpp_machine_t *parent, io_t *io)
{
    scan_t *scan = scan_io("C", io, parent->instance->arena);
    scan_set_context(scan, parent);

    text_view_t text = scan_source(scan);

    cpp_machine_t machine = {
        .instance = parent->instance,
        .result = parent->result,
        .path = scan_path(scan),
        .text = text,
        .offset = 0,
        .state = eStateInitial,
        .directive_name = { 0 },
        .directive_extra = { 0 },
    };

    return machine;
}

static void machine_rewind(cpp_machine_t *machine, size_t offset)
{
    machine->offset = offset;
}

static bool match_ahead(cpp_machine_t *machine, const char *str)
{
    text_view_t text = machine->text;
    size_t i = machine->offset;

    size_t len = strlen(str);

    bool matched = strncmp(text.text + i, str, len) == 0;

    return matched;
}

static bool consume_ahead(cpp_machine_t *machine, const char *str)
{
    text_view_t text = machine->text;
    size_t i = machine->offset;

    size_t len = strlen(str);

    bool matched = strncmp(text.text + i, str, len) == 0;

    if (matched)
    {
        machine->offset += len;
    }

    return matched;
}

static void process_directive_name(cpp_machine_t *machine)
{
    text_view_t directive = machine->directive_name;
    ctu_log("directive name: `%.*s`", (int)directive.size, directive.text);
}

static bool try_include(cpp_machine_t *machine, const char *dir, const char *trimmed)
{
    char *path = str_format(machine->instance->arena, "%s" NATIVE_PATH_SEPARATOR "%s", dir, trimmed);
    ctu_log("Trying include `%s`", path);

    cpp_machine_t *existing = map_get(machine->instance->files, path);
    if (existing != NULL)
    {
        ctu_log("Found existing include `%s`", path);

        if (machine->pragma_once)
        {
            return true;
        }

        machine_rewind(existing, 0);
        while (machine_step(existing)) { }
        return true;
    }

    io_t *io = io_file_arena(path, eAccessRead | eAccessText, machine->instance->arena);
    if (io_error(io) == 0)
    {
        ctu_log("Found new include `%s`", path);

        cpp_machine_t child = machine_child(machine, io);
        cpp_machine_t *ptr = arena_memdup(&child, sizeof(cpp_machine_t), machine->instance->arena);
        map_set(machine->instance->files, path, ptr);

        while (machine_step(&child)) { }
        return true;
    }

    return false;
}

static void process_include(cpp_machine_t *machine)
{
    ctu_log("include: `%.*s` (depth %zu)", (int)machine->directive_extra.size, machine->directive_extra.text, machine->skip_depth);
    if (machine->skip_depth != 0)
        return;

    cpp_instance_t *instance = machine->instance;
    text_view_t extra = machine->directive_extra;
    const char *trimmed = arena_strndup(extra.text + 1, extra.size - 2, instance->arena);

    char *local = str_directory(instance->arena, machine->path);
    if (try_include(machine, local, trimmed))
        return;

    size_t len = vector_len(instance->include_directories);
    for (size_t i = 0; i < len; i++)
    {
        const char *dir = vector_get(instance->include_directories, i);

        if (try_include(machine, dir, trimmed))
        {
            return;
        }
    }
}

static void process_define(cpp_machine_t *machine)
{
    if (machine->skip_depth > 0)
        return;

    cpp_instance_t *instance = machine->instance;
    text_view_t extra = machine->directive_extra;

    // find the first space in the extra
    size_t i = 0;
    for (; i < extra.size; i++)
    {
        if (extra.text[i] == ' ')
            break;
    }

    // everything before the space is the name
    const char *name = arena_strndup(extra.text, i, instance->arena);

    // everything after the space is the value
    size_t value_len = extra.size - i;
    if (value_len == 0)
    {
        ctu_log("define: `%s`", name);

        cpp_define_t *define = cpp_define_new(instance->arena, name, "");
        map_set(instance->defines, name, define);
        return;
    }

    const char *value = arena_strndup(extra.text + i + 1, value_len - 1, instance->arena);

    ctu_log("define: `%s` `%s`", name, value);

    cpp_define_t *define = cpp_define_new(instance->arena, name, value);
    map_set(instance->defines, name, define);
}

static void process_undef(cpp_machine_t *machine)
{
    if (machine->skip_depth > 0)
        return;

    cpp_instance_t *instance = machine->instance;
    text_view_t extra = machine->directive_extra;

    const char *name = arena_strndup(extra.text, extra.size, instance->arena);

    ctu_log("undef: `%s`", name);

    map_delete(instance->defines, name);
}

static void process_pragma(cpp_machine_t *machine)
{
    text_view_t extra = machine->directive_extra;
    if (strncmp(extra.text, "once", extra.size) == 0)
    {
        machine->pragma_once = true;
    }

    ctu_log("pragma: `%.*s`", (int)extra.size, extra.text);
}

static void process_ifdef(cpp_machine_t *machine)
{
    if (machine->skip_depth != 0)
        return;

    text_view_t extra = machine->directive_extra;
    const char *name = arena_strndup(extra.text, extra.size, machine->instance->arena);

    if (map_get(machine->instance->defines, name) == NULL)
    {
        machine->skip_depth += 1;
    }

    ctu_log("ifdef: `%s`", name);
}

static void process_ifndef(cpp_machine_t *machine)
{
    if (machine->skip_depth != 0)
        return;

    text_view_t extra = machine->directive_extra;
    const char *name = arena_strndup(extra.text, extra.size, machine->instance->arena);

    if (map_get(machine->instance->defines, name) != NULL)
    {
        machine->skip_depth += 1;
    }

    ctu_log("ifndef: `%s`", name);
}

static void process_if(cpp_machine_t *machine)
{
    if (machine->skip_depth != 0)
        return;

}

static void process_elif(cpp_machine_t *machine)
{
    if (machine->skip_depth != 0)
        return;

}

static void process_endif(cpp_machine_t *machine)
{
    if (machine->skip_depth > 0)
        machine->skip_depth -= 1;

    ctu_log("endif");
}

static void process_directive_extra(cpp_machine_t *machine)
{
    text_view_t directive = machine->directive_name;
    text_view_t extra = machine->directive_extra;

    if (strncmp(directive.text, "include", directive.size) == 0)
    {
        process_include(machine);
        return;
    }
    else if (strncmp(directive.text, "define", directive.size) == 0)
    {
        process_define(machine);
        return;
    }
    else if (strncmp(directive.text, "undef", directive.size) == 0)
    {
        process_undef(machine);
        return;
    }
    else if (strncmp(directive.text, "if", directive.size) == 0)
    {
        process_if(machine);
        return;
    }
    else if (strncmp(directive.text, "elif", directive.size) == 0)
    {
        process_elif(machine);
        return;
    }
    else if (strncmp(directive.text, "ifdef", directive.size) == 0)
    {
        process_ifdef(machine);
        return;
    }
    else if (strncmp(directive.text, "ifndef", directive.size) == 0)
    {
        process_ifndef(machine);
        return;
    }
    else if (strncmp(directive.text, "endif", directive.size) == 0)
    {
        process_endif(machine);
        return;
    }
    else if (strncmp(directive.text, "pragma", directive.size) == 0)
    {
        process_pragma(machine);
        return;
    }

    ctu_log("directive: `%.*s` `%.*s`", (int)directive.size, directive.text, (int)extra.size, extra.text);
}

static void enter_state(cpp_machine_t *machine, cpp_state_t state)
{
    cpp_state_t old = machine->state;
    machine->state = state;

    if (state != eStateInitial)
        return;

    switch (old)
    {
    case eStateDirective:
    case eStateDirectiveName:
        process_directive_name(machine);
        break;

    case eStateDirectiveExtra:
        process_directive_extra(machine);
        break;

    default:
        break;
    }

    if (state == eStateInitial)
    {
        machine->directive_name.text = NULL;
        machine->directive_name.size = 0;
        machine->directive_extra.text = NULL;
        machine->directive_extra.size = 0;
    }
}

static char peek_letter(cpp_machine_t *machine)
{
    if (machine->offset >= machine->text.size)
    {
        return '\0';
    }

    return machine->text.text[machine->offset];
}

static bool consume_letter(cpp_machine_t *machine, char letter)
{
    if (peek_letter(machine) == letter)
    {
        machine->offset += 1;
        return true;
    }

    return false;
}

#define STR_WHITESPACE_NOT_NEWLINE " \t\r"

static bool skip_whitespace_not_newline(cpp_machine_t *machine, bool preserve)
{
    size_t start = machine->offset;
    while (char_is_any_of(peek_letter(machine), STR_WHITESPACE_NOT_NEWLINE))
    {
        machine->offset += 1;
    }

    // append the skipped whitespace
    if (machine->offset > start)
    {
        if (preserve)
            typevec_append(machine->result, machine->text.text + start, machine->offset - start - 1);
        return true;
    }
    return false;
}

static bool skip_whitespace(cpp_machine_t *machine, bool preserve)
{
    size_t start = machine->offset;
    while (char_is_any_of(peek_letter(machine), STR_WHITESPACE))
    {
        machine->offset += 1;
    }

    // append the skipped whitespace
    if (machine->offset > start)
    {
        if (preserve)
            typevec_append(machine->result, machine->text.text + start, machine->offset - start - 1);
        return true;
    }
    return false;
}

static void skip_line_comment(cpp_machine_t *machine)
{
    size_t start = machine->offset;
    while (machine->offset < machine->text.size && peek_letter(machine) != '\n')
    {
        machine->offset += 1;
    }

    // append the skipped whitespace
    if (machine->offset > start)
    {
        typevec_append(machine->result, machine->text.text + start, machine->offset - start - 1);
    }
}

static void skip_block_comment(cpp_machine_t *machine)
{
    size_t start = machine->offset;
    while (machine->offset < machine->text.size && !match_ahead(machine, "*/"))
    {
        machine->offset += 1;
    }

    // append the skipped whitespace
    if (machine->offset > start)
    {
        typevec_append(machine->result, machine->text.text + start, machine->offset - start - 1);
    }
}

static void step_initial(cpp_machine_t *machine)
{
    skip_whitespace(machine, true);

    if (consume_ahead(machine, "#"))
    {
        enter_state(machine, eStateDirective);
        machine->offset += 1;
    }
    else if (match_ahead(machine, "//"))
    {
        skip_line_comment(machine);
    }
    else if (match_ahead(machine, "/*"))
    {
        skip_block_comment(machine);
    }
    else
    {
        enter_state(machine, eStateSource);
    }
}

static void step_source(cpp_machine_t *machine)
{
    // just skip until a newline
    size_t start = machine->offset;
    while (machine->offset < machine->text.size && peek_letter(machine) != '\n')
    {
        machine->offset += 1;
    }

    // append the skipped source code
    if (machine->offset > start)
    {
        typevec_append(machine->result, machine->text.text + start, machine->offset - start);
    }

    enter_state(machine, eStateInitial);
}

static void step_directive(cpp_machine_t *machine)
{
    // skip whitespace until we find the directive name
    bool whitespace = skip_whitespace(machine, false);

    // if we hit a newline then we're done
    if (consume_letter(machine, '\n'))
    {
        enter_state(machine, eStateInitial);
        return;
    }

    // otherwise we're in the directive name
    enter_state(machine, eStateDirectiveName);
    machine->directive_name.text = machine->text.text + machine->offset - (whitespace ? 0 : 1);
    machine->directive_name.size = 0;
}

static void step_directive_name(cpp_machine_t *machine)
{
    // collect the directive name
    while (machine->offset < machine->text.size && !char_is_any_of(peek_letter(machine), STR_WHITESPACE))
    {
        machine->offset += 1;
    }

    // append the directive name
    machine->directive_name.size = machine->offset - (machine->directive_name.text - machine->text.text);

    enter_state(machine, eStateDirectiveExtra);
}

static void step_directive_extra(cpp_machine_t *machine)
{
    // skip extra whitespace
    skip_whitespace_not_newline(machine, false);

    // if we hit a newline then we're done
    if (consume_letter(machine, '\n'))
    {
        enter_state(machine, eStateInitial);
        return;
    }

    // otherwise we're in the directive name
    enter_state(machine, eStateDirectiveExtra);
    machine->directive_extra.text = machine->text.text + machine->offset;
    machine->directive_extra.size = 0;

    // collect the directive extra

    while (machine->offset < machine->text.size && peek_letter(machine) != '\n')
    {
        machine->offset += 1;
    }

    // append the dextra
    machine->directive_extra.size = machine->offset - (machine->directive_extra.text - machine->text.text) - 1;

    // back to the initial state
    enter_state(machine, eStateInitial);
}

static bool machine_step(cpp_machine_t *machine)
{
    if (machine->offset >= machine->text.size)
    {
        return false;
    }

    switch (machine->state)
    {
    case eStateInitial:
        step_initial(machine);
        break;

    case eStateSource:
        step_source(machine);
        break;

    case eStateDirective:
        step_directive(machine);
        break;

    case eStateDirectiveName:
        step_directive_name(machine);
        break;

    case eStateDirectiveExtra:
        step_directive_extra(machine);
        break;

    default:
        machine->offset += 1;
        enter_state(machine, eStateInitial);
        break;
    }

    return true;
}

io_t *cpp_process_file(cpp_instance_t *instance, scan_t *source)
{
    cpp_machine_t machine = machine_init(instance, source);

    // run the machine until it's done
    while (machine_step(&machine)) { }

    const char *text = typevec_data(machine.result);
    size_t size = typevec_len(machine.result);

    io_t *io = io_view("out", text, size, instance->arena);

    ctu_log("Processed `%s`", io_name(io));

    return io;
}
