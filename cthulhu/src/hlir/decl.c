#include "cthulhu/hlir/decl.h"
#include "cthulhu/hlir/query.h"

#include "common.h"

#define IS_AGGREGATE(hlir) (hlis_is_or_will_be(hlir, eHlirStruct) || hlis_is_or_will_be(hlir, eHlirUnion))

///
/// builder functions
///

static hlir_t *hlir_begin_aggregate_with_fields(node_t node, const char *name, vector_t *fields, hlir_kind_t type)
{
    hlir_t *self = hlir_new_forward(node, name, kMetaType, type);
    self->fields = fields;
    return self;
}

static hlir_t *hlir_begin_aggregate(node_t node, const char *name, hlir_kind_t type)
{
    return hlir_begin_aggregate_with_fields(node, name, vector_new(4), type);
}

static void hlir_finish(hlir_t *self, hlir_kind_t type)
{
    CHECK_NULL(self);
    CTASSERT(hlir_is(self, eHlirForward), "hlir-finish called on non-forward hlir");
    CTASSERT(self->expected == type, "hlir-finish called with wrong type");

    self->type = type;
}

///
/// struct interface
///

hlir_t *hlir_begin_struct(node_t node, const char *name)
{
    return hlir_begin_aggregate(node, name, eHlirStruct);
}

void hlir_build_struct(hlir_t *self)
{
    hlir_finish(self, eHlirStruct);
}

hlir_t *hlir_struct(node_t node, const char *name, vector_t *fields)
{
    hlir_t *self = hlir_begin_aggregate_with_fields(node, name, fields, eHlirStruct);
    hlir_build_struct(self);
    return self;
}

///
/// union interface
///

hlir_t *hlir_begin_union(node_t node, const char *name)
{
    return hlir_begin_aggregate(node, name, eHlirUnion);
}

void hlir_build_union(hlir_t *self)
{
    hlir_finish(self, eHlirUnion);
}

hlir_t *hlir_union(node_t node, const char *name, vector_t *fields)
{
    hlir_t *self = hlir_begin_aggregate_with_fields(node, name, fields, eHlirUnion);
    hlir_build_union(self);
    return self;
}

///
/// generic aggregate interface
///

void hlir_add_field(hlir_t *self, hlir_t *field)
{
    CHECK_NULL(self);
    CTASSERT(IS_AGGREGATE(self), "hlir-add-field called on non-aggregate hlir");
    CTASSERT(hlir_is(field, eHlirField), "hlir-add-field called with non-field hlir");

    hlir_set_parent(field, self);
    vector_push(&self->fields, field);
}

///
/// alias interface
///

hlir_t *hlir_begin_alias(node_t node, const char *name)
{
    return hlir_new_forward(node, name, kMetaType, eHlirAlias);
}

void hlir_build_alias(hlir_t *self, const hlir_t *alias, bool newtype)
{
    hlir_finish(self, eHlirAlias);
    self->alias = alias;
    self->newtype = newtype;
}

hlir_t *hlir_alias(node_t node, const char *name, const hlir_t *type, bool newtype)
{
    hlir_t *self = hlir_begin_alias(node, name);
    hlir_build_alias(self, type, newtype);
    return self;
}

hlir_t *hlir_begin_global(node_t node, const char *name, const hlir_t *type)
{
    return hlir_new_forward(node, name, type, eHlirGlobal);
}

void hlir_build_global(hlir_t *self, const hlir_t *init)
{
    hlir_finish(self, eHlirGlobal);
    self->value = init;
}

hlir_t *hlir_global(node_t node, const char *name, const hlir_t *type, const hlir_t *init)
{
    hlir_t *self = hlir_begin_global(node, name, type);
    hlir_build_global(self, init);
    return self;
}

hlir_t *hlir_local(node_t node, const char *name, const hlir_t *type)
{
    return hlir_new_decl(node, name, type, eHlirLocal);
}

hlir_t *hlir_param(node_t node, const char *name, const hlir_t *type)
{
    return hlir_new_decl(node, name, type, eHlirParam);
}

static hlir_t *hlir_begin_function_with_locals(node_t node, const char *name, vector_t *locals, signature_t signature)
{
    hlir_t *self = hlir_new_forward(node, name, kMetaType, eHlirFunction);
    self->params = signature.params;
    self->result = signature.result;
    self->variadic = signature.variadic;
    self->locals = locals;
    self->of = self; // TODO: this should be a closure
    return self;
}

hlir_t *hlir_begin_function(node_t node, const char *name, signature_t signature)
{
    return hlir_begin_function_with_locals(node, name, vector_new(4), signature);
}

void hlir_build_function(hlir_t *self, hlir_t *body)
{
    hlir_finish(self, eHlirFunction);
    self->body = body;
}

hlir_t *hlir_function(node_t node, const char *name, signature_t signature, vector_t *locals, hlir_t *body)
{
    hlir_t *self = hlir_begin_function_with_locals(node, name, locals, signature);
    hlir_build_function(self, body);
    return self;
}

void hlir_add_local(hlir_t *self, hlir_t *local)
{
    CTASSERT(hlir_will_be(self, eHlirFunction), "hlir-add-local called on non-function hlir");
    vector_push(&self->locals, local);
}

hlir_t *hlir_begin_module(node_t node, const char *name)
{
    return hlir_new_forward(node, name, NULL, eHlirModule);
}

void hlir_build_module(hlir_t *self, vector_t *types, vector_t *globals, vector_t *functions)
{
    hlir_finish(self, eHlirModule);

    self->types = types;
    self->globals = globals;
    self->functions = functions;
}

void hlir_update_module(hlir_t *self, vector_t *types, vector_t *globals, vector_t *functions)
{
    self->types = types;
    self->globals = globals;
    self->functions = functions;
}

hlir_t *hlir_module(node_t node, const char *name, vector_t *types, vector_t *globals, vector_t *functions)
{
    hlir_t *self = hlir_begin_module(node, name);
    hlir_build_module(self, types, globals, functions);
    return self;
}
