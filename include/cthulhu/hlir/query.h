#include "hlir.h"

///
/// general queries
///

hlir_kind_t get_hlir_kind(const hlir_t *hlir);
const hlir_t *get_hlir_type(const hlir_t *hlir);
const char *get_hlir_name(const hlir_t *hlir);
const hlir_attributes_t *get_hlir_attributes(const hlir_t *hlir);
const node_t *get_hlir_node(const hlir_t *hlir);
const hlir_t *get_hlir_parent(const hlir_t *hlir);
bool hlir_is(const hlir_t *hlir, hlir_kind_t kind);
bool hlir_will_be(const hlir_t *hlir, hlir_kind_t kind);
bool hlis_is_or_will_be(const hlir_t *hlir, hlir_kind_t kind);

///
/// detail queries
///

bool hlir_is_type(const hlir_t *hlir);
bool hlir_is_decl(const hlir_t *hlir);

///
/// debugging queries
///

const char *hlir_kind_to_string(hlir_kind_t kind);
const char *hlir_sign_to_string(sign_t sign);
const char *hlir_digit_to_string(digit_t digit);
