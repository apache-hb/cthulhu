#include "hlir.h"

///
/// type constructors
///

/**
 * @brief create a new integer type
 * 
 * @param node where this type was defined
 * @param name the name of this integer type
 * @param width the width of this integer type
 * @param sign the sign of this integer type
 * @return hlir_t* the constructed integer type
 */
hlir_t *hlir_digit(IN_OPT const node_t *node, 
                   IN_OPT const char *name, 
                   digit_t width, 
                   sign_t sign);

/**
 * @brief construct a new bool type
 * 
 * @param node the node where this type was defined
 * @param name the name of this bool type
 * @return hlir_t* the constructed bool type
 */
hlir_t *hlir_bool(IN_OPT const node_t *node, 
                  IN_OPT const char *name);

/**
 * @brief construct a new string type
 * 
 * @param node the node where this type was defined
 * @param name the name of this string type
 * @return hlir_t* the constructed string type
 */
hlir_t *hlir_string(IN_OPT const node_t *node, 
                    IN_OPT const char *name);

/**
 * @brief construct a new void type
 * 
 * @param node the node where this type was defined
 * @param name the name of this void type
 * @return hlir_t* 
 */
hlir_t *hlir_void(IN_OPT const node_t *node, 
                  IN_OPT const char *name);

/**
 * @brief construct a new closure type
 * 
 * @param node the node where this type was defined
 * @param name the name of this closure type
 * @param params the parameters of this closure type
 * @param result the result of this closure type
 * @param variadic is this a variadic function?
 * @return hlir_t* the constructed closure type
 */
hlir_t *hlir_closure(IN_OPT const node_t *node, 
                     IN_OPT const char *name, 
                     IN vector_t *params, 
                     IN const hlir_t *result, 
                     bool variadic);

/**
 * @brief construct a new pointer type
 * 
 * @param node the node where this type was defined
 * @param name the name of this pointer type
 * @param type the type this pointer points to
 * @param indexable can this pointer be indexed?
 * @return hlir_t* the constructed pointer type
 */
hlir_t *hlir_pointer(IN_OPT const node_t *node, 
                     IN_OPT const char *name, 
                     IN hlir_t *type, 
                     bool indexable);

/**
 * @brief construct a new array type
 * 
 * @param node the node where this type was defined
 * @param name the name of this array type
 * @param element the element type of this array
 * @param length the length of this array
 * @return hlir_t* the constructed array type
 */
hlir_t *hlir_array(IN_OPT const node_t *node, 
                   IN_OPT const char *name, 
                   IN hlir_t *element, 
                   IN hlir_t *length);
