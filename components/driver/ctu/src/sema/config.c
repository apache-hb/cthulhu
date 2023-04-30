#include "config.h"

#include "argparse/argparse.h"
#include "std/vector.h"
#include "base/panic.h"

typedef struct feature_data_t {
    bool defaultValue;

    param_t *param;
} feature_data_t;

static bool gFeaturesInit = false;
static feature_data_t gFeatures[eFeatureTotal];

static const char *kDefaultInitNames[] = { "--ctu-enable-default-expr", "-Fctu-default-expr" };
#define DEFAULT_INIT_NAMES (sizeof(kDefaultInitNames) / sizeof(const char *))

static void init_features(void)
{
    if (gFeaturesInit) { return; }
    gFeaturesInit = true;

    feature_data_t defaultInitFeature = {
        .defaultValue = false,
        .param = bool_param("enables `default` in expressions to refer to an implicit types default value", kDefaultInitNames, DEFAULT_INIT_NAMES)
    };

    gFeatures[eFeatureDefaultInit] = defaultInitFeature;
}

void ctu_add_commands(vector_t **groups)
{
    init_features();

    vector_t *params = vector_of(eFeatureTotal);
    for (size_t i = 0; i < eFeatureTotal; i++) 
    {
        feature_data_t data = gFeatures[i];
        vector_set(params, i, data.param);
    }

    group_t *group = group_new("cthulhu features", "unstable cthulhu features and extensions", params);

    vector_push(groups, group);
}

static argparse_t *gArgParse = NULL;

void config_init(argparse_t *args)
{
    gArgParse = args;
}

bool config_get_feature(feature_t feature)
{
    CTASSERTM(gFeaturesInit, "config_get_groups must be called before config_get_feature");
    CTASSERTM(gArgParse != NULL, "config_init must be called before config_get_feature");

    feature_data_t data = gFeatures[feature];
    return get_bool_arg(gArgParse, data.param, data.defaultValue);
}
