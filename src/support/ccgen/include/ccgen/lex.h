// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "core/compiler.h"

#include <stddef.h>

typedef struct arena_t arena_t;
typedef struct io_t io_t;

CT_BEGIN_API

// lexing regex pattern
typedef struct lex_rule_t lex_rule_t;

// action to take when a lexing rule is matched
typedef struct lex_action_t lex_action_t;

// information about a token type
typedef struct lex_token_t lex_token_t;

typedef enum lex_match_t
{
#define LEX_RULE_TYPE(ID, STR) ID,
#include "ccgen.inc"

    eMatchCount
} lex_match_t;

typedef enum token_type_t
{
#define TOKEN_TYPE(ID, STR) ID,
#include "ccgen.inc"

    eTokenCount
} token_type_t;

// match this exact string
typedef struct lex_match_exact_t
{
    const char *chars;
    size_t count;
} lex_match_exact_t;

// match any character in this string
typedef struct lex_match_range_t
{
    const char *chars;
    size_t count;
} lex_match_range_t;

// match these rules in sequence
typedef struct lex_match_seq_t
{
    const lex_rule_t **rules;
    size_t count;
} lex_match_seq_t;

typedef struct lex_match_or_t
{
    const lex_rule_t **rules;
    size_t count;
} lex_match_choice_t;

typedef struct lex_match_repeat_t
{
    // match this rule
    const lex_rule_t *rule;

    // at least this many times
    int min;

    // and at most this many times
    int max;
} lex_match_repeat_t;

typedef struct lex_rule_t
{
    lex_match_t kind;

    union {
        lex_match_exact_t exact;
        lex_match_range_t range;
        lex_match_seq_t seq;
        lex_match_choice_t choice;
        lex_match_repeat_t repeat;
    };
} lex_rule_t;

typedef struct lex_token_t
{
    token_type_t kind;
    const char *name;
} lex_token_t;

typedef struct lex_action_t
{
    const lex_rule_t* rule;
    const lex_token_t* token;
} lex_action_t;

typedef struct lex_grammar_t
{
    const char *name;

    const lex_token_t *tokens;
    size_t token_count;

    const lex_rule_t *rules;
    size_t rule_count;

    const lex_action_t *actions;
    size_t action_count;
} lex_grammar_t;

void emit_lexer(const lex_grammar_t *grammar, arena_t *arena, io_t *io);

void emit_tmlanguage(const lex_grammar_t *grammar, arena_t *arena, io_t *io);

CT_END_API
