#include "cthulhu/interface/runtime.h"

#include "cthulhu/hlir/init.h"
#include "cthulhu/hlir/query.h"
#include "cthulhu/util/util.h"

void common_init(void)
{
    init_gmp();
    init_hlir();
}

hlir_t *find_module(runtime_t *runtime, const char *path)
{
    return map_get(runtime->modules, path);
}

void add_module(runtime_t *runtime, hlir_t *hlir)
{
    logverbose("adding module `%s`", get_hlir_name(hlir));
    map_set(runtime->modules, get_hlir_name(hlir), hlir);
}
