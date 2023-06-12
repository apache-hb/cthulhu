#include "common.h"

#include "cthulhu/mediator/interface.h"

#include "base/memory.h"
#include "base/panic.h"

#include "scan/scan.h"

#include "report/report.h"

#include "platform/error.h"
#include "stacktrace/stacktrace.h"
#include "cthulhu/hlir/init.h"

static void runtime_init(void)
{
    GLOBAL_INIT();

    stacktrace_init();
    platform_init();
    init_gmp(&globalAlloc);
    init_hlir();
}

static const language_t *add_language_extension(map_t *map, const char *ext, const language_t *lang)
{
    CTASSERT(map != NULL);
    CTASSERT(ext != NULL);
    CTASSERT(lang != NULL);

    logverbose("mapping language `%s` to `%s`", lang->id, ext);

    const language_t *old = map_get(map, ext);
    if (old != NULL)
    {
        return old;
    }

    map_set(map, ext, (void*)lang);
    return NULL;
}

mediator_t *mediator_new(const char *id, version_info_t version)
{
    CTASSERT(id != NULL);

    runtime_init();

    mediator_t *self = ctu_malloc(sizeof(mediator_t));

    self->id = id;
    self->version = version;

    return self;
}

lifetime_t *lifetime_new(mediator_t *mediator)
{
    CTASSERT(mediator != NULL);

    lifetime_t *self = ctu_malloc(sizeof(lifetime_t));

    self->parent = mediator;

    self->extensions = map_new(16);
    self->modules = map_new(64);

    return self;
}

void lifetime_add_language(lifetime_t *lifetime, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(lang != NULL);

    CTASSERT(lang->fnCreate != NULL);

    for (size_t i = 0; lang->exts[i] != NULL; i++)
    {
        const language_t *old = add_language_extension(lifetime->extensions, lang->exts[i], lang);
        CTASSERTF(old == NULL, "language `%s` registered under extension `%s` clashes with previously registered language `%s`", lang->id, lang->exts[i], old->id); // TODO: handle this
    }

    lang->fnCreate(lifetime);
}

const language_t *lifetime_get_language(lifetime_t *lifetime, const char *ext)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(ext != NULL);

    return map_get(lifetime->extensions, ext);
}

void lifetime_parse(lifetime_t *lifetime, reports_t *reports, const language_t *lang, io_t *io)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(lang != NULL);
    CTASSERT(io != NULL);

    CTASSERT(lang->fnParse != NULL);

    scan_t *scan = scan_io(reports, lang->id, io, lifetime);

    lang->fnParse(lifetime, scan);
}
