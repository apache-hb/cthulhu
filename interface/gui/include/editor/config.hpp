#pragma once

#include "config/config.h"

namespace ed
{
    void draw_config_panel(config_t *config);

    template<typename T>
    struct ConfigField
    {
        cfg_field_t *field = nullptr;
    };

    struct ConfigGroup
    {
        ConfigGroup(config_t *parent, const cfg_info_t *info)
        {
            group = config_group(parent, info);
        }

        config_t *group = nullptr;
    };

    struct ReportConfig : public ConfigGroup
    {
        static inline const cfg_info_t kInfo = {
            .name = "Report",
            .description = "Report settings"
        };

        ReportConfig(config_t *parent);

        ConfigField<int> warning_level;
        ConfigField<bool> werror;
    };

    struct TestConfig : public ConfigGroup
    {
        TestConfig(config_t *parent);

        ConfigField<int> test_int;
        ConfigField<bool> test_bool;
        ConfigField<const char*> test_string;
        ConfigField<size_t> test_enum;
        ConfigField<size_t> test_flags;
    };
}
