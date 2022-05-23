#include "common.h"

#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/type.h"

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

hlir_t *hlir_array(reports_t *reports, node_t node, const char *name, hlir_t *element, hlir_t *length)
{
    node_t error = check_const_expr(reports, length);
    if (node_is_valid(error))
    {
        report(reports, ERROR, error, "array length must be a constant expression");
        return hlir_error(node, "array length must be a constant expression");
    }

    const hlir_t *lengthType = get_hlir_type(length);

    if (!hlir_is(lengthType, HLIR_DIGIT))
    {
        report(reports, ERROR, get_hlir_node(length), "array length must be a digit");
        return hlir_error(node, "array length must be a digit");
    }

    hlir_t *hlir = hlir_new_decl(node, name, kMetaType, HLIR_ARRAY);
    hlir->element = element;
    hlir->length = length;
    return hlir;
}
