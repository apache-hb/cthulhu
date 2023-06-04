#include "ast.h"

#include "base/memory.h"

static cc_t *cc_new(scan_t *scan, where_t where, cc_kind_t kind)
{
    cc_t *cc = ctu_malloc(sizeof(cc_t));
    cc->kind = kind;
    cc->node = node_new(scan, where);
    return cc;
}

static cc_t *cc_decl_new(scan_t *scan, where_t where, char *name, cc_kind_t kind)
{
    cc_t *cc = cc_new(scan, where, kind);
    cc->name = name;
    return cc;
}

cc_t *cc_module(scan_t *scan, where_t where, vector_t *path, vector_t *imports, vector_t *decls)
{
    cc_t *cc = cc_new(scan, where, eAstModule);
    cc->modspec = path;
    cc->imports = imports;
    cc->decls = decls;
    return cc;
}

cc_t *cc_import(scan_t *scan, where_t where, vector_t *path, char *name)
{
    cc_t *cc = cc_decl_new(scan, where, name, eAstImport);
    cc->import = path;
    return cc;
}

cc_t *cc_typedef(scan_t *scan, where_t where, char *name, cc_t *type)
{
    cc_t *cc = cc_decl_new(scan, where, name, eAstTypeDefine);
    cc->type = type;
    return cc;
}

cc_t *cc_bool(scan_t *scan, where_t where)
{
    return cc_new(scan, where, eAstBool);
}

cc_t *cc_digit(scan_t *scan, where_t where, sign_t sign, digit_t digit)
{
    cc_t *cc = cc_new(scan, where, eAstDigit);
    cc->sign = sign;
    cc->digit = digit;
    return cc;
}

cc_t *cc_pointer(scan_t *scan, where_t where, cc_t *type)
{
    cc_t *cc = cc_new(scan, where, eAstPointer);
    cc->ptr = type;
    return cc;
}

cc_t *cc_path(scan_t *scan, where_t where, vector_t *path)
{
    cc_t *cc = cc_new(scan, where, eAstPath);
    cc->path = path;
    return cc;
}
