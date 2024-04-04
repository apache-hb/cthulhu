// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "json_scan.h"

#include "core/analyze.h"
#include "core/compiler.h"

CT_BEGIN_API

CT_LOCAL void json_action(INOUT_NOTNULL json_where_t *where, const char *text, size_t len);
CT_LOCAL void json_init(OUT_NOTNULL json_where_t *where);
CT_LOCAL void json_update(INOUT_NOTNULL json_where_t *where, IN_READS(steps) const json_where_t *offsets, int steps);

#define YY_USER_ACTION json_action(yylloc, yytext, yyleng);
#define YYLLOC_DEFAULT(current, rhs, offset) json_update(&(current), rhs, offset)
#define YY_USER_INIT json_init(yylloc);

CT_END_API

#include "interop/flex.h" // IWYU pragma: export
