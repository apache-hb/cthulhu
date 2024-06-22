// SPDX-License-Identifier: LGPL-3.0-or-later
#include "arena/arena.h"
#include "base/util.h"
#include "ccgen/lex.h"

#include "base/panic.h"

#include "io/io.h"
#include "std/str.h"
#include "std/typed/vector.h"

#include <limits.h>
#include <stdio.h>

typedef struct tmlang_t
{
    const lex_grammar_t *grammar;
    arena_t *arena;
} tmlang_t;

static char *emit_rule(tmlang_t *ctx, const lex_rule_t *rule);

static bool valid_for_range(char c)
{
    return ctu_isalnum(c) || c == '_';
}

static size_t write_range(char *buffer, const char *begin, const char *end)
{
    bool range = true;
    for (const char *c = begin; c != end; c++)
    {
        if (!valid_for_range(*c))
        {
            range = false;
            break;
        }
    }

    // if we can't print all characters, we need to escape them
    if (!range)
    {
        return str_normalize_into(buffer, SIZE_MAX, begin, end - begin);
    }

    // use a range if it makes sense
    if (end - begin > 3)
    {
        // if this range is fully alphabetic or numeric, we can use a range
        bool alpha = ctu_isalpha(*begin) && ctu_isalpha(*(end - 1));
        if (alpha)
            return str_sprintf(buffer, 4, "%c-%c", *begin, *(end - 1));

        bool digit = ctu_isdigit(*begin) && ctu_isdigit(*(end - 1));
        if (digit)
            return str_sprintf(buffer, 4, "%c-%c", *begin, *(end - 1));
    }

    // otherwise, just write out the characters
    for (const char *c = begin; c != end; c++)
    {
        *buffer++ = *c;
    }

    return end - begin;
}

static char *emit_match_range(tmlang_t *ctx, const lex_rule_t *rule)
{
    CTASSERT(rule != NULL);
    CTASSERT(rule->kind == eMatchRange);

    lex_match_range_t info = rule->range;

    // +1 for the null terminator
    char *sorted = ARENA_MALLOC(info.count + 1, "emit_match_any", NULL, ctx->arena);
    ctu_memcpy(sorted, info.chars, info.count);
    sorted[info.count] = '\0';

    str_sort_inplace(sorted, info.count);

    // shrink down ranges of consecutive characters

    size_t outlen = str_normalize_into(NULL, 0, sorted, info.count);

    char *result = ARENA_MALLOC(outlen + 3, "emit_match_any", NULL, ctx->arena);

    const char *read = sorted; // current position
    const char *start = read; // start character of the current range
    const char *end = start + info.count; // end of the string

    char *write = result; // write position
    char last = *start; // the last character seen

    *write++ = '[';

    // we know the string is sorted from smallest to largest
    while (read != end)
    {
        char cur = *read;

        if (cur == last + 1)
        {
            // this is the next character in a sequence
            // update last and continue
            last = cur;
        }
        else
        {
            write += write_range(write, start, read);

            // update the start and last
            last = *read;
            start = read;
        }

        read += 1;
    }

    // write out the last range
    write += write_range(write, start, read);

    *write++ = ']';
    *write++ = '\0';

    return result;
}

static char *emit_match_exact(tmlang_t *ctx, const lex_rule_t *rule)
{
    CTASSERT(rule != NULL);
    CTASSERT(rule->kind == eMatchExact);

    lex_match_exact_t info = rule->exact;

    size_t len = str_normalize_into(NULL, 0, info.chars, info.count);
    char *buffer = ARENA_MALLOC(len + 3, "emit_match_exact", NULL, ctx->arena);

    char *write = buffer;
    *write++ = '"';
    write += str_normalize_into(write, len, info.chars, info.count);
    *write++ = '"';
    *write = '\0';

    return buffer;
}

static char *emit_match_choice(tmlang_t *ctx, const lex_rule_t *rule)
{
    CTASSERT(rule != NULL);
    CTASSERT(rule->kind == eMatchChoice);

    lex_match_choice_t info = rule->choice;
    typevec_t *buffer = typevec_new(sizeof(char), 256, ctx->arena);

    for (size_t i = 0; i < info.count; i++)
    {
        if (i != 0)
            typevec_push(buffer, "|");

        char *emit = emit_rule(ctx, info.rules[i]);
        typevec_append(buffer, emit, ctu_strlen(emit));
    }

    typevec_push(buffer, "\0");

    return typevec_data(buffer);
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
    case eMatchExact:
        return emit_match_exact(ctx, rule);

    case eMatchRange:
        return emit_match_range(ctx, rule);

    case eMatchChoice:
        return emit_match_choice(ctx, rule);

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

    size_t count = grammar->action_count;
    const lex_action_t *actions = grammar->actions;

    for (size_t i = 0; i < count; i++)
    {
        const lex_action_t *rule = actions + i;
        char *emit = emit_rule(&ctx, rule->rule);

        io_printf(io, "rule: `%s`\n", emit);
    }
}
