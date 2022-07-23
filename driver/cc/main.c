#include "cthulhu/interface/interface.h"
#include "cthulhu/interface/runtime.h"
#include "scan/compile.h"

#include "base/macros.h"

#include "cc-bison.h"
#include "cc-flex.h"

CT_CALLBACKS(kCallbacks, cc);

const driver_t kDriver = {
    .name = "C",
    .version = NEW_VERSION(0, 0, 1),
};

driver_t get_driver(void)
{
    return kDriver;
}
