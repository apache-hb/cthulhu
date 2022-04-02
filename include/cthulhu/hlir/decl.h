#include "hlir.h"

///
/// struct builders
///

hlir_t *hlir_begin_struct(const node_t *node, const char *name);
hlir_t *hlir_build_struct(hlir_t *self);
hlir_t *hlir_struct(const node_t *node, const char *name, vector_t *fields);

void hlir_add_field(hlir_t *self, hlir_t *field);

///
/// global builders
///

hlir_t *hlir_begin_global(const node_t *node, const char *name, const hlir_t *type);
void hlir_build_global(hlir_t *self, const hlir_t *init);
hlir_t *hlir_global(const node_t *node, const char *name, const hlir_t *type, const hlir_t *init);

///
/// local builders
///

hlir_t *hlir_local(const node_t *node, const char *name, const hlir_t *type);

///
/// function builders
///

hlir_t *hlir_begin_function(const node_t *node, const char *name, vector_t *params, bool variadic, const hlir_t *result);
void hlir_build_function(hlir_t *self, const hlir_t *body);
hlir_t *hlir_function(const node_t *node, const char *name, const hlir_t *type, vector_t *locals, const hlir_t *body);

///
/// extra function details
///

void hlir_add_local(hlir_t *self, const hlir_t *local);
hlir_t *hlir_get_param(const hlir_t *self, const char *name);

///
/// module builders
///

hlir_t *hlir_begin_module(const node_t *node, const char *name);
void hlir_build_module(hlir_t *self, vector_t *types, vector_t *globals, vector_t *functions);

///
/// decl modification
///

void hlir_set_attributes(hlir_t *self, const hlir_attributes_t *attributes);
