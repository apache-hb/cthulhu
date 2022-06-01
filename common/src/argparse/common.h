#pragma once

#include "scan/node.h"

#include "argparse/argparse.h"

#define CMDLTYPE where_t

typedef struct
{
    reports_t *reports; ///< the report sink
    map_t *params; ///< provided parameters
    vector_t *extra; ///< provided files 
} argparse_t;

void argparse_begin_flag(argparse_t *argparse, const char *name);
void argparse_push_string(argparse_t *argparse, const char *value);
void argparse_push_digit(argparse_t *argparse, mpz_t value);
