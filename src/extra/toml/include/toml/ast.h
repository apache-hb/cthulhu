#pragma once

#include "scan/node.h"

typedef enum toml_kind_t
{
    eTomlString,
    eTomlInteger,
    eTomlFloat,
    eTomlBoolean,
    eTomlDatetime,
    eTomlArray,
    eTomlTable,
    eTomlInlineTable,
    eTomlInlineArray
} toml_kind_t;

typedef struct toml_ast_t
{
    toml_kind_t kind;
    const node_t *node;
} toml_ast_t;
