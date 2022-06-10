#pragma once

#include <gmp.h>

#include "scan/node.h"

#define CMDLTYPE where_t

typedef struct argparse_t argparse_t;

void argparse_begin_flag(argparse_t *argparse, const char *name);
void argparse_push_string(argparse_t *argparse, const char *value);
void argparse_push_digit(argparse_t *argparse, mpz_t value);
void argparse_end_flag(argparse_t *argparse);
