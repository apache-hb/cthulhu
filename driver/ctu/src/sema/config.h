#pragma once

#include <stdbool.h>

typedef struct vector_t vector_t;
typedef struct argparse_t argparse_t;

typedef enum feature_t {
    eFeatureDefaultInit,

    eFeatureTotal
} feature_t;

vector_t *config_get_groups(void);
void config_init(argparse_t *args);

bool config_get_feature(feature_t feature);
