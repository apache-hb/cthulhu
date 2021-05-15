#include "ir.h"

#include <stdlib.h>

unit_t *new_unit(void) {
    unit_t *unit = malloc(sizeof(unit_t));
    unit->size = 8;
    unit->length = 0;
    unit->ops = malloc(sizeof(opcode_t) * unit->size);
    return unit;
}

size_t unit_add(unit_t *unit, opcode_t op) {
    if (unit->length + 1 > unit->size) {
        unit->size += 8;
        unit->ops = realloc(unit->ops, sizeof(opcode_t) * unit->size);
    }
    unit->ops[unit->length] = op;
    return unit->length++;
}
