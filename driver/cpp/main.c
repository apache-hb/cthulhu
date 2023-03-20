#include "cthulhu/interface/interface.h"

const driver_t kDriver = {
    .name = "CPP",
    .version = NEW_VERSION(0, 0, 1),
    .exts = ".c,.h"
};

driver_t get_driver()
{
    return kDriver;
}
