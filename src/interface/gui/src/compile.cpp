#include "editor/compile.hpp"

#include "cthulhu/runtime/interface.h"

#include "notify/notify.h"

#include "std/str.h"

#include "std/typed/vector.h"

#include "support/langs.h"

using namespace ed;

/// @brief parse a source file
///
/// @param index the index of the source file to parse
///
/// @return nullptr on success, otherwise an error message
char *CompileInfo::parse_source(size_t index)
{
    const char *path = sources.get_path(index);

    char *ext = str_ext(path, &global);
    if (ext == nullptr)
    {
        return str_format(&global, "could not determine file extension for '%s'", path);
    }

    const language_t *lang = lifetime_get_language(lifetime, ext);
    if (lang == nullptr)
    {
        const char *basepath = str_filename(path, &global);
        return str_format(&global, "could not find language for `%s` by extension `%s`", basepath, ext);
    }

    io_t *io = sources.get_io(index);

    lifetime_parse(lifetime, lang, io);
    return nullptr;
}

void CompileInfo::init_alloc()
{
    global.install_global();
    gmp.install_gmp();
}

void CompileInfo::init_lifetime()
{
    lifetime = lifetime_new(mediator, &global);

    langs_t langs = get_langs();
    for (size_t i = 0; i < langs.size; i++)
    {
        lifetime_add_language(lifetime, langs.langs[i]);
    }
}

void CompileInfo::init_reports()
{
    reports = lifetime_get_logger(lifetime);
}

void CompileInfo::init()
{
    if (setup) return;
    setup = true;

    init_alloc();
    init_lifetime();
    init_reports();
}

/// @brief check if there are any reports
///
/// @retval true if there are no reports
/// @retval false if there are reports
bool CompileInfo::check_reports() const
{
    typevec_t *events = logger_get_events(reports);
    return typevec_len(events) == 0;
}
