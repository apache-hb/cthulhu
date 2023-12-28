#include "common.h"

#include "base/log.h"
#include "base/panic.h"
#include "config/config.h"
#include "core/macros.h"

#include "memory/memory.h"

#include "scan/node.h"

#include "std/map.h"
#include "std/vector.h"
#include "std/str.h"

#include <string.h>

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

// flex + bison callbacks

static void update_flags(cfg_field_t *param, const char *value, arena_t *arena)
{
    // first split `value` by `,`
    const char *start = value;
    const char *end = value;

    while (*end != '\0')
    {
        if (*end == ',')
        {
            size_t len = end - start;
            char *flag = arena_strndup(start, len, arena);

            // if the flag starts with `-` then it's a negative flag
            // otherwise it's a positive flag
            bool negate = *flag == '-';
            if (negate)
                flag++;

            cfg_set_flag(param, flag, !negate);

            start = end + 1;
        }

        end++;
    }

    // handle the last flag
    cfg_set_flag(param, start, true);
}

void ap_on_string(scan_t *scan, cfg_field_t *param, const char *value)
{
    ap_t *self = scan_get_context(scan);
    vector_t *callbacks = map_get_ptr(self->event_lookup, param);

    if (callbacks)
        apply_callbacks(scan, param, value, callbacks);

    // TODO: handle these failing
    // how do we get error messages out of here nicely?

    cfg_type_t type = cfg_get_type(param);
    switch (type) {
    case eConfigString:
        cfg_set_string(param, arena_strdup(value, self->arena));
        break;

    case eConfigEnum:
        cfg_set_enum(param, value);
        break;

    case eConfigFlags:
        update_flags(param, value, self->arena);
        break;

    default:
        NEVER("unknown config type %d", type);
    }
}

void ap_on_bool(scan_t *scan, cfg_field_t *param, bool value)
{
    ap_t *self = scan_get_context(scan);
    vector_t *callbacks = map_get_ptr(self->event_lookup, param);

    if (callbacks)
        apply_callbacks(scan, param, &value, callbacks);

    cfg_set_bool(param, value);
}

void ap_on_int(scan_t *scan, cfg_field_t *param, mpz_t value)
{
    ap_t *self = scan_get_context(scan);

    vector_t *callbacks = map_get_ptr(self->event_lookup, param);

    if (callbacks)
        apply_callbacks(scan, param, value, callbacks);

    int v = mpz_get_si(value);

    ctu_log("setting %s to %d", cfg_get_info(param)->name, v);

    // TODO: handle the int being out of range
    //       how do we get error messages out of here nicely?
    cfg_set_int(param, v);
}

void ap_on_posarg(scan_t *scan, const char *value)
{
    ap_t *self = scan_get_context(scan);
    apply_callbacks(scan, NULL, value, self->posarg_callbacks);
    vector_push(&self->posargs, (char*)value);
}

void ap_on_error(scan_t *scan, const char *message)
{
    ap_t *self = scan_get_context(scan);
    vector_push(&self->unknown, ctu_strdup(message));
}

void aperror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    CTU_UNUSED(state);
    CTU_UNUSED(where);

    ap_on_error(scan, msg);
}
