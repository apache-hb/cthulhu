#include "ctu/ast.h"

#include "base/memory.h"

static ctu_t *ctu_new(scan_t *scan, where_t where, ctu_kind_t kind)
{
    ctu_t *self = ctu_malloc(sizeof(ctu_t));
    self->kind = kind;
    self->node = node_new(scan, where);
    return self;
}

static ctu_t *ctu_decl(scan_t *scan, where_t where, ctu_kind_t kind, char *name)
{
    ctu_t *self = ctu_new(scan, where, kind);
    self->name = name;
    return self;
}

ctu_t *ctu_module(scan_t *scan, where_t where, vector_t *modspec, vector_t *imports, vector_t *decls)
{
    ctu_t *ast = ctu_new(scan, where, eCtuModule);
    ast->modspec = modspec;
    ast->imports = imports;
    ast->decls = decls;
    return ast;
}

ctu_t *ctu_import(scan_t *scan, where_t where, vector_t *path, char *alias)
{
    ctu_t *ast = ctu_new(scan, where, eCtuImport);
    ast->importPath = path;
    ast->alias = alias;
    return ast;
}

ctu_t *ctu_global(scan_t *scan, where_t where, bool exported, bool mut, char *name)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuGlobal, name);
    ast->exported = exported;
    ast->mut = mut;
    return ast;
}
