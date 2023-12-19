#pragma once

#include "core/compiler.h"

BEGIN_API

typedef struct lifetime_t lifetime_t;

/**
 * @brief check the lifetime object for errors, and validate it for malformed data
 *
 * @param reports the reports object
 * @param lifetime the lifetime object
 */
void lifetime_check(lifetime_t *lifetime);

END_API
