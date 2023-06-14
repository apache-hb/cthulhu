#include "config.h"

#include "cthulhu/mediator/driver.h"
#include "argparse/commands.h"

#include "base/panic.h"
#include "base/memory.h"

#include <string.h>

typedef struct feature_data_t 
{
    bool defaultValue;

    ap_param_t *param;
} feature_data_t;

typedef struct config_t 
{
    bool features[eFeatureTotal];
} config_t;

static const char *kDefaultInitNames[] = { "--ctu:enable-default-expr", "-ctu:default", NULL };
static const char *kTemplateNames[] = { "--ctu:enable-templates", "-ctu:templates", NULL };
static const char *kInterfaceNames[] = { "--ctu:enable-interfaces", "-ctu:interfaces", NULL };

static config_t *gConfig = NULL;

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

void ctu_config(driver_t *handle, ap_t *args) 
{
    GLOBAL_INIT();

    gConfig = new_config();
    ap_group_t *group = ap_group_new(args, "cthulhu features", "unstable cthulhu features and extensions");
    ap_param_t *defaultEnable = ap_add_bool(group, "default expressions", "enables `default` in expressions to explicitly refer to a types default value (experimental)", kDefaultInitNames);
    ap_param_t *templateEnable = ap_add_bool(group, "templates", "enables templated types and functions (unimplemented)", kTemplateNames);
    ap_param_t *interfaceEnable = ap_add_bool(group, "interfaces", "enables interface declarations (unimplemented)", kInterfaceNames);

    ap_event(args, defaultEnable, on_feature, &gConfig->features[eFeatureDefaultExpr]);
    ap_event(args, templateEnable, on_feature, &gConfig->features[eFeatureTemplateDecls]);
    ap_event(args, interfaceEnable, on_feature, &gConfig->features[eFeatureInterfaceDecls]);
}

bool ctu_has_feature(feature_t feature)
{
    CTASSERT(gConfig != NULL);
    CTASSERT(feature < eFeatureTotal);

    return gConfig->features[feature];
}
