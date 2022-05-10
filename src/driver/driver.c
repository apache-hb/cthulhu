#include "cthulhu/driver/driver.h"

#include "cthulhu/hlir/init.h"
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
