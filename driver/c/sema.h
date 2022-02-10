#pragma once

#include "cthulhu/hlir/sema.h"

#include "scan.h"
#include "ast.h"

typedef struct {
    node_t *node;
    char *name;
    ast_t *init;
} vardecl_t;

vardecl_t *new_vardecl(scan_t *scan, where_t where, char *name, ast_t *init);

typedef struct {
    char *path;
    sema_t *sema;
} context_t;

context_t *new_context(reports_t *reports);

void cc_module(scan_t *scan, vector_t *path);
void cc_vardecl(scan_t *scan, hlir_linkage_t storage, type_t *type, vector_t *decls);
void cc_finish(scan_t *scan, where_t where);

const char *get_name_for_sign(sign_t sign);
