#include "editor/compile.hpp"

#include "cthulhu/broker/broker.h"

#include "notify/notify.h"

#include "std/str.h"

#include "std/typed/vector.h"

#include "support/support.h"

using namespace ed;

static constexpr frontend_t kEditorInfo = {
    .info = {
        .id = "frontend-editor",
        .name = "Editor GUI",
        .version = {
            .license = "GPLv3",
            .desc = "ImGui based frontend for the Cthulhu Compiler Collection",
            .author = "Elliot Haisley",
            .version = CT_NEW_VERSION(1, 0, 2),
        }
    }
};

CompileInfo::CompileInfo(loader_t *loader, const char *name)
    : name(name)
    , broker(broker_new(&kEditorInfo, &global))
    , loader(loader)
    , sources(&global)
{ }

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

    language_runtime_t *lang = support_get_lang(support, ext);
    if (lang == nullptr)
    {
        const char *basepath = str_filename(path, &global);
        return str_format(&global, "could not find language for `%s` by extension `%s`", basepath, ext);
    }

    io_t *io = sources.get_io(index);

    broker_parse(lang, io);
    return nullptr;
}

void CompileInfo::init_alloc()
{
    global.install_global();
    gmp.install_gmp();
}

void CompileInfo::init_support()
{
    support = support_new(broker, loader, &global);
}

void CompileInfo::init()
{
    if (setup) return;
    setup = true;

    init_alloc();
    init_support();
}

/// @brief check if there are any reports
///
/// @retval true if there are no reports
/// @retval false if there are reports
bool CompileInfo::check_reports() const
{
    logger_t *logger = broker_get_logger(broker);
    typevec_t *events = logger_get_events(logger);
    return typevec_len(events) == 0;
}
