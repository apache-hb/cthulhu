#include "ccgen/lex.h"

#include "io/console.h"
#include "memory/memory.h"
#include "setup/setup.h"

#include "core/macros.h"
#include "base/util.h"

#include <limits.h>

static lex_rule_t lex_exact(const char *chars)
{
    lex_match_exact_t exact = {
        .chars = chars,
        .count = ctu_strlen(chars)
    };

    lex_rule_t rule = {
        .kind = eMatchExact,
        .exact = exact
    };

    return rule;
}

static lex_rule_t lex_range(const char *chars)
{
    lex_match_range_t range = {
        .chars = chars,
        .count = ctu_strlen(chars)
    };

    lex_rule_t rule = {
        .kind = eMatchRange,
        .range = range
    };

    return rule;
}

static lex_rule_t lex_repeat(const lex_rule_t *rule, int min, int max)
{
    lex_match_repeat_t repeat = {
        .rule = rule,
        .min = min,
        .max = max
    };

    lex_rule_t result = {
        .kind = eMatchRepeat,
        .repeat = repeat
    };

    return result;
}

static lex_rule_t lex_seq(const lex_rule_t **rules, size_t count)
{
    lex_match_seq_t seq = {
        .rules = rules,
        .count = count
    };

    lex_rule_t result = {
        .kind = eMatchSeq,
        .seq = seq
    };

    return result;
}

static lex_rule_t lex_or(const lex_rule_t **rules, size_t count)
{
    lex_match_choice_t choice = {
        .rules = rules,
        .count = count
    };

    lex_rule_t result = {
        .kind = eMatchChoice,
        .choice = choice
    };

    return result;
}

int main(int argc, const char **argv)
{
    CT_UNUSED(argc);
    CT_UNUSED(argv);

    setup_default(NULL);

    enum { eTokWS, eTokID, eTokCount };
    enum {
        eRuleWS,
        eRuleLineComment, eRuleCommentHash, eRuleCommentC,
        eRuleIdHead, eRuleIdTail, eRuleIdBody, eRuleId,
        eRuleCount
    };
    enum { eActionID, eActionWS, eActionComment, eActionCount };

    const lex_token_t tokens[eTokCount] = {
        [eTokWS] = { .kind = eTokenWhiteSpace, .name = "whitespace" },
        [eTokID] = { .kind = eTokenNone, .name = "identifier" }
    };

    lex_rule_t rules[eRuleCount] = {
        [eRuleWS] = lex_range(" \v\f\t\n\r"),

        [eRuleCommentHash] = lex_exact("#"),
        [eRuleCommentC] = lex_exact("//"),

        [eRuleIdHead] = lex_range("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"),
        [eRuleIdTail] = lex_range("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789"),
        [eRuleIdBody] = lex_repeat(&rules[eRuleIdTail], 0, INT_MAX),
    };

    const lex_rule_t *comment[] = { &rules[eRuleCommentHash], &rules[eRuleCommentC] };
    rules[eRuleLineComment] = lex_or(comment, CT_ARRAY_LEN(comment));

    const lex_rule_t *id[] = { &rules[eRuleIdHead], &rules[eRuleIdBody] };
    rules[eRuleId] = lex_seq(id, CT_ARRAY_LEN(id));

    const lex_action_t actions[eActionCount] = {
        [eActionID] = { .rule = &rules[eRuleId], .token = &tokens[eTokID] },
        [eActionWS] = { .rule = &rules[eRuleWS], .token = &tokens[eTokWS] },
        [eActionComment] = { .rule = &rules[eRuleLineComment], .token = &tokens[eTokWS] }
    };

    const lex_grammar_t grammar = {
        .name = "test",

        .tokens = tokens,
        .token_count = eTokCount,

        .rules = rules,
        .rule_count = eRuleCount,

        .actions = actions,
        .action_count = eActionCount,
    };

    // emit_lexer(&grammar, get_global_arena(), io_stdout());

    emit_tmlanguage(&grammar, get_global_arena(), io_stdout());
}
