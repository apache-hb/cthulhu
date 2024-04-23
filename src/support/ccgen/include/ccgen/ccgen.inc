#ifndef LEX_RULE_TYPE
#   define LEX_RULE_TYPE(id, str)
#endif

LEX_RULE_TYPE(eMatchAny, "any")
LEX_RULE_TYPE(eMatchSeq, "seq")
LEX_RULE_TYPE(eMatchRepeat, "repeat")

#undef LEX_RULE_TYPE

#ifndef TOKEN_TYPE
#   define TOKEN_TYPE(id, str)
#endif

TOKEN_TYPE(eTokenNone, "none")
TOKEN_TYPE(eTokenWhiteSpace, "whitespace") // is whitespace
TOKEN_TYPE(eTokenNewLine, "newline") // is a newline
TOKEN_TYPE(eTokenLineComment, "comment") // is a comment
TOKEN_TYPE(eTokenOpenScope, "open symbol") // open paren, bracket, brace, etc.
TOKEN_TYPE(eTokenCloseScope, "close symbol") // close paren, bracket, brace, etc.
TOKEN_TYPE(eTokenEval, "eval") // the tokens content requires additional processing later

#undef TOKEN_TYPE

#ifndef PARSE_RULE_TYPE
#   define PARSE_RULE_TYPE(id, str)
#endif

PARSE_RULE_TYPE(eRuleChoice, "choice")
PARSE_RULE_TYPE(eRuleRepeat, "repeat")

#undef PARSE_RULE_TYPE
