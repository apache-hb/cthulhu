#include "cthulhu/hlir/decl.h"

#include "common.h"

#define CHECK_NULL(expr) CTASSERT(expr != NULL, "null pointer")
#define IS_AGGREGATE(type) (type == HLIR_STRUCT || type == HLIR_UNION)

///
/// builder functions
///

static hlir_t *hlir_begin_aggregate_with_fields(const node_t *node, const char *name, vector_t *fields, hlir_type_t type) {
    hlir_t *self = hlir_new_forward(node, name, TYPE, type);
    self->fields = fields;
    return self;
}

static hlir_t *hlir_begin_aggregate(const node_t *node, const char *name, hlir_type_t type) {
    return hlir_begin_aggregate_with_fields(node, name, vector_new(4), type);
}

static void hlir_finish(hlir_t *self, hlir_type_t type) {
    CHECK_NULL(self);
    CTASSERT(hlir_is(self, HLIR_FORWARD), "hlir-finish called on non-forward hlir");
    CTASSERT(self->expected == type, "hlir-finish called with wrong type");

    self->type = type;
}

///
/// struct interface
///

hlir_t *hlir_begin_struct(const node_t *node, const char *name) {
    return hlir_begin_aggregate(node, name, HLIR_STRUCT);
}

hlir_t *hlir_build_struct(hlir_t *self) {
    hlir_finish(self, HLIR_STRUCT);
}

hlir_t *hlir_struct(const node_t *node, const char *name, vector_t *fields) {
    hlir_t *self = hlir_begin_aggregate_with_fields(node, name, fields, HLIR_STRUCT);
    hlir_build_struct(self);
    return self;
}

///
/// union interface
///

hlir_t *hlir_begin_union(const node_t *node, const char *name) {
    return hlir_begin_aggregate(node, name, HLIR_UNION);
}

hlir_t *hlir_build_union(hlir_t *self) {
    hlir_finish(self, HLIR_UNION);
}

hlir_t *hlir_union(const node_t *node, const char *name, vector_t *fields) {
    hlir_t *self = hlir_begin_aggregate_with_fields(node, name, fields, HLIR_UNION);
    hlir_build_union(self);
    return self;
}

///
/// generic aggregate interface
///

void hlir_add_field(hlir_t *self, hlir_t *field) {
    CHECK_NULL(self);
    CTASSERT(IS_AGGREGATE(self->type), "hlir-add-field called on non-aggregate hlir");
    vector_push(&self->fields, field);
}

///
/// alias interface
///

hlir_t *hlir_begin_alias(const node_t *node, const char *name) {
    return hlir_new_forward(node, name, TYPE, HLIR_ALIAS);
}

hlir_t *hlir_build_alias(hlir_t *self, const hlir_t *type) {
    hlir_finish(self, HLIR_ALIAS);
    self->alias = type;
}

hlir_t *hlir_alias(const node_t *node, const char *name, const hlir_t *type) {
    hlir_t *self = hlir_begin_alias(node, name);
    hlir_build_alias(self, type);
    return self;
}

hlir_t *hlir_begin_global(const node_t *node, const char *name, const hlir_t *type) {
    return hlir_new_forward(node, name, type, HLIR_GLOBAL);
}

void hlir_build_global(hlir_t *self, const hlir_t *init) {
    hlir_finish(self, HLIR_GLOBAL);
    self->value = init;
}

hlir_t *hlir_global(const node_t *node, const char *name, const hlir_t *type, const hlir_t *init) {
    hlir_t *self = hlir_begin_global(node, name, type);
    hlir_build_global(self, init);
    return self;
}



hlir_t *hlir_local(const node_t *node, const char *name, const hlir_t *type);


hlir_t *hlir_begin_function(const node_t *node, const char *name, signature_t signature);
void hlir_build_function(hlir_t *self, const hlir_t *body);
hlir_t *hlir_function(const node_t *node, const char *name, signature_t signature, vector_t *locals, const hlir_t *body);



void hlir_add_local(hlir_t *self, const hlir_t *local);
hlir_t *hlir_get_param(const hlir_t *self, const char *name);

hlir_t *hlir_begin_module(const node_t *node, const char *name) {
    return hlir_new_forward(node, name, NULL, HLIR_MODULE);
}

void hlir_build_module(hlir_t *self, vector_t *types, vector_t *globals, vector_t *functions) {
    hlir_finish(self, HLIR_MODULE);

    self->types = types;
    self->globals = globals;
    self->functions = functions;
}

hlir_t *hlir_module(const node_t *node, const char *name, vector_t *types, vector_t *globals, vector_t *functions) {
    hlir_t *self = hlir_begin_module(node, name);
    hlir_build_module(self, types, globals, functions);
    return self;
}
