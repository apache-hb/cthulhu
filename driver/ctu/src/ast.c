#include "ctu/ast.h"

#include "base/memory.h"

static ctu_t *ctu_new(scan_t *scan, where_t where, ctu_kind_t kind)
{
    ctu_t *self = ctu_malloc(sizeof(ctu_t));
    self->kind = kind;
    self->node = node_new(scan, where);
    return self;
}

static ctu_t *ctu_decl(scan_t *scan, where_t where, ctu_kind_t kind, char *name, bool exported)
{
    ctu_t *self = ctu_new(scan, where, kind);
    self->name = name;
    self->exported = exported;
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

ctu_t *ctu_import(scan_t *scan, where_t where, vector_t *path, char *name)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuImport, name, false);
    ast->importPath = path;
    return ast;
}

/* exprs */

ctu_t *ctu_expr_noinit(scan_t *scan, where_t where)
{
    return ctu_new(scan, where, eCtuExprNoInit);
}

/* types */

ctu_t *ctu_type_name(scan_t *scan, where_t where, vector_t *path)
{
    ctu_t *ast = ctu_new(scan, where, eCtuTypeName);
    ast->typeName = path;
    return ast;
}

ctu_t *ctu_type_pointer(scan_t *scan, where_t where, ctu_t *pointer)
{
    ctu_t *ast = ctu_new(scan, where, eCtuTypePointer);
    ast->pointer = pointer;
    return ast;
}

/* decls */

ctu_t *ctu_decl_global(scan_t *scan, where_t where, bool exported, bool mut, char *name, ctu_t *type, ctu_t *global)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuDeclGlobal, name, exported);
    ast->mut = mut;
    ast->type = type;
    ast->global = global;
    return ast;
}

ctu_t *ctu_decl_function(scan_t *scan, where_t where, bool exported, char *name, ctu_t *returnType)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuDeclFunction, name, exported);
    ast->returnType = returnType;
    return ast;
}
