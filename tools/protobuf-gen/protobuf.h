#pragma once

#include "scan/node.h"

#include <stdint.h>

typedef struct vector_t vector_t;

typedef enum astof_t {
    eAstModule,
    eAstMessage,
    eAstField,
    eAstName,
    eAstEnum,
    eAstCase,
    eAstUnion
} astof_t;

typedef enum field_kind_t
{
    eFieldRequired,
    eFieldOptional,
    eFieldRepeated,
    
    eFieldTotal
} field_kind_t;

typedef struct ast_t {
    astof_t kind;
    node_t node;

    union {
        vector_t *messages;

        struct {
            const char *name;

            union {
                vector_t *fields;

                struct {
                    uint_least32_t index;
                    const char *type;
                    field_kind_t field;
                };
            };
        };
    };
} ast_t;

ast_t *pb_module(scan_t scan, where_t where, vector_t *messages);
ast_t *pb_message(scan_t scan, where_t where, const char *name, vector_t *fields);
ast_t *pb_field(scan_t scan, where_t where, const char *name, uint_least32_t index, const char *type, field_kind_t field);
ast_t *pb_enum(scan_t scan, where_t where, const char *name, vector_t *fields);
ast_t *pb_case(scan_t scan, where_t where, const char *name, uint_least32_t index);
ast_t *pb_oneof(scan_t scan, where_t where, const char *name, vector_t *fields);

uint_least32_t field_id(scan_t *scan, where_t where, const char *text);
