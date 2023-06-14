#pragma once

#include <stdbool.h>

typedef struct driver_t driver_t;
typedef struct ap_t ap_t;

typedef enum feature_t {
    eFeatureDefaultExpr,
    eFeatureTemplateDecls,
    eFeatureInterfaceDecls,

    eFeatureTotal
} feature_t;

void ctu_config(driver_t *handle, ap_t *args);

bool ctu_has_feature(feature_t feature);
