#pragma once

#include <stdio.h>

#include "ast.h"

typedef size_t msg_idx_t;

void max_errors(size_t num);
msg_idx_t add_lexer_error(const char *msg, scanner_t *scanner, YYLTYPE loc);
msg_idx_t add_error(const char *msg, node_t *node);
msg_idx_t add_warn(const char *msg, node_t *node);
void add_note(msg_idx_t id, const char *note);
bool write_messages(const char *stage);

nodes_t *compile_file(const char *path, FILE *stream);
nodes_t *compile_string(const char *path, const char *text);
