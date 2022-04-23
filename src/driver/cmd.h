#pragma once

#include "cthulhu/util/util.h"
#include "cthulhu/util/vector.h"
#include "cthulhu/ast/compile.h"

#include <gmp.h>
#include <stdbool.h>

typedef vector_t *flag_t;

typedef struct {
    enum option_type_t { OPT_STRING, OPT_BOOL, OPT_INT } type;
    union {
        const char *string;
        bool boolean;
        mpz_t digit;
    };
} option_t;

option_t *string_option(const char *string);
option_t *bool_option(bool boolean);
option_t *number_option(mpz_t digit);

typedef enum {
#define GROUP(name, desc) name,
#include "flags.inc"
    GROUP_COUNT
} cmd_group_t;

typedef enum {
#define FLAG(group, name, type, desc, ...) group##_##name,
#include "flags.inc"
    FLAG_COUNT
} cmd_flag_t;

cmd_flag_t get_flag(const char *name);

void add_option(cmd_flag_t flag, option_t *option);
void add_file(const char *file);
void init_cmd(void);

#define CMDLTYPE where_t
