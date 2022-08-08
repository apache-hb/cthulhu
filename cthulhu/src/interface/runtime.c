#include "cthulhu/interface/runtime.h"

#include "base/memory.h"
#include "base/panic.h"
#include "cthulhu/hlir/init.h"

#include "argparse/argparse.h"

#include <stdio.h>

void common_init(void)
{
    GLOBAL_INIT();

    init_gmp(&globalAlloc);
    argparse_init();
    init_hlir();
}

sema_t *find_module(runtime_t *runtime, const char *path)
{
    logverbose("getting module %s", path);
    return map_get(runtime->modules, path);
}

void add_module(runtime_t *runtime, const char *name, sema_t *sema)
{
    logverbose("adding module %s", name);
    map_set(runtime->modules, name, sema);
}
