#include "ccgen/lex.h"

#include "io/console.h"
#include "memory/memory.h"
#include "setup/setup.h"

#include "core/macros.h"
#include "base/util.h"

#include <limits.h>

static lex_rule_t lex_match_any(const char *chars)
{
    lex_match_any_t any = {
        .chars = chars,
        .count = ctu_strlen(chars)
    };

    lex_rule_t rule = {
        .kind = eMatchAny,
        .any = any
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

int main(int argc, const char **argv)
{
    CT_UNUSED(argc);
    CT_UNUSED(argv);

    setup_default(NULL);

    enum { eTokWS, eTokID, eTokCount };
    enum { eRuleWS, eRuleIdHead, eRuleIdTail, eRuleIdBody, eRuleId, eRuleCount };
    enum { eActionID, eActionWS, eActionCount };

    const lex_token_t tokens[eTokCount] = {
        [eTokWS] = { .kind = eTokenWhiteSpace, .name = "whitespace" },
        [eTokID] = { .kind = eTokenNone, .name = "identifier" }
    };

    lex_rule_t rules[eRuleCount] = {
        [eRuleWS] = lex_match_any(" \v\f\t\n\r"),

        [eRuleIdHead] = lex_match_any("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"),
        [eRuleIdTail] = lex_match_any("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789"),
        [eRuleIdBody] = lex_repeat(&rules[eRuleIdTail], 0, INT_MAX),
    };

    const lex_rule_t *id[] = { &rules[eRuleIdHead], &rules[eRuleIdBody] };

    rules[eRuleId] = lex_seq(id, CT_ARRAY_LEN(id));

    const lex_action_t actions[eActionCount] = {
        [eActionID] = { .rule = &rules[eRuleId], .token = &tokens[eTokID] },
        [eActionWS] = { .rule = &rules[eRuleWS], .token = &tokens[eTokWS] }
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
