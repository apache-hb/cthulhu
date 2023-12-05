#pragma once

#include "core/macros.h"

BEGIN_API

typedef struct logger_t reports_t;
typedef struct lifetime_t lifetime_t;

/**
 * @brief check the lifetime object for errors, and validate it for malformed data
 *
 * @param reports the reports object
 * @param lifetime the lifetime object
 */
void lifetime_check(reports_t *reports, lifetime_t *lifetime);

END_API
