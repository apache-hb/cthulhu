#include "format/version.h"
#include "base/panic.h"

#include "io/io.h"

void print_version(print_version_t options)
{
    CTASSERT(options.name != NULL);

    print_options_t base = options.options;

    version_info_t version = options.version;
    int major = VERSION_MAJOR(version.version);
    int minor = VERSION_MINOR(version.version);
    int patch = VERSION_PATCH(version.version);

    io_printf(base.io, "%s %d.%d.%d\n", options.name, major, minor, patch);
    io_printf(base.io, "written by %s and licensed under %s\n", version.author, version.license);
    io_printf(base.io, "%s\n", version.desc);
}
