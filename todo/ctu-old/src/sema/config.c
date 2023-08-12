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

typedef struct ctu_config_t
{
    bool features[eFeatureTotal];
} ctu_config_t;

static const char *kDefaultInitNames[] = { "--ctu:enable-default-expr", "-ctu:default", NULL };
static const char *kTemplateNames[] = { "--ctu:enable-templates", "-ctu:templates", NULL };
static const char *kInterfaceNames[] = { "--ctu:enable-interfaces", "-ctu:interfaces", NULL };
static const char *kNewTypeNames[] = { "--ctu:enable-newtypes", "-ctu:newtypes", NULL };

static ctu_config_t *gConfig = NULL;

static AP_EVENT(on_feature, ap, param, value, data)
{
    CTU_UNUSED(ap);
    CTU_UNUSED(param);

    bool *feature = data;
    const bool *update = value;

    *feature = *update;

    return eEventHandled;
}

static ctu_config_t *new_config(void)
{
    ctu_config_t *config = ctu_malloc(sizeof(ctu_config_t));
    memset(config->features, 0, sizeof(config->features));

    return config;
}

void ctu_config(lifetime_t *lifetime, ap_t *args)
{
    CTU_UNUSED(lifetime);

    GLOBAL_INIT();

    gConfig = new_config();
    ap_group_t *group = ap_group_new(args, "cthulhu features", "unstable cthulhu features and extensions");
    ap_param_t *defaultEnable = ap_add_bool(group, "default expressions", "enables `default` in expressions to explicitly refer to a types default value (experimental)", kDefaultInitNames);
    ap_param_t *templateEnable = ap_add_bool(group, "templates", "enables templated types and functions (unimplemented)", kTemplateNames);
    ap_param_t *interfaceEnable = ap_add_bool(group, "interfaces", "enables interface declarations (unimplemented)", kInterfaceNames);
    ap_param_t *newTypeEnable = ap_add_bool(group, "newtypes", "enables newtype declarations (unimplemented)", kNewTypeNames);

    ap_event(args, defaultEnable, on_feature, &gConfig->features[eFeatureDefaultExpr]);
    ap_event(args, templateEnable, on_feature, &gConfig->features[eFeatureTemplateDecls]);
    ap_event(args, interfaceEnable, on_feature, &gConfig->features[eFeatureInterfaceDecls]);
    ap_event(args, newTypeEnable, on_feature, &gConfig->features[eFeatureNewTypes]);
}

bool ctu_has_feature(feature_t feature)
{
    CTASSERT(gConfig != NULL);
    CTASSERT(feature < eFeatureTotal);

    return gConfig->features[feature];
}
