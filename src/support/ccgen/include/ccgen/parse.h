// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "core/compiler.h"

CT_BEGIN_API

// parsing ebnf rule
typedef struct parse_rule_t parse_rule_t;

// action to take when a parse rule is matched
typedef struct parse_action_t parse_action_t;

typedef enum parse_match_t
{
#define PARSE_RULE_TYPE(ID, STR) ID,
#include "ccgen.inc"

    eParseCount
} parse_match_t;

typedef struct parse_rule_t
{
    parse_match_t kind;
} parse_rule_t;

typedef struct parse_action_t
{
    const parse_rule_t *rule;
} parse_action_t;

CT_END_API
