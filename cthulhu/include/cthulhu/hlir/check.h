#pragma once

typedef struct reports_t reports_t;
typedef struct hlir_t hlir_t;

typedef struct
{
    reports_t *reports;

    // keep pointers to the entry points for error reporting
    // we can only have one of each
    const hlir_t *cliEntryPoint;
    const hlir_t *guiEntryPoint;
} check_t;

void check_module(check_t *check, hlir_t *mod);
