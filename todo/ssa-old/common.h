#pragma once

#include "cthulhu/ssa/ssa.h"

typedef struct h2_t h2_t;

const char *ssa_opcode_name(ssa_opcode_t op);
const char *ssa_operand_name(ssa_operand_kind_t op);
const char *ssa_kind_name(ssa_kind_t kind);

const ssa_type_t *ssa_get_step_type(ssa_step_t step);

ssa_kind_t ssa_get_value_kind(const ssa_value_t *value);

ssa_param_t *ssa_param_new(const char *name, const ssa_type_t *type);

const char *ssa_type_to_string(const ssa_type_t *type);