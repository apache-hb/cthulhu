#include "sema/sema.h"
#include "sema/config.h"

#include "scan/compile.h"

#include "base/macros.h"

#include "std/str.h"

#include "ctu-bison.h"
#include "ctu-flex.h"

CT_CALLBACKS(kCallbacks, ctu);

static vector_t *find_mod_path(ast_t *mod, char *fp)
{
    if (mod == NULL) { return vector_init(str_filename_noext(fp)); }

    return vector_len(mod->path) > 0 
        ? mod->path
        : vector_init(str_filename_noext(fp));
}

static void ctu_parse_file(driver_t *runtime, scan_t *scan)
{
    lifetime_t *lifetime = handle_get_lifetime(runtime);

    ctu_init_scan(scan, runtime);
    ast_t *ast = compile_scanner(scan, &kCallbacks);

    char *fp = (char*)scan_path(scan);
    vector_t *path = find_mod_path(ast->modspec, fp);

    context_t *ctx = context_new(runtime, vector_tail(path), ast, NULL, NULL);

    add_context(lifetime, path, ctx);
}

static void pl0_config(lifetime_t *lifetime, ap_t *ap)
{
    ctu_config(lifetime, ap);
}

static const char *kLangNames[] = { "ct", "ctu", NULL };

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

    .fnCreate = ctu_init,

    .fnParse = ctu_parse_file,

    .fnCompilePass = {
        [eStageForwardSymbols] = ctu_forward_decls,
        [eStageCompileImports] = ctu_process_imports,
        [eStageCompileSymbols] = ctu_compile_module
    }
};
