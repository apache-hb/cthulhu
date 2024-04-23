// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ccgen/lex.h"

#include "base/panic.h"

#include "io/io.h"
#include "std/str.h"

// yy_accept map of state id to action

static size_t compute_stt_size(const lex_grammar_t *grammar)
{
    CTASSERT(grammar != NULL);

    size_t size = 1; // 1 for initial state

    for (size_t i = 0; i < grammar->rule_count; i++)
    {
        // +1 for entering the rule
        // +1 for exiting the rule
        size += 2;
    }

    return size;
}

void emit_lexer(const lex_grammar_t *grammar, arena_t *arena, io_t *io)
{
    CTASSERT(grammar != NULL);
    CTASSERT(arena != NULL);
    CTASSERT(io != NULL);

    char *token = str_format(arena, "%s_token_t", grammar->name);
    char *stt = str_format(arena, "%s_state_transition_table", grammar->name);

    size_t stt_size = compute_stt_size(grammar);

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
