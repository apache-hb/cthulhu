#include "import.h"
#include "sema.h"
#include "ctu/frontend/ctu/scan.h"
#include "ctu/driver/include.h"
#include "ctu/util/io.h"

static const char *import_name(ctu_t *ctu) {
    if (ctu->alias != NULL) {
        return ctu->alias;
    }
    
    return vector_tail(ctu->path);
}

static void add_module(sema_t *sema, ctu_t *ctu, sema_t *mod) {
    const char *name = import_name(ctu);
    sema_t *cached = get_module(sema, name);

    if (cached != NULL) {
        message_t *id = report(sema->reports, ERROR, ctu->node, "symbol %s already defined", name);
        report_note(id, "alias the import with `as` to resolve this error");
        return;
    }

    set_module(sema, name, mod);

    if (!is_complete(mod)) {
        ctu_finish(mod);
    }
}

void compile_import(sema_t *sema, ctu_t *ctu) {
    const char *ext = format("%s", strjoin(PATH_SEP, ctu->path));
    path_t *search = relative_path(NODE_PATH(ctu), ext);
    path_t *path = find_include(search);
    if (path == NULL) {
        report(sema->reports, ERROR, ctu->node, "import file not found: %s", path_relative(search));
        return;
    }

    sema_t *cached = get_cache(path_realpath(path));
    if (cached != NULL) {
        add_module(sema, ctu, cached);
        return;
    }

    file_t *file = path_open(path, FILE_READ);
    if (file == NULL) {
        report(sema->reports, ERROR, ctu->node, "failed to open file: %s", path_relative(search));
        return;
    }
    
    scan_t scan = ctu_open(sema->reports, file);
    ctu_t *tree = ctu_compile(BOX(scan));
    if (tree == NULL) {
        return;
    }

    sema_t *import = ctu_start(sema->reports, tree);
    add_module(sema, ctu, import);
    set_cache(path_realpath(path), import);
}
