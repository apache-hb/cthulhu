// SPDX-License-Identifier: LGPL-3.0-or-later
#include "demangle/microsoft.h"

#include "core/version_def.h"
#include "setup/setup.h"

static const version_info_t kToolVersion = {
    .license = "LGPL-3.0-or-later",
    .desc = "Demangler tool",
    .author = "Elliot Haisley",
    .version = CT_NEW_VERSION(0, 1, 0),
};

typedef struct tool_t
{
    setup_options_t options;
} tool_t;

int main(int argc, const char **argv)
{
    setup_default(NULL);
}
