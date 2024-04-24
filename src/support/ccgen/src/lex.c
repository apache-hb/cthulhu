// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ccgen/lex.h"

#include "base/panic.h"

#include "core/macros.h"
#include "io/io.h"
#include "std/str.h"

// yy_accept map of state id to action

// compute number of possible states
static size_t compute_state_count(const lex_grammar_t *grammar)
{
    CTASSERT(grammar != NULL);

    // +1 for initial state
    // +1 for final state
    // +1 for error state
    return grammar->rule_count + 3;
}

#if 0
static size_t compute_initial_state(const lex_grammar_t *grammar)
{
    CT_UNUSED(grammar);

    return 0;
}

static size_t compute_final_state(const lex_grammar_t *grammar)
{
    return grammar->rule_count + 1;
}

static size_t compute_error_state(const lex_grammar_t *grammar)
{
    return grammar->rule_count + 2;
}
#endif

void emit_lexer(const lex_grammar_t *grammar, arena_t *arena, io_t *io)
{
    CTASSERT(grammar != NULL);
    CTASSERT(arena != NULL);
    CTASSERT(io != NULL);

    char *token = str_format(arena, "%s_token_t", grammar->name);
    char *stt = str_format(arena, "%s_state_transition_table", grammar->name);

    size_t stt_size = compute_state_count(grammar);

    io_printf(io, "const int %s[%zu] = {", stt, stt_size);

    io_printf(io, "};\n");

    io_printf(io, "typedef struct %s {\n", token);
    io_printf(io, "\tint type;\n");
    io_printf(io, "\tint start;\n");
    io_printf(io, "\tint end;\n");
    io_printf(io, "} %s;\n", token);

    io_printf(io, "int %s_lex(const char *input, %s *tokens)\n", grammar->name, token);
    io_printf(io, "{\n");
    io_printf(io, "\tint state = 0; // current state index\n");
    io_printf(io, "\tint start = 0; // current offset into input\n");
    io_printf(io, "\tint count = 0; // number of tokens written\n");

    io_printf(io, "\twhile (input[start] != '\\0') {\n");

    io_printf(io, "\t}\n");

    io_printf(io, "}\n");
}
