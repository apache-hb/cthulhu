#pragma once

#include <stdbool.h>

typedef struct lifetime_t lifetime_t;
typedef struct ap_t ap_t;

typedef enum feature_t {
    eFeatureDefaultExpr,
    eFeatureTemplateDecls,
    eFeatureInterfaceDecls,
    eFeatureNewTypes,

    eFeatureTotal
} feature_t;

void ctu_config(lifetime_t *lifetime, ap_t *args);

bool ctu_has_feature(feature_t feature);
