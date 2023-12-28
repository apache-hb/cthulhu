#include "config/config.h"
#include "core/macros.h"
#include "memory/memory.h"
#include "pl0/sema.h"
#include "pl0/ast.h"

#include "cthulhu/runtime/driver.h"

#include "interop/compile.h"

#include "std/str.h"

#include "pl0_bison.h" // IWYU pragma: keep
#include "pl0_flex.h" // IWYU pragma: keep

CTU_CALLBACKS(kCallbacks, pl0);

static void *pl0_preparse(driver_t *handle, scan_t *scan)
{
    CTU_UNUSED(scan);

    lifetime_t *lifetime = handle_get_lifetime(handle);
    logger_t *reports = lifetime_get_logger(lifetime);

    pl0_scan_t info = {
        .reports = reports
    };

    return ctu_memdup(&info, sizeof(pl0_scan_t));
}

static void pl0_postparse(driver_t *handle, scan_t *scan, void *tree)
{
    pl0_t *ast = tree;
    CTASSERT(ast->type == ePl0Module);

    // TODO: dedup this with pl0_forward_decls
    const char *fp = scan_path(scan);
    vector_t *path = vector_len(ast->mod) > 0
        ? ast->mod
        : vector_init(str_filename_noext(fp));

    lifetime_t *lifetime = handle_get_lifetime(handle);
    context_t *ctx = context_new(handle, vector_tail(path), ast, NULL);

    add_context(lifetime, path, ctx);
}

static const cfg_info_t kGroupInfo = {
    .name = "pl0",
    .brief = "PL/0 driver options",
};

static const char *const kExtLongArgs[] = { "pl0ext", NULL };

static const cfg_info_t kExtInfo = {
    .name = "ext",
    .brief =
        "Enable extensions for compatibility with other PL/0 compilers\n"
        "modules: enable module interop support\n"
        "ibara: enable support for ibara extensions (https://github.com/ibara/pl0c)\n"
        "orion: enable support for orion extensions (https://github.com/oriontransfer/PL0-Language-Tools)\n"
        "tianbo: enable support for tianbo extensions (https://github.com/haotianbo/PL0-Compiler)",
    .long_args = kExtLongArgs,
};

typedef enum pl0_extension_t
{
    eExtNone        = 0,
    eExtModules     = (1 << 0),
    eExtIbara       = (1 << 1),
    eExtOrion       = (1 << 2),
    eExtTianbo      = (1 << 3),
} pl0_extension_t;

static const cfg_choice_t kExtChoices[] = {
    { .text = "none", .value = eExtNone },
    { .text = "modules", .value = eExtModules },
    { .text = "ibara", .value = eExtIbara },
    { .text = "orion", .value = eExtOrion },
    { .text = "tianbo", .value = eExtTianbo },
};

#define EXT_COUNT (sizeof(kExtChoices) / sizeof(cfg_choice_t))

static config_t *pl0_config(driver_t *handle, config_t *root)
{
    CTU_UNUSED(handle);

    config_t *group = config_group(root, &kGroupInfo);

    cfg_flags_t flags = {
        .options = kExtChoices,
        .count = EXT_COUNT,
        .initial = eExtNone
    };

    config_flags(group, &kExtInfo, flags);

    return group;
}

static const char *const kLangNames[] = { "pl", "pl0", NULL };

const language_t kPl0Module = {
    .id = "pl0",
    .name = "PL/0",
    .version = {
        .license = "LGPLv3",
        .desc = "PL/0 language driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(2, 3, 2)
    },

    .exts = kLangNames,

    .fn_config = pl0_config,

    .fn_create = pl0_init,

    .fn_preparse = pl0_preparse,
    .fn_postparse = pl0_postparse,
    .parse_callbacks = &kCallbacks,

    .fn_compile_passes = {
        [eStageForwardSymbols] = pl0_forward_decls,
        [eStageCompileImports] = pl0_process_imports,
        [eStageCompileSymbols] = pl0_compile_module
    }
};
