#pragma once

#include <gmp.h>

#include "scan/node.h"

#define CMDLTYPE where_t

typedef struct arg_t arg_t;
typedef struct param_t param_t;
typedef struct argparse_t argparse_t;

void argparse_string_opt(argparse_t *argparse, param_t *name, const char *value);
void argparse_int_opt(argparse_t *argparse, param_t *name, mpz_t value);
void argparse_flag_opt(argparse_t *argparse, param_t *name);
void argparse_add_file(argparse_t *argparse, char *name);

int arg_parse_opt(argparse_t *argparse, const char *text, param_t **param);
