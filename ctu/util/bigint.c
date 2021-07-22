#include "bigint.h"

#include "str.h"

bigint_t new_bigint(void) {
    bigint_t b;
    b.sign = true;
    b.inner = strdup("0");
}