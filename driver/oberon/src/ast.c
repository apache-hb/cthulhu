#include "oberon/ast.h"
#include "oberon/scan.h"

#include "report/report.h"

#include "std/str.h"

#include "base/memory.h"
#include "base/panic.h"

static void ensure_block_names_match(scan_t *scan, const node_t *node, const char *type, const char *name, const char *end)
{
    obr_scan_t *data = scan_get(scan);

    if (end == NULL) { return; }

    if (!str_equal(name, end))
    {
        message_t *id = report(data->reports, eWarn, node, "mismatching %s block BEGIN and END names", type);
        report_note(id, "BEGIN name `%s` does not match END name `%s`", name, end);
    }
}

static obr_t *obr_new(scan_t *scan, where_t where, obr_kind_t kind)
{
    CTASSERT(scan != NULL);

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

obr_t *obr_module(scan_t *scan, where_t where, char *name, char *end, vector_t *imports, vector_t *decls)
{
    obr_t *self = obr_decl(scan, where, eObrModule, name);
    self->imports = imports;
    self->decls = decls;

    ensure_block_names_match(scan, self->node, "MODULE", name, end);

    return self;
}

obr_t *obr_import(scan_t *scan, where_t where, char *name, char *symbol)
{
    obr_t *self = obr_decl(scan, where, eObrImport, name);
    self->symbol = symbol;
    return self;
}

obr_t *obr_decl_global(scan_t *scan, where_t where, bool mut, char *name, obr_t *type, obr_t *value)
{
    obr_t *self = obr_decl(scan, where, eObrDeclGlobal, name);
    self->mut = mut;
    self->type = type;
    self->value = value;
    return self;
}

obr_t *obr_decl_procedure(scan_t *scan, where_t where, char *name, char *end)
{
    obr_t *self = obr_decl(scan, where, eObrDeclProcedure, name);

    ensure_block_names_match(scan, self->node, "PROCEDURE", name, end);

    return self;
}

obr_t *obr_type_name(scan_t *scan, where_t where, char *name)
{
    obr_t *self = obr_new(scan, where, eObrTypeName);
    self->name = name;
    return self;
}

obr_t *obr_type_qual(scan_t *scan, where_t where, char *name, char *symbol)
{
    obr_t *self = obr_decl(scan, where, eObrTypeQual, name);
    self->symbol = symbol;
    return self;
}

/* extras */

obr_partial_value_t *obr_partial_value(scan_t *scan, where_t where, char *name)
{
    obr_partial_value_t *self = ctu_malloc(sizeof(obr_partial_value_t));
    self->scan = scan;
    self->where = where;
    self->name = name;
    return self;
}

vector_t *obr_expand_values(bool mut, vector_t *names, obr_t *type)
{
    size_t len = vector_len(names);
    vector_t *values = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        obr_partial_value_t *value = vector_get(names, i);
        obr_t *decl = obr_decl_global(value->scan, value->where, mut, value->name, type, NULL);
        vector_set(values, i, decl);
    }
    return values;
}
