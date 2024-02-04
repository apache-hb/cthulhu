#include "common.h"

#include "base/panic.h"
#include "config/config.h"
#include "core/macros.h"

#include "arena/arena.h"

#include "scan/scan.h"
#include "std/map.h"
#include "std/vector.h"
#include "std/str.h"

#include <string.h>

typedef struct where_t where_t;

// internals

static void apply_callbacks(scan_t *scan, const cfg_field_t *param, const void *value, vector_t *all)
{
    CTASSERT(scan != NULL);
    CTASSERT(all != NULL);
    CTASSERT(value != NULL);

    ap_t *self = scan_get_context(scan);
    size_t len = vector_len(all);

    for (size_t i = 0; i < len; i++)
    {
        ap_callback_t *cb = vector_get(all, i);
        ap_event_result_t result = cb->callback(self, param, value, cb->data);

        if (result == eEventHandled)
        {
            return;
        }
    }
}

void ap_add_error(ap_t *self, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *error = str_vformat(self->arena, fmt, args);
    va_end(args);

    vector_push(&self->errors, error);
}

// flex + bison callbacks

static void update_flags(cfg_field_t *param, const char *value, ap_t *self)
{
    // first split `value` by `,`
    const char *start = value;
    const char *end = value;

    while (*end != '\0')
    {
        if (*end == ',')
        {
            size_t len = end - start;
            char *flag = arena_strndup(start, len, self->arena);

            // if the flag starts with `-` then it's a negative flag
            // otherwise it's a positive flag
            bool negate = *flag == '-';
            if (negate)
                flag++;

            if (!cfg_set_flag(param, flag, !negate))
            {
                ap_add_error(self, "unknown flag value: %s", flag);
            }

            start = end + 1;
        }

        end++;
    }

    // handle the last flag
    bool negate = *start == '-';
    if (negate)
        start++;

    if (!cfg_set_flag(param, start, !negate))
    {
        ap_add_error(self, "unknown flag value: %s", start);
    }
}

void ap_on_string(scan_t *scan, cfg_field_t *param, char *value)
{
    ap_t *self = scan_get_context(scan);
    vector_t *callbacks = map_get(self->event_lookup, param);

    if (callbacks)
        apply_callbacks(scan, param, value, callbacks);

    cfg_type_t type = cfg_get_type(param);
    switch (type) {
    case eConfigString:
        cfg_set_string(param, arena_strdup(value, self->arena));
        break;

    case eConfigVector:
        cfg_vector_push(param, arena_strdup(value, self->arena));
        break;

    case eConfigEnum:
        // TODO: handle failure
        cfg_set_enum(param, value);
        break;

    case eConfigFlags:
        update_flags(param, value, self);
        break;

    default:
        NEVER("unknown config type %d", type);
    }
}

void ap_on_bool(scan_t *scan, cfg_field_t *param, bool value)
{
    ap_t *self = scan_get_context(scan);
    vector_t *callbacks = map_get(self->event_lookup, param);

    if (callbacks)
        apply_callbacks(scan, param, &value, callbacks);

    cfg_set_bool(param, value);
}

void ap_on_int(scan_t *scan, cfg_field_t *param, mpz_t value)
{
    ap_t *self = scan_get_context(scan);

    vector_t *callbacks = map_get(self->event_lookup, param);

    if (callbacks)
        apply_callbacks(scan, param, value, callbacks);

    int v = mpz_get_si(value);

    if (!cfg_set_int(param, v))
    {
        ap_add_error(self, "value out of range: %d", v);
    }
}

void ap_on_posarg(scan_t *scan, char *value)
{
    ap_t *self = scan_get_context(scan);
    apply_callbacks(scan, NULL, value, self->posarg_callbacks);
    vector_push(&self->posargs, (char*)value);
}

void ap_on_invalid(scan_t *scan, char *value)
{
    ap_t *self = scan_get_context(scan);

    ap_add_error(self, "invalid ascii character: `%s`", value);
}

void aperror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CT_UNUSED(state);
    CT_UNUSED(where);

    ap_t *self = scan_get_context(scan);

    ap_add_error(self, "%s", msg);
}
