#include "util.h"

#include <string.h>

bool startswith(const char *str, const char *sub) {
    return strncmp(str, sub, strlen(sub)) == 0;
}
