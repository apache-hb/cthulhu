#include "hlir.h"

///
/// struct builders
///

hlir_t *hlir_begin_struct(node_t node, const char *name);
void hlir_build_struct(hlir_t *self);
hlir_t *hlir_struct(node_t node, const char *name, vector_t *fields);

///
/// union builders
///

hlir_t *hlir_begin_union(node_t node, const char *name);
void hlir_build_union(hlir_t *self);
hlir_t *hlir_union(node_t node, const char *name, vector_t *fields);

///
/// aggregate builders
///

void hlir_add_field(hlir_t *self, hlir_t *field);

///
/// alias builders
///

hlir_t *hlir_begin_alias(node_t node, const char *name);
void hlir_build_alias(hlir_t *self, const hlir_t *alias, bool newtype);
hlir_t *hlir_alias(node_t node, const char *name, const hlir_t *type, bool newtype);

///
/// global builders
///

hlir_t *hlir_begin_global(node_t node, const char *name, const hlir_t *type);
void hlir_build_global(hlir_t *self, const hlir_t *init);
hlir_t *hlir_global(node_t node, const char *name, const hlir_t *type, const hlir_t *init);

///
/// local builders
///

hlir_t *hlir_local(node_t node, const char *name, const hlir_t *type);

///
/// param builders
///

hlir_t *hlir_param(node_t node, const char *name, const hlir_t *type);

///
/// function builders
///

typedef struct
{
    vector_t *params;
    const hlir_t *result;
    bool variadic;
} signature_t;

hlir_t *hlir_begin_function(node_t node, const char *name, signature_t signature);
void hlir_build_function(hlir_t *self, hlir_t *body);
hlir_t *hlir_function(node_t node, const char *name, signature_t signature, vector_t *locals, hlir_t *body);

///
/// extra function details
///

void hlir_add_local(hlir_t *self, hlir_t *local);

///
/// module builders
///

hlir_t *hlir_begin_module(node_t node, const char *name);
void hlir_build_module(hlir_t *self, vector_t *types, vector_t *globals, vector_t *functions);
void hlir_update_module(hlir_t *self, vector_t *types, vector_t *globals, vector_t *functions);
hlir_t *hlir_module(node_t node, const char *name, vector_t *types, vector_t *globals, vector_t *functions);

///
/// decl modification
///

void hlir_set_attributes(hlir_t *self, const hlir_attributes_t *attributes);
void hlir_set_parent(hlir_t *self, const hlir_t *parent);

/**
 * @brief set the type of a declaration or expression
 * 
 * @note use this with great care.
 * 
 * @param self the node to change the type of
 * @param type the type to update to
 */
void hlir_set_type(hlir_t *self, const hlir_t *type);
