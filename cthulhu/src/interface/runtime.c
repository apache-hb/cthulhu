#include "cthulhu/interface/runtime.h"

#include "cthulhu/hlir/init.h"
#include "base/util.h"

#include "argparse/argparse.h"

#include "platform/segfault.h"

#include <stdio.h>

void common_init(void)
{
    install_segfault();
    init_gmp();
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
