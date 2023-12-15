#include "editor/compile.hpp"

#include "cthulhu/mediator/interface.h"

#include "io/io.h"

#include "report/report.h"

#include "std/str.h"

#include "std/vector.h"

#include "support/langs.h"

using namespace ed;

/// @brief parse a source file
///
/// @param index the index of the source file to parse
///
/// @return nullptr on success, otherwise an error message
char *CompileInfo::parse_source(size_t index)
{
    const char *path = sources.get(index);

    const char *ext = str_ext(path);
    if (ext == nullptr)
    {
        return format("could not determine file extension for '%s'", path);
    }

    const language_t *lang = lifetime_get_language(lifetime, ext);
    if (lang == nullptr)
    {
        const char *basepath = str_filename(path);
        return format("could not find language for `%s` by extension `%s`", basepath, ext);
    }

    io_t *io = io_file(path, eAccessRead);
    if (os_error_t err = io_error(io); err != 0)
    {
        return format("could not open file '%s' (os error: %s)", path, os_error_string(err));
    }

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
        lifetime_add_language(lifetime, &langs.langs[i]);
    }
}

void CompileInfo::init_reports()
{
    reports = lifetime_get_reports(lifetime);
}

/// @brief check if there are any reports
///
/// @retval true if there are no reports
/// @retval false if there are reports
bool CompileInfo::check_reports() const
{
    return vector_len(reports->messages) == 0;
}
