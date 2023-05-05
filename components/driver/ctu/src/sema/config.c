#include "config.h"

#include "cthulhu/mediator/mediator.h"
#include "argparse2/commands.h"

#include "base/panic.h"
#include "base/memory.h"

#include <string.h>

typedef struct feature_data_t {
    bool defaultValue;

    ap_param_t *param;
} feature_data_t;

typedef struct config_t {
    bool features[eFeatureTotal];
} config_t;

static const char *kDefaultInitNames[] = { "--ctu-enable-default-expr", "-Fctu-default-expr", NULL };

static AP_EVENT(on_feature, ap, param, value, data)
{
    bool *feature = data;
    const bool *update = value;

    *feature = *update;

    return eEventHandled;
}

static config_t *new_config(void)
{
    config_t *config = ctu_malloc(sizeof(config_t));
    memset(config->features, 0, sizeof(config->features));

    return config;
}

void ctu_config_init(lang_handle_t *handle, ap_t *args) 
{
    config_t *config = new_config();
    
    ap_group_t *group = ap_group_new(args, "cthulhu features", "unstable cthulhu features and extensions");
    ap_param_t *defaultInit = ap_add_bool(group, "default expressions", "enables `default` in expressions to refer to an implicit types default value", kDefaultInitNames);

    ap_event(args, defaultInit, on_feature, &config->features[eFeatureDefaultExpr]);

    handle->user = config;
}

bool ctu_has_feature(lang_handle_t *handle, feature_t feature)
{
    CTASSERT(handle != NULL);
    CTASSERT(feature < eFeatureTotal);

    config_t *config = handle->user;

    return config->features[feature];
}
