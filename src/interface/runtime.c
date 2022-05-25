#include "cthulhu/interface/runtime.h"

#include "cthulhu/hlir/init.h"
#include "base/util.h"

#include <stdio.h>

void common_init(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    init_gmp();
    init_hlir();
}

sema_t *find_module(runtime_t *runtime, const char *path)
{
    return map_get(runtime->modules, path);
}

void add_module(runtime_t *runtime, const char *name, sema_t *sema)
{
    map_set(runtime->modules, name, sema);
}
