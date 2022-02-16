#include "cthulhu/driver/driver.h"

static const driver_t DRIVER = {
    .name = "brainfuck",
    .version = "1.0.0"
};

int main(int argc, const char **argv) {
    common_init();

    return common_main(argc, argv, DRIVER);
}
