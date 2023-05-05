#include "cthulhu/interface/runtime.h"

#include "base/memory.h"
#include "base/panic.h"

#include "stacktrace/stacktrace.h"
#include "std/map.h"

#include "cthulhu/hlir/init.h"

#include <stdio.h>

void common_init(void)
{
    GLOBAL_INIT();

    stacktrace_init();
    platform_init();
    init_gmp(&globalAlloc);
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
