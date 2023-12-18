#include "notify/colour.h"
#include "base/panic.h"

const char *colour_get(const text_colour_t *colours, colour_t idx)
{
    CTASSERT(colours != NULL);
    CTASSERT(idx < eColourCount);

    return colours->colours[idx];
}

const char *colour_reset(const text_colour_t *colours)
{
    CTASSERT(colours != NULL);

    return colours->reset;
}
