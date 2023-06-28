#include "cthulhu/emit/emit.h"
#include "cthulhu/ssa/ssa.h"

#include "report/report.h"

#include "io/fs.h"

#include "std/str.h"
#include "std/map.h"

typedef struct c89_t {
    reports_t *reports;
    fs_t *fs;
} c89_t;

static void create_module_dir(c89_t *emit, const char *root, ssa_module_t *mod)
{
    const char *name = mod->name;
    if (name != NULL)
    {
        root = format("%s/%s", root, name);
        fs_dir_create(emit->fs, root);
        logverbose("created directory: %s", root);
    }

    map_iter_t iter = map_iter(mod->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        create_module_dir(emit, root, entry.value);
    }
}

void emit_c89(reports_t *reports, fs_t *fs, ssa_module_t *module)
{
    c89_t c89 = { reports, fs };
    fs_dir_create(fs, "c89");

    create_module_dir(&c89, "c89", module);
}
