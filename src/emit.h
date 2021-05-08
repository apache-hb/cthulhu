#ifndef EMIT_H
#define EMIT_H

#include "ast.h"
#include <stdio.h>

void emit(char *name, FILE *source, FILE *header, node_t *prog);

#endif 
