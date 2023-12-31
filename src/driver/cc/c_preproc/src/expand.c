#include "base/log.h"
#include "base/panic.h"
#include "core/macros.h"
#include "cpp/cpp.h"
#include "cpp/events.h"
#include "cpp/scan.h"
#include "cthulhu/events/events.h"
#include "notify/notify.h"
#include "std/map.h"
#include "std/str.h"

typedef struct eval_t
{
    const node_t *root;

    scan_t *scan;
    logger_t *logger;
    vector_t *tokens;
    size_t index;
} eval_t;

// context is a mapping of names to vectors of already expanded tokens
static void expand_tokens(eval_t *dst, cpp_ast_t **tokens, size_t len, map_t *context);

// collect the macro arguments into a vector of tokens
// the final vector only contains the tokens, we skip over the commas and enclosing parens
static vector_t *collect_macro_args(eval_t *dst, cpp_ast_t **tokens, size_t remaining)
{
    CTASSERT(dst != NULL);
    CTASSERT(tokens != NULL);

    CTU_UNUSED(remaining);

    arena_t *arena = scan_get_arena(dst->scan);

    vector_t *args = vector_new_arena(4, arena);

    return args;
}

static map_t *make_context(eval_t *ctx, cpp_ast_t *macro, const node_t *node, vector_t *args)
{
    CTASSERT(macro != NULL);
    CTASSERT(cpp_ast_is(macro, eCppMacro));

    cpp_params_t params = macro->params;

    size_t len = vector_len(params.names);
    size_t arglen = vector_len(args);

    if (!params.variadic && (arglen > len))
    {
        msg_notify(ctx->logger, &kEvent_TooManyArgs, node, "too many arguments to macro");
    }

    map_t *context = map_optimal_arena(len, scan_get_arena(ctx->scan));

    size_t it = MIN(len, arglen);

    for (size_t i = 0; i < it; i++)
    {
        const char *name = vector_get(params.names, i);
        vector_t *arg = vector_get(args, i);

        map_set(context, name, arg);
    }

    // if theres more args than params, collect them into a comma seperated string
    if (arglen > len)
    {
        vector_t *varargs = vector_new_arena(4, scan_get_arena(ctx->scan));

        for (size_t i = len; i < arglen; i++)
        {
            vector_t *arg = vector_get(args, i);
            vector_push(&varargs, arg);
        }

        map_set(context, "__VA_ARGS__", varargs);
    }

    return context;
}

static void expand_ident(eval_t *dst, cpp_ast_t *ident, cpp_ast_t **tokens, size_t remaining, map_t *context)
{
    CTASSERT(dst != NULL);
    CTASSERT(ident != NULL);

    vector_t *inner = map_get(context, ident->text);
    if (inner != NULL)
    {
        cpp_ast_t **inner_tokens = (cpp_ast_t**)vector_data(inner);
        size_t inner_len = vector_len(inner);

        // if we matched a param expand that in
        expand_tokens(dst, inner_tokens, inner_len, context);
        return;
    }

    // check if this is a macro
    cpp_ast_t *macro = cpp_get_define(dst->scan, ident->text);
    if (macro == NULL)
    {
        // if not just push the ident
        vector_push(&dst->tokens, ident);
    }
    else if (macro->kind == eCppDefine)
    {
        // simple text define
        cpp_ast_t **inner_tokens = (cpp_ast_t**)vector_data(macro->body);
        size_t inner_len = vector_len(macro->body);

        expand_tokens(dst, inner_tokens, inner_len, context);
    }
    else if (macro->kind == eCppMacro)
    {
        // its a macro, first expand the params
        vector_t *save = dst->tokens;
        dst->tokens = vector_new_arena(4, scan_get_arena(dst->scan));

        // expand tokens after this into the new context
        expand_tokens(dst, tokens + 1, remaining - 1, context);

        // swap the tokens back
        vector_t *expanded = dst->tokens;
        dst->tokens = save;

        // now we have the tokens we can firstly find the params
        // and then expand them, before expanding the macro body
        cpp_ast_t **expanded_tokens = (cpp_ast_t**)vector_data(expanded);
        size_t expanded_len = vector_len(expanded);

        vector_t *args = collect_macro_args(dst, expanded_tokens, expanded_len);

        map_t *new_context = make_context(dst, macro, ident->node, args);

        // now expand the macro body
        cpp_ast_t **inner_tokens = (cpp_ast_t**)vector_data(macro->body);
        size_t inner_len = vector_len(macro->body);

        expand_tokens(dst, inner_tokens, inner_len, new_context);
    }
    else
    {
        // huh thats odd, how did this happen?
        NEVER("unknown macro kind %d", macro->kind);
    }
}

static size_t expand_defined(eval_t *dst, cpp_ast_t **tokens, size_t remaining)
{
    // lookahead to the next token
    if (remaining <= 1)
    {
        msg_notify(dst->logger, &kEvent_UnexpectedToken, dst->root, "unexpected end of input");
        return 0;
    }

    arena_t *arena = scan_get_arena(dst->scan);

    size_t offset = 1;
    cpp_ast_t *next = tokens[offset];
    bool parens = next->kind == eCppLParen;
    if (parens)
    {
        offset += 1;
    }

    // check if this is a macro
    cpp_ast_t *ident = tokens[offset];
    if (cpp_ast_is_not(ident, eCppIdent))
    {
        msg_notify(dst->logger, &kEvent_UnexpectedToken, ident->node, "expected identifier during `defined` evaluation");
        return 0;
    }

    cpp_ast_t *macro = cpp_get_define(dst->scan, ident->text);
    cpp_ast_t *tok = cpp_number_int(arena, ident->node, macro != NULL);
    vector_push(&dst->tokens, tok);

    if (parens)
    {
        if (remaining <= 2)
        {
            msg_notify(dst->logger, &kEvent_UnexpectedToken, dst->root, "unexpected end of input");
            return 0;
        }

        cpp_ast_t *close = tokens[offset + 1];
        if (close->kind != eCppRParen)
        {
            msg_notify(dst->logger, &kEvent_UnexpectedToken, close->node, "expected closing paren");
            return 2;
        }

        // skip the paren, the token, and the paren after
        return 3;
    }
    else
    {
        // skip the next token
        return 1;
    }
}

static void expand_tokens(eval_t *dst, cpp_ast_t **tokens, size_t remaining, map_t *context)
{
    CTASSERT(dst != NULL);
    CTASSERT(tokens != NULL);

    for (size_t i = 0; i < remaining; i++)
    {
        cpp_ast_t *token = tokens[i];
        CTASSERT(token != NULL);

        if (token->kind == eCppIdent)
        {
            // we may need to expand an ident
            expand_ident(dst, token, tokens, remaining - i, context);
        }
        else if (token->kind == eCppDefined)
        {
            i += expand_defined(dst, tokens + i, remaining - i);
        }
        else
        {
            // all other tokens can be pushed directly
            vector_push(&dst->tokens, token);
        }
    }
}

static cpp_ast_t *eval_peek(eval_t *eval)
{
    CTASSERT(eval != NULL);

    size_t len = vector_len(eval->tokens);
    if (eval->index >= len)
    {
        return NULL;
    }

    return vector_get(eval->tokens, eval->index);
}

static cpp_ast_t *eval_consume(eval_t *eval, cpp_kind_t kind)
{
    cpp_ast_t *ast = eval_peek(eval);
    if (cpp_ast_is_not(ast, kind)) return NULL;

    eval->index += 1;
    return ast;
}

static cpp_ast_t *eval_consume_unary(eval_t *eval, unary_t unary)
{
    cpp_ast_t *ast = eval_peek(eval);
    if (cpp_ast_is_not(ast, eCppUnary)) return NULL;

    if (ast->unary != unary) return NULL;

    eval->index += 1;
    return ast;
}

static cpp_ast_t *eval_consume_binary(eval_t *eval, binary_t binary)
{
    cpp_ast_t *ast = eval_peek(eval);
    if (cpp_ast_is_not(ast, eCppBinary)) return NULL;

    if (ast->binary != binary) return NULL;

    eval->index += 1;
    return ast;
}

static cpp_ast_t *eval_consume_binary_any(eval_t *eval, const binary_t *choices, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        cpp_ast_t *ast = eval_consume_binary(eval, choices[i]);
        if (ast != NULL)
        {
            return ast;
        }
    }

    return NULL;
}

static cpp_ast_t *eval_consume_compare(eval_t *eval, compare_t compare)
{
    cpp_ast_t *ast = eval_peek(eval);
    if (cpp_ast_is_not(ast, eCppCompare)) return NULL;

    if (ast->compare != compare) return NULL;

    eval->index += 1;
    return ast;
}

static cpp_ast_t *eval_consume_compare_any(eval_t *eval, const compare_t *choices, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        cpp_ast_t *ast = eval_consume_compare(eval, choices[i]);
        if (ast != NULL)
        {
            return ast;
        }
    }

    return NULL;
}

static bool eval_expect(eval_t *eval, cpp_kind_t kind)
{
    cpp_ast_t *ast = eval_peek(eval);
    if (ast == NULL)
    {
        msg_notify(eval->logger, &kEvent_UnexpectedToken, eval->root, "unexpected end of input");
        return false;
    }

    if (ast->kind != kind)
    {
        msg_notify(eval->logger, &kEvent_UnexpectedToken, ast->node, "unexpected token during evaluation");
        where_t where = node_get_location(eval->root);
        ctu_log("expected %d, got %d %s %llu", kind, ast->kind, scan_path(node_get_scan(eval->root)), where.first_line);
        return false;
    }
    else
    {
        // only consume if we matched
        eval->index += 1;
        return true;
    }
}

// we try and follow the grammar defined in 6.5 [ISO/IEC 9899:201x] as closely as possible

static bool eval_constant_expr(eval_t *eval, mpz_t value);

// 6.5.1 Primary expressions
static bool eval_primary_expr(eval_t *eval, mpz_t value)
{
    cpp_ast_t *ast = eval_consume(eval, eCppNumber);
    if (ast != NULL)
    {
        mpz_init_set(value, ast->digit);
        return true;
    }

    if (eval_consume(eval, eCppLParen) == NULL)
        return false;

    if (!eval_constant_expr(eval, value)) return false;

    return eval_expect(eval, eCppRParen);
}

// 6.5.3 Unary operators
static bool eval_unary_expr(eval_t *eval, mpz_t value)
{
    cpp_ast_t *unary = eval_consume_unary(eval, eUnaryNot);
    if (unary != NULL)
    {
        mpz_t tmp;
        if (!eval_unary_expr(eval, tmp))
            return false;

        switch (unary->unary) {
        case eUnaryNot:
            mpz_init_set_ui(value, !(mpz_cmp_ui(tmp, 0) != 0));
            break;
        case eUnaryNeg:
            mpz_init_set(value, tmp);
            mpz_neg(value, value);
            break;
        case eUnaryFlip:
            mpz_init_set(value, tmp);
            mpz_com(value, value);
            break;

        default:
            NEVER("unknown unary operator %d", unary->unary);
        }

        return true;
    }

    return eval_primary_expr(eval, value);
}

static const binary_t kMulOps[] = { eBinaryMul, eBinaryDiv, eBinaryRem };
#define MUL_COUNT (sizeof(kMulOps) / sizeof(binary_t))

// 6.5.5 Multiplicative operators
static bool eval_multiplicative_expr(eval_t *eval, mpz_t value)
{
    mpz_t lhs;
    if (!eval_unary_expr(eval, lhs))
        return false;

    cpp_ast_t *ast = eval_consume_binary_any(eval, kMulOps, MUL_COUNT);
    if (ast == NULL)
    {
        mpz_init_set(value, lhs);
        return true;
    }

    mpz_t rhs;
    if (!eval_multiplicative_expr(eval, rhs))
        return false;

    mpz_init_set(value, lhs);

    switch (ast->binary) {
    case eBinaryMul:
        mpz_mul(value, value, rhs);
        break;
    case eBinaryDiv:
        if (mpz_cmp_ui(rhs, 0) == 0)
        {
            msg_notify(eval->logger, &kEvent_DivideByZero, ast->node, "divide by zero during evaluation");
            return false;
        }
        mpz_divexact(value, value, rhs);
        break;
    case eBinaryRem:
        mpz_mod(value, value, rhs);
        break;
    default:
        NEVER("unknown binary operator %d", ast->binary);
    }

    return true;
}

static const binary_t kAddOps[] = { eBinaryAdd, eBinarySub };
#define ADD_COUNT (sizeof(kAddOps) / sizeof(binary_t))

// 6.5.6 Additive operators
static bool eval_addative_expr(eval_t *eval, mpz_t value)
{
    mpz_t lhs;
    if (!eval_multiplicative_expr(eval, lhs))
        return false;

    cpp_ast_t *ast = eval_consume_binary_any(eval, kAddOps, ADD_COUNT);
    if (ast == NULL)
    {
        mpz_init_set(value, lhs);
        return true;
    }

    mpz_t rhs;
    if (!eval_addative_expr(eval, rhs))
        return false;

    mpz_init_set(value, lhs);

    switch (ast->binary) {
    case eBinaryAdd:
        mpz_add(value, value, rhs);
        break;
    case eBinarySub:
        mpz_sub(value, value, rhs);
        break;
    default:
        NEVER("unknown binary operator %d", ast->binary);
    }

    return true;
}

static const binary_t kShiftOps[] = { eBinaryShl, eBinaryShr };
#define SHIFT_COUNT (sizeof(kShiftOps) / sizeof(binary_t))
// 6.5.7 Bitwise shift operators
static bool eval_shift_expr(eval_t *eval, mpz_t value)
{
    mpz_t lhs;
    if (!eval_addative_expr(eval, lhs))
        return false;

    cpp_ast_t *ast = eval_consume_binary_any(eval, kShiftOps, SHIFT_COUNT);
    if (ast == NULL)
    {
        mpz_init_set(value, lhs);
        return true;
    }

    mpz_t rhs;
    if (!eval_shift_expr(eval, rhs))
        return false;

    mpz_init_set(value, lhs);

    switch (ast->binary) {
    case eBinaryShl:
        mpz_mul_2exp(value, value, mpz_get_ui(rhs));
        break;
    case eBinaryShr:
        mpz_fdiv_q_2exp(value, value, mpz_get_ui(rhs));
        break;
    default:
        NEVER("unknown binary operator %d", ast->binary);
    }

    return true;
}

static const compare_t kCompareOps[] = { eCompareLt, eCompareGt, eCompareLte, eCompareGte };
#define COMPARE_COUNT (sizeof(kCompareOps) / sizeof(compare_t))

// 6.5.8 Relational operators
static bool eval_relational_expr(eval_t *eval, mpz_t value)
{
    if (!eval_shift_expr(eval, value))
        return false;

    cpp_ast_t *ast = eval_consume_compare_any(eval, kCompareOps, COMPARE_COUNT);
    if (ast == NULL)
        return true;

    mpz_t rhs;
    if (!eval_relational_expr(eval, rhs))
        return false;

    switch (ast->compare) {
    case eCompareLt:
        mpz_init_set_ui(value, mpz_cmp(value, rhs) < 0);
        break;
    case eCompareGt:
        mpz_init_set_ui(value, mpz_cmp(value, rhs) > 0);
        break;
    case eCompareLte:
        mpz_init_set_ui(value, mpz_cmp(value, rhs) <= 0);
        break;
    case eCompareGte:
        mpz_init_set_ui(value, mpz_cmp(value, rhs) >= 0);
        break;
    default:
        NEVER("unknown compare operator %d", ast->compare);
    }

    return true;
}

static const compare_t kEqualityOps[] = { eCompareEq, eCompareNeq };
#define EQUALITY_COUNT (sizeof(kEqualityOps) / sizeof(compare_t))

// 6.5.9 Equality operators
static bool eval_equality_expr(eval_t *eval, mpz_t value)
{
    if (!eval_relational_expr(eval, value))
        return false;

    cpp_ast_t *ast = eval_consume_compare_any(eval, kEqualityOps, EQUALITY_COUNT);
    if (ast == NULL)
        return true;

    mpz_t rhs;
    if (!eval_equality_expr(eval, rhs))
        return false;

    switch (ast->compare) {
    case eCompareEq:
        mpz_init_set_ui(value, mpz_cmp(value, rhs) == 0);
        break;
    case eCompareNeq:
        mpz_init_set_ui(value, mpz_cmp(value, rhs) != 0);
        break;
    default:
        NEVER("unknown compare operator %d", ast->compare);
    }

    return true;
}

// 6.5.10 Bitwise AND operator
static bool eval_and_expr(eval_t *eval, mpz_t value)
{
    if (!eval_equality_expr(eval, value))
        return false;

    cpp_ast_t *ast = eval_consume_binary(eval, eBinaryBitAnd);
    if (ast == NULL)
        return true;

    mpz_t rhs;
    if (!eval_and_expr(eval, rhs))
        return false;

    mpz_init_set(value, rhs);
    mpz_and(value, value, rhs);

    return true;
}

// 6.5.11 Bitwise exclusive OR operator
static bool eval_xor_expr(eval_t *eval, mpz_t value)
{
    if (!eval_and_expr(eval, value))
        return false;

    cpp_ast_t *ast = eval_consume_binary(eval, eBinaryXor);
    if (ast == NULL)
        return true;

    mpz_t rhs;
    if (!eval_xor_expr(eval, rhs))
        return false;

    mpz_init_set(value, rhs);
    mpz_xor(value, value, rhs);

    return true;
}

// 6.5.12 Bitwise inclusive OR operator
static bool eval_inclusive_or_expr(eval_t *eval, mpz_t value)
{
    if (!eval_xor_expr(eval, value))
        return false;

    cpp_ast_t *ast = eval_consume_binary(eval, eBinaryBitOr);
    if (ast == NULL)
        return true;

    mpz_t rhs;
    if (!eval_inclusive_or_expr(eval, rhs))
        return false;

    mpz_init_set(value, rhs);
    mpz_ior(value, value, rhs);

    return true;
}

// 6.5.13 Logical AND operator
static bool eval_logical_and_expr(eval_t *eval, mpz_t value)
{
    mpz_t tmp;
    if (!eval_inclusive_or_expr(eval, tmp))
        return false;

    cpp_ast_t *ast = eval_consume_compare(eval, eCompareAnd);
    if (ast == NULL)
    {
        mpz_init_set(value, tmp);
        return true;
    }

    mpz_t rhs;
    if (!eval_logical_and_expr(eval, rhs))
        return false;

    mpz_init_set_ui(value, !(mpz_cmp_ui(tmp, 0) != 0 && mpz_cmp_ui(rhs, 0) != 0));
    return true;
}

// 6.5.14 Logical OR operator
static bool eval_logical_or_expr(eval_t *eval, mpz_t value)
{
    mpz_t tmp;
    if (!eval_logical_and_expr(eval, tmp))
        return false;

    cpp_ast_t *ast = eval_consume_compare(eval, eCompareOr);
    if (ast == NULL)
    {
        mpz_init_set(value, tmp);
        return true;
    }

    mpz_t rhs;
    if (!eval_logical_or_expr(eval, rhs))
        return false;

    mpz_init_set_ui(value, !(mpz_cmp_ui(tmp, 0) != 0 || mpz_cmp_ui(rhs, 0) != 0));
    return true;
}

// 6.5.15 Conditional operator
static bool eval_conditional_expr(eval_t *eval, mpz_t value)
{
    mpz_t tmp;
    if (!eval_logical_or_expr(eval, tmp))
        return false;

    cpp_ast_t *ast = eval_peek(eval);
    if (cpp_ast_is(ast, eCppTernary))
    {
        // we expect a constant-expression rather than an expression
        // which technically deviates from the grammar.
        // however, we can't evaluate an expression at compile time
        mpz_t truthy;
        if (!eval_constant_expr(eval, truthy))
            return false;

        if (!eval_expect(eval, eCppColon))
            return false;

        mpz_t falsey;
        if (!eval_constant_expr(eval, falsey))
            return false;

        if (mpz_cmp_ui(truthy, 0) != 0)
        {
            mpz_init_set(value, tmp);
        }
        else
        {
            mpz_init_set(value, falsey);
        }
    }
    else
    {
        mpz_init_set(value, tmp);
    }

    return true;
}

// 6.6 Constant expressions
static bool eval_constant_expr(eval_t *eval, mpz_t value)
{
    return eval_conditional_expr(eval, value);
}

bool eval_condition(scan_t *scan, const node_t *node, vector_t *condition)
{
    CTASSERT(scan != NULL);
    CTASSERT(condition != NULL);

    arena_t *arena = scan_get_arena(scan);
    size_t len = vector_len(condition);
    if (len == 0)
    {
        // TODO: this is an error
        return false;
    }

    // first expand the tokens
    cpp_scan_t *extra = cpp_scan_context(scan);

    eval_t eval = {
        .root = node,
        .scan = scan,
        .logger = extra->instance->logger,
        .tokens =  vector_new_arena(len, arena),
    };

    cpp_ast_t **tokens = (cpp_ast_t**)vector_data(condition);

    expand_tokens(&eval, tokens, len, map_optimal_arena(1, arena));

    // now evaluate the tokens

    mpz_t value;
    mpz_init(value);
    bool result = eval_constant_expr(&eval, value);

    if (!result) return false;

    return mpz_cmp_ui(value, 0) != 0;
}

vector_t *expand_macro(scan_t *scan, const char *name, vector_t *args)
{
    CTASSERT(scan != NULL);
    CTASSERT(name != NULL);
    CTASSERT(args != NULL);

    vector_t *temp = vector_new(4);

    return temp; // TODO: implement
}
