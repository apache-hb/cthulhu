#include "mark.h"

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"

#include "base/memory.h"
#include "base/panic.h"

#include "std/map.h"

#include <stdio.h>

typedef enum
{
    eMarkVariant,
    eMarkTotal
} mark_tag_t;

typedef struct
{
    mark_tag_t tag;
    union {
        struct {
            const hlir_t *defaultCase;
            map_t *cases;
        };
    };
} mark_data_t;

static mark_data_t *mark_data_new(mark_tag_t tag)
{
    mark_data_t *self = ctu_malloc(sizeof(mark_data_t));
    self->tag = tag;
    return self;
}

static mark_data_t *mark_variant_new(size_t size)
{
    mark_data_t *self = mark_data_new(eMarkVariant);
    self->defaultCase = NULL;
    self->cases = map_optimal(size);
    return self;
}

void user_mark_variant(hlir_t *self)
{
    CTASSERT(self != NULL);

    self->user = mark_variant_new(64);
}

void user_add_variant_case(hlir_t *self, const char *name, const hlir_t *it)
{
    map_t *map = user_variant_cases(self);
    CTASSERT(map != NULL);

    map_set(map, name, (hlir_t*)it);
}

const hlir_t *user_set_variant_default(hlir_t *self, const hlir_t *it)
{
    CTASSERT(self != NULL);
    mark_data_t *data = self->user;
    CTASSERT(data != NULL && data->tag == eMarkVariant);

    printf("set-default: %p %s\n", self, hlir_kind_to_string(get_hlir_kind(self)));

    const hlir_t *old = data->defaultCase;
    data->defaultCase = it;
    return old;
}

map_t *user_variant_cases(const hlir_t *self)
{
    if (self == NULL) { return NULL; }
    if (self->user == NULL) { return NULL; }

    mark_data_t *data = self->user;
    if (data->tag != eMarkVariant) { return NULL; }

    return data->cases;
}

const hlir_t *user_variant_default_case(const hlir_t *self)
{
    printf("get-default: %p %s\n", self, hlir_kind_to_string(get_hlir_kind(self)));
    if (self == NULL) { return NULL; }
    if (self->user == NULL) { return NULL; }

    mark_data_t *data = self->user;
    if (data->tag != eMarkVariant) { return NULL; }

    return data->defaultCase;
}
