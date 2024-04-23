// SPDX-License-Identifier: LGPL-3.0-or-later
#include "arena/arena.h"
#include "base/util.h"
#include "ccgen/lex.h"

#include "base/panic.h"

#include "io/io.h"
#include "std/str.h"
#include "std/typed/vector.h"

#include <limits.h>

typedef struct tmlang_t
{
    const lex_grammar_t *grammar;
    arena_t *arena;
} tmlang_t;

static char *emit_rule(tmlang_t *ctx, const lex_rule_t *rule);

static char *emit_match_any(tmlang_t *ctx, const lex_rule_t *rule)
{
    CTASSERT(rule != NULL);
    CTASSERT(rule->kind == eMatchAny);

    lex_match_any_t info = rule->any;

    // +1 for the null terminator
    // +2 for the enclosing []
    // *4 for the worst case of every character needing an escape sequence
    char *sorted = ARENA_MALLOC((info.count * 4) + 1 + 2, "emit_match_any", NULL, ctx->arena);
    sorted[0] = '[';
    ctu_memcpy(sorted + 1, info.chars, info.count);
    sorted[info.count + 1] = ']';
    sorted[info.count + 2] = '\0';

    str_sort_inplace(sorted + 1, info.count);

    // shrink down ranges of consecutive characters

    char *pos = sorted + 1; // current position
    char *end = sorted + info.count + 1; // end of the string

    char *write = pos; // write position
    char *start = pos; // start character of the current range
    char last = *start; // the last character seen

    // we know the string is sorted from smallest to largest
    while (pos != end)
    {
        char cur = *pos;

        if (cur == last + 1)
        {
            // this is the next character in a sequence
            // update last and continue
            last = cur;
        }
        else if (last - *start > 3)
        {
            // this is a range of characters thats long enough
            // to justify converting to a range
            write += str_sprintf(write, (size_t)(end - write), "%c-%c", *start, last);

            // update the start and last
            start = pos;
            last = *start;
        }
        else
        {
            // this is a range of characters that is too short
            // to justify converting to a range
            // write out the characters individually
            for (char *c = start; c != pos; c++)
            {
                *write = *c;
                write += 1;
            }

            // update the start and last
            start = pos;
            last = *start;
        }

        pos += 1;
    }

    // write out the last range
    if (last - *start > 3)
    {
        write += str_sprintf(write, (size_t)(end - write), "%c-%c", *start, last);
    }
    else
    {
        for (char *c = start; c != pos; c++)
        {
            *write = *c;
            write += 1;
        }
    }

    *write++ = ']';
    *write = '\0';

    return sorted;
}

static char *emit_match_seq(tmlang_t *ctx, const lex_rule_t *rule)
{
    CTASSERT(rule != NULL);
    CTASSERT(rule->kind == eMatchSeq);

    lex_match_seq_t info = rule->seq;
    typevec_t *buffer = typevec_new(sizeof(char), 256, ctx->arena);

    for (size_t i = 0; i < info.count; i++)
    {
        char *emit = emit_rule(ctx, info.rules[i]);
        typevec_append(buffer, emit, ctu_strlen(emit));
    }

    typevec_push(buffer, "\0");

    return typevec_data(buffer);
}

static char *emit_match_repeat(tmlang_t *ctx, const lex_rule_t *rule)
{
    CTASSERT(rule != NULL);
    CTASSERT(rule->kind == eMatchRepeat);

    lex_match_repeat_t info = rule->repeat;
    if (info.min == 0 && info.max == 1)
    {
        return str_format(ctx->arena, "%s?", emit_rule(ctx, info.rule));
    }

    if (info.min == 0 && info.max == INT_MAX)
    {
        return str_format(ctx->arena, "%s*", emit_rule(ctx, info.rule));
    }

    if (info.min == 1 && info.max == INT_MAX)
    {
        return str_format(ctx->arena, "%s+", emit_rule(ctx, info.rule));
    }

    if (info.min == info.max)
    {
        return str_format(ctx->arena, "%s{%d}", emit_rule(ctx, info.rule), info.min);
    }

    return str_format(ctx->arena, "%s{%d,%d}", emit_rule(ctx, info.rule), info.min, info.max);
}

static char *emit_rule(tmlang_t *ctx, const lex_rule_t *rule)
{
    CTASSERT(rule != NULL);

    switch (rule->kind)
    {
    case eMatchAny:
        return emit_match_any(ctx, rule);

    case eMatchSeq:
        return emit_match_seq(ctx, rule);

    case eMatchRepeat:
        return emit_match_repeat(ctx, rule);

    default:
        CT_NEVER("unhandled rule kind %d", rule->kind);
    }

    return NULL;
}

void emit_tmlanguage(const lex_grammar_t *grammar, arena_t *arena, io_t *io)
{
    CTASSERT(grammar != NULL);
    CTASSERT(arena != NULL);
    CTASSERT(io != NULL);

    tmlang_t ctx = {
        .grammar = grammar,
        .arena = arena,
    };

    size_t count = grammar->rule_count;
    const lex_rule_t *rules = grammar->rules;

    for (size_t i = 0; i < count; i++)
    {
        const lex_rule_t *rule = rules + i;
        char *emit = emit_rule(&ctx, rule);

        io_printf(io, "rule: \"%s\"\n", emit);
    }
}
