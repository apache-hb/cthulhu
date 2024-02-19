#include "base/util.h"
#include "common.h"

#include "config/config.h"
#include "core/macros.h"
#include "arena/arena.h"
#include "std/typed/vector.h"
#include "std/vector.h"
#include "std/str.h"
#include "std/map.h"

#include "io/io.h"

#include "interop/compile.h"

#include "ap_bison.h" // IWYU pragma: keep
#include "ap_flex.h" // IWYU pragma: keep

CT_CALLBACKS(kCallbacks, ap);

static void push_single_arg(typevec_t *vec, const char *arg)
{
    CTASSERT(vec != NULL);
    CTASSERT(arg != NULL);

    // TODO: this is a bit of a bodge, i should think of a better
    // way to extract flags from the command line
    bool is_flag = arg[0] == '-' || arg[0] == '/';
    size_t len = ctu_strlen(arg);

    if (is_flag)
    {
        size_t idx = 0;
        for (size_t i = 0; i < len; i++)
        {
            if (arg[i] == ':' || arg[i] == '=')
            {
                idx = i + 1;
                break;
            }
        }

        // we found an assignment so we need to split the string
        if (idx > 0)
        {
            typevec_append(vec, arg, idx);

            typevec_append(vec, " \"", 2);
            typevec_append(vec, arg + idx, len - idx);
            typevec_push(vec, "\"");
            return;
        }
        else
        {
            // otherwise just paste it in
            typevec_append(vec, arg, len);
            return;
        }
    }

    // if its not a flag then we just need to wrap it in quotes
    typevec_push(vec, "\"");
    typevec_append(vec, arg, len);
    typevec_push(vec, "\"");
}

static char *join_args(int argc, const char **argv, arena_t *arena)
{
    typevec_t *vec = typevec_new(sizeof(char), argc - 1, arena);
    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];

        push_single_arg(vec, arg);

        typevec_push(vec, " ");
    }

    typevec_push(vec, "\0");

    return typevec_data(vec);
}

int ap_parse_common(ap_t *self, const char *text)
{
    CTASSERT(self != NULL);
    CTASSERT(text != NULL);

    io_t *io = io_string("<command>", text, self->arena);
    scan_t *scan = scan_io("args", io, self->arena);

    scan_set_context(scan, self);
    parse_result_t result = scan_buffer(scan, &kCallbacks);

    return result.result == eParseOk ? CT_EXIT_OK : CT_EXIT_ERROR;
}

int ap_parse_args(ap_t *self, int argc, const char **argv)
{
    char *args = join_args(argc, argv, self->arena);
    return ap_parse_common(self, args);
}

int ap_parse(ap_t *self, const char *str)
{
    return ap_parse_common(self, str);
}

static int get_option_type(const cfg_field_t *field)
{
    switch (cfg_get_type(field))
    {
    case eConfigBool: return AP_BOOL_OPTION;
    case eConfigInt: return AP_INT_OPTION;

    case eConfigEnum:
    case eConfigFlags:
    case eConfigString:
    case eConfigVector:
        return AP_STRING_OPTION;

    default:
        NEVER("unknown option type %d", cfg_get_type(field));
    }
}

static const char *get_lookup_name(const char *name, bool *negate, arena_t *arena)
{
    CTASSERT(name != NULL);
    CTASSERT(negate != NULL);

    // if the name ends with `-` then its a negation
    // so we need to strip that off

    *negate = false;
    size_t len = ctu_strlen(name);

    // something is wrong if the name is empty
    CTASSERT(len > 0);

    if (name[len - 1] == '-')
    {
        *negate = true;
        len -= 1;
        return arena_strndup(name, len, arena);
    }

    return name;
}

int ap_get_opt(scan_t *scan, const char *name, ap_field_t *param, char **value)
{
    CTASSERT(scan != NULL);
    CTASSERT(name != NULL);
    CTASSERT(param != NULL);

    ap_t *self = scan_get_context(scan);

    // if the name ends with `-` then its a negation
    // so we need to strip that off

    bool negate = false;
    const char *lookup = get_lookup_name(name, &negate, self->arena);

    cfg_field_t *result = map_get(self->name_lookup, lookup);
    if (result == NULL)
    {
        *value = arena_strdup(name, self->arena);
        vector_push(&self->unknown, *value);
        return AP_ERROR;
    }

    ap_field_t out = {
        .field = result,
        .negate = negate
    };

    self->count += 1;
    *param = out;
    cfg_type_t type = get_option_type(result);

    if (negate && cfg_get_type(result) != eConfigBool)
    {
        ap_add_error(self, "cannot negate non-boolean flag: %s", name);
    }

    return type;
}
