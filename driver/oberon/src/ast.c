#include "oberon/ast.h"

#include "base/memory.h"

static obr_t *obr_new(scan_t *scan, where_t where, obr_kind_t kind)
{
    obr_t *self = ctu_malloc(sizeof(obr_t));
    self->kind = kind;
    self->node = node_new(scan, where);
    return self;
}

static obr_t *obr_decl(scan_t *scan, where_t where, obr_kind_t kind, char *name)
{
    obr_t *self = obr_new(scan, where, kind);
    self->name = name;
    return self;
}

obr_t *obr_module(scan_t *scan, where_t where, char *name, vector_t *imports)
{
    obr_t *self = obr_decl(scan, where, eObrModule, name);
    self->imports = imports;
    return self;
}

obr_t *obr_import(scan_t *scan, where_t where, char *name, char *symbol)
{
    obr_t *self = obr_decl(scan, where, eObrImport, name);
    self->symbol = symbol;
    return self;
}
