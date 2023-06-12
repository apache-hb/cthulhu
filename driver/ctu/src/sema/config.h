#pragma once

#include <stdbool.h>

typedef struct lang_handle_t lang_handle_t;
typedef struct ap_t ap_t;

typedef enum feature_t {
    eFeatureDefaultExpr,
    eFeatureTemplateDecls,
    eFeatureInterfaceDecls,

    eFeatureTotal
} feature_t;

void ctu_config(lang_handle_t *handle, ap_t *args);

bool ctu_has_feature(lang_handle_t *handle, feature_t feature);
