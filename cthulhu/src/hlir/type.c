#include "common.h"

#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/type.h"
#include "cthulhu/hlir/decl.h"

hlir_t *hlir_digit(node_t *node, const char *name, digit_t width, sign_t sign)
{
    hlir_t *hlir = hlir_decl_new(node, name, kMetaType, eHlirDigit);
    hlir->width = width;
    hlir->sign = sign;
    return hlir;
}

hlir_t *hlir_decimal(node_t *node, const char *name)
{
    return hlir_decl_new(node, name, kMetaType, eHlirDecimal);
}

hlir_t *hlir_bool(node_t *node, const char *name)
{
    return hlir_decl_new(node, name, kMetaType, eHlirBool);
}

hlir_t *hlir_string(node_t *node, const char *name)
{
    return hlir_decl_new(node, name, kMetaType, eHlirString);
}

hlir_t *hlir_unit(node_t *node, const char *name)
{
    return hlir_decl_new(node, name, kMetaType, eHlirUnit);
}

hlir_t *hlir_empty(node_t *node, const char *name)
{
    return hlir_decl_new(node, name, kMetaType, eHlirEmpty);
}

hlir_t *hlir_opaque(node_t *node, const char *name)
{
    return hlir_decl_new(node, name, kMetaType, eHlirOpaque);
}

hlir_t *hlir_closure(node_t *node, const char *name, vector_t *params, const hlir_t *result)
{
    hlir_t *hlir = hlir_decl_new(node, name, kMetaType, eHlirClosure);
    hlir->params = params;
    hlir->result = result;
    return hlir;
}

hlir_t *hlir_pointer(node_t *node, const char *name, const hlir_t *ptr, bool indexable)
{
    hlir_t *hlir = hlir_decl_new(node, name, kMetaType, eHlirPointer);
    hlir->ptr = ptr;
    hlir->indexable = indexable;
    return hlir;
}

hlir_t *hlir_array(reports_t *reports, node_t *node, const char *name, hlir_t *element, hlir_t *length)
{
    node_t *error = check_const_expr(reports, length);
    if (node_is_valid(error))
    {
        report(reports, eFatal, error, "array length must be a constant expression");
        return hlir_error(node, "array length must be a constant expression");
    }

    const hlir_t *lengthType = get_hlir_type(length);

    if (!hlir_is(lengthType, eHlirDigit))
    {
        report(reports, eFatal, get_hlir_node(length), "array length must be a digit");
        return hlir_error(node, "array length must be a digit");
    }

    hlir_t *hlir = hlir_decl_new(node, name, kMetaType, eHlirArray);
    hlir->element = element;
    hlir->length = length;
    return hlir;
}

hlir_t *hlir_qualified(const hlir_t *type, hlir_attributes_t *attribs)
{
    hlir_t *hlir = hlir_decl_new(get_hlir_node(type), get_hlir_name(type), type, eHlirQualified);
    hlir_set_attributes(hlir, attribs);
    return hlir;
}

hlir_t *hlir_va_args(node_t *node, const char *name)
{
    return hlir_decl_new(node, name, kMetaType, eHlirVaArgs);
}

hlir_t *hlir_va_list(node_t *node, const char *name)
{
    return hlir_decl_new(node, name, kMetaType, eHlirVaList);
}
