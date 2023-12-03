#include "argparse/commands.h"
#include "ap_common.h"

#include "base/panic.h"

#include "std/vector.h"

#include <ctu_config.h>

#include <stdio.h>

static const char *const kParamTypeNames[] = {
    [eParamBool] = "boolean",
    [eParamString] = "string",
    [eParamInt] = "integer",
};

void ap_print_help_header(const ap_t *ap, const char *name)
{
    CTASSERT(ap != NULL);

    printf("usage: %s [options] [arguments] [sources...]\n", name);
    version_info_t info = {
        .license = "LGPLv3",
        .desc = "common compiler mediator",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(CTU_MAJOR, CTU_MINOR, CTU_PATCH)
    };

    ap_print_version_info(info, "cthulhu");
}

void ap_print_version_info(version_info_t info, const char *name)
{
    printf(" | %s: %s (%" PRI_VERSION ".%" PRI_VERSION ".%" PRI_VERSION ")\n", name, info.desc,
           VERSION_MAJOR(info.version), VERSION_MINOR(info.version), VERSION_PATCH(info.version));
    printf(" | license: %s\n", info.license);
    printf(" | author: %s\n", info.author);
}

void ap_print_help_body(const ap_t *ap, const char *name)
{
    printf("========================================\n");
    printf("parser commands are executed in the order they are parsed unless specified otherwise.\n");
    printf("this means %s -a -b may not behave the same as %s -b -a\n", name, name);
    printf("keep this in mind when writing commands.\n");
    printf("========================================\n");

    const vector_t *groups = ap_get_groups(ap);

    size_t groupCount = vector_len(groups);
    for (size_t i = 0; i < groupCount; i++)
    {
        const ap_group_t *group = vector_get(groups, i);
        const vector_t *params = ap_get_params(group);
        size_t paramCount = vector_len(params);

        printf("\n%s - %s (%zu commands)\n", group->name, group->desc, paramCount);

        for (size_t j = 0; j < paramCount; j++)
        {
            printf("  [");
            const ap_param_t *param = vector_get(params, j);
            for (size_t n = 0; param->names[n]; n++)
            {
                if (n > 0)
                    printf(", ");
                printf("%s", param->names[n]);
            }

            printf("]: %s\n", param->desc);

            printf("    type: %s\n", kParamTypeNames[param->type]);
        }
    }
}

void ap_version(const ap_t *ap)
{
    printf("%s\n", ap->desc);
    printf("interface version: %" PRI_VERSION ".%" PRI_VERSION ".%" PRI_VERSION "\n", VERSION_MAJOR(ap->version),
           VERSION_MINOR(ap->version), VERSION_PATCH(ap->version));
    printf("cthulhu version: %d.%d.%d\n", CTU_MAJOR, CTU_MINOR, CTU_PATCH);
}
