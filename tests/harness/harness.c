#include "base/memory.h"

#include "argparse/argparse.h"

#include "cthulhu/mediator/plugin.h"

#ifdef _WIN32
#   define DEFAULT_CC "cl.exe"
#   define DEFAULT_ARGS "/nologo"
#else
#   define DEFAULT_CC "gcc"
#   define DEFAULT_ARGS ""
#endif

typedef struct harness_t
{
    const char *compiler;
    const char *args;
} harness_t;

static const char *kCompilerNames[] = { "-cc", "--compiler", NULL };
static const char *kCompilerArgNames[] = { "-ca", "--compiler-args", NULL };

static ap_event_result_t on_compiler_name(ap_t *ap, const ap_param_t *param, const void *value, void *data)
{
    harness_t *harness = data;

    harness->compiler = value;

    return eEventHandled;
}

static ap_event_result_t on_compiler_args(ap_t *ap, const ap_param_t *param, const void *value, void *data)
{
    harness_t *harness = data;

    harness->args = value;

    return eEventHandled;
}

static void harness_configure(plugin_handle_t *handle, ap_t *ap)
{
    ap_group_t *group = ap_group_new(ap, "harness", "Test harness config options");
    ap_param_t *compiler = ap_add_string(group, "compiler", "compiler to use for ensuring resulting C89 is valid (default: " DEFAULT_CC ")", kCompilerNames);
    ap_param_t *args = ap_add_string(group, "compiler args", "extra compiler args for the compiler (default: " DEFAULT_ARGS ")", kCompilerArgNames);

    harness_t *harness = ctu_malloc(sizeof(harness_t));
    harness->compiler = DEFAULT_CC;
    harness->args = DEFAULT_ARGS;

    ap_event(ap, compiler, on_compiler_name, harness);
    ap_event(ap, args, on_compiler_args, harness);

    plugin_set_user(handle, harness);
}

static const plugin_t kPluginInfo = {
    .id = "harness",
    .name = "Test Harness",
    .version = {
        .license = "LGPLv3",
        .desc = "Provides a harness for testing language drivers and plugins effectively.",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(1, 1, 0)
    },

    .fnConfigure = harness_configure,
};

extern const plugin_t *harness_acquire(mediator_t *mediator)
{
    return &kPluginInfo;
}
