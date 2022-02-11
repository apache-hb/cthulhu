#pragma once

#include "cthulhu/hlir/sema.h"

#include "scan.h"
#include "ast.h"

typedef struct {
    node_t *node;
    type_t *type;
    char *name;
    ast_t *init;
} vardecl_t;

vardecl_t *new_vardecl(scan_t *scan, where_t where, type_t *type, char *name, ast_t *init);

typedef struct {
    char *path;
    type_t *current;
    type_t *default_int;
    sema_t *sema;
} context_t;

context_t *new_context(reports_t *reports);

void cc_module(scan_t *scan, vector_t *path);
void cc_vardecl(scan_t *scan, hlir_linkage_t storage, vector_t *decls);
void cc_finish(scan_t *scan, where_t where);

void set_current_type(scan_t *scan, type_t *type);
type_t *get_current_type(scan_t *scan);
type_t *default_int(scan_t *scan, const node_t *node);
