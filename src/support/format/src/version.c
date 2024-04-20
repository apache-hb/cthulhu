// SPDX-License-Identifier: LGPL-3.0-only

#include "format/version.h"
#include "base/panic.h"

#include "io/io.h"

USE_DECL
void print_version(print_version_t config, version_info_t version, const char *name)
{
    CTASSERT(name != NULL);

    print_options_t base = config.options;

    int major = CT_VERSION_MAJOR(version.version);
    int minor = CT_VERSION_MINOR(version.version);
    int patch = CT_VERSION_PATCH(version.version);

    io_printf(base.io, "%s %d.%d.%d\n", name, major, minor, patch);
    io_printf(base.io, "written by %s and licensed under %s\n", version.author, version.license);
    io_printf(base.io, "%s\n", version.desc);
}
