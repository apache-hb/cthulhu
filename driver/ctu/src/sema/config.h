#pragma once

#include <stdbool.h>

typedef struct vector_t vector_t;
typedef struct argparse_t argparse_t;

typedef enum feature_t {
    eFeatureDefaultInit,

    eFeatureTotal
} feature_t;

void ctu_add_commands(vector_t **groups);
void config_init(argparse_t *args);

bool config_get_feature(feature_t feature);
