#include "ctu/driver.h"

#include "cthulhu/mediator/driver.h"

#include "interop/compile.h"

#include "std/vector.h"
#include "std/str.h"

#include "core/macros.h"

#include "memory/memory.h"

#include "ctu_bison.h"
#include "ctu_flex.h"

CTU_CALLBACKS(kCallbacks, ctu);

static void ctu_config(lifetime_t *lifetime, ap_t *args)
{
    CTU_UNUSED(lifetime);
    CTU_UNUSED(args);
}

static vector_t *find_mod_path(ctu_t *ast, char *fp)
{
    if (ast == NULL) { return vector_init(str_filename_noext(fp)); }

    return vector_len(ast->modspec) > 0
        ? ast->modspec
        : vector_init(str_filename_noext(fp));
}

static void ctu_parse_file(driver_t *runtime, scan_t *scan)
{
    lifetime_t *lifetime = handle_get_lifetime(runtime);

    ctu_scan_t info = { .attribs = vector_new(0) };
    scan_set(scan, BOX(info));

    ctu_t *ast = compile_scanner(scan, &kCallbacks);
    if (ast == NULL) { return; }

    CTASSERT(ast->kind == eCtuModule);

    char *fp = (char*)scan_path(scan);
    vector_t *path = find_mod_path(ast, fp);

    context_t *ctx = context_new(runtime, vector_tail(path), ast, NULL);

    add_context(lifetime, path, ctx);
}

static const char *kLangNames[] = { "ct", "ctu", "cthulhu", NULL };

const language_t kCtuModule = {
    .id = "ctu",
    .name = "Cthulhu",
    .version = {
        .license = "LGPLv3",
        .desc = "Cthulhu language driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(0, 4, 0)
    },

    .exts = kLangNames,

    .fnConfig = ctu_config,
    .fnCreate = ctu_init,

    .fnParse = ctu_parse_file,

    .fnCompilePass = {
        [eStageForwardSymbols] = ctu_forward_decls,
        [eStageCompileImports] = ctu_process_imports,
        [eStageCompileSymbols] = ctu_compile_module
    }
};
