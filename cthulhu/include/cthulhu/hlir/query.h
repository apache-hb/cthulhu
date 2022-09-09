#include "hlir.h"

///
/// general queries
///

NODISCARD PUREFN hlir_kind_t get_hlir_kind(const hlir_t *hlir);

NODISCARD PUREFN const hlir_t *get_hlir_type(const hlir_t *hlir);

NODISCARD PUREFN const char *get_hlir_name(const hlir_t *hlir);

NODISCARD PUREFN const hlir_attributes_t *get_hlir_attributes(const hlir_t *hlir);

NODISCARD PUREFN node_t *get_hlir_node(const hlir_t *hlir);

NODISCARD PUREFN bool hlir_is(const hlir_t *hlir, hlir_kind_t kind);

///
/// detail queries
///

/**
 * @brief follow a type until either a newtype or a real type is reached
 *
 * @param hlir the type to follow
 * @return const hlir_t* the base type
 */
const hlir_t *hlir_follow_type(const hlir_t *hlir);

/**
 * @brief follow a type until a real type is reached, ignoring newtypes
 *
 * @param hlir the type to follow
 * @return const hlir_t* the base type
 */
const hlir_t *hlir_real_type(const hlir_t *hlir);

bool hlir_is_type(const hlir_t *hlir);
bool hlir_is_decl(const hlir_t *hlir);

bool hlir_types_equal(const hlir_t *lhs, const hlir_t *rhs);

///
/// debugging queries
///

NODISCARD PUREFN const char *hlir_kind_to_string(hlir_kind_t kind);

NODISCARD PUREFN const char *hlir_sign_to_string(sign_t sign);

NODISCARD PUREFN const char *hlir_digit_to_string(digit_t digit);
