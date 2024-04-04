// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <stdbool.h>

#include "core/compiler.h"

CT_LOCAL size_t info_ptr_hash(const void *key);
CT_LOCAL bool info_ptr_equal(const void *lhs, const void *rhs);

CT_LOCAL size_t info_str_hash(const void *key);
CT_LOCAL bool info_str_equal(const void *lhs, const void *rhs);

CT_LOCAL size_t info_text_hash(const void *key);
CT_LOCAL bool info_text_equal(const void *lhs, const void *rhs);
