#include "common.h"

hlir_t *hlir_digit(node_t node, const char *name, digit_t width, sign_t sign)
{
    hlir_t *hlir = hlir_new_decl(node, name, kMetaType, HLIR_DIGIT);
    hlir->width = width;
    hlir->sign = sign;
    return hlir;
}

hlir_t *hlir_bool(node_t node, const char *name)
{
    return hlir_new_decl(node, name, kMetaType, HLIR_BOOL);
}

hlir_t *hlir_string(node_t node, const char *name)
{
    return hlir_new_decl(node, name, kMetaType, HLIR_STRING);
}

hlir_t *hlir_void(node_t node, const char *name)
{
    return hlir_new_decl(node, name, kMetaType, HLIR_VOID);
}

hlir_t *hlir_closure(node_t node, const char *name, vector_t *params, const hlir_t *result, bool variadic)
{
    hlir_t *hlir = hlir_new_decl(node, name, kMetaType, HLIR_CLOSURE);
    hlir->params = params;
    hlir->result = result;
    hlir->variadic = variadic;
    return hlir;
}

hlir_t *hlir_pointer(node_t node, const char *name, hlir_t *ptr, bool indexable)
{
    hlir_t *hlir = hlir_new_decl(node, name, kMetaType, HLIR_POINTER);
    hlir->ptr = ptr;
    hlir->indexable = indexable;
    return hlir;
}

hlir_t *hlir_array(node_t node, const char *name, hlir_t *element, hlir_t *length)
{
    hlir_t *hlir = hlir_new_decl(node, name, kMetaType, HLIR_ARRAY);
    hlir->element = element;
    hlir->length = length;
    return hlir;
}
