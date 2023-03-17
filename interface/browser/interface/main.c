#include "cthulhu/interface/interface.h"

#include <stdio.h>

int main() {
    common_init();
    driver_t driver = get_driver();

    printf("name: %s\n", driver.name);
}
