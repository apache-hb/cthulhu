#pragma once

#include "config/config.h"
#include <type_traits>

namespace ed
{
    void draw_config_panel(config_t *config);

    class ConfigField
    {
    protected:
        cfg_field_t *field = nullptr;
    };

    class ConfigGroup
    {
    public:
        config_t *get_group() const { return group; }

    protected:
        ConfigGroup(config_t *parent, const cfg_info_t *info);

        config_t *group = nullptr;
    };

    class ConfigBool : ConfigField
    {
        bool get_value() const;
    };

    class ConfigInt : ConfigField
    {
        int get_value() const;
    };

    class ConfigString : ConfigField
    {
        const char *get_value() const;
    };

    template<typename T>
    class ConfigEnum : ConfigField
    {
        static_assert(std::is_enum_v<T>, "T must be an enum type");

    public:
        ConfigEnum(const ConfigGroup& group, const cfg_info_t *info, const cfg_enum_t *enum_config)
        {
            field = cfg_enum_create(group.get_group(), info, enum_config);
        }

        T get_value() const;
    };

    template<typename T>
    class ConfigFlags : ConfigField
    {
        static_assert(std::is_enum_v<T>, "T must be an enum type");

    public:
        ConfigFlags(const ConfigGroup& group, const cfg_info_t *info, const cfg_flags_t *flag_config)
        {
            field = cfg_flags_create(group.get_group(), info, flag_config);
        }

        T get_value() const;
    };

    template<typename T>
    class ConfigFieldBuilder
    {
    public:
        ConfigFieldBuilder(arena_t *memory)
        {
            arena = memory;
            choices = typevec_new(sizeof(cfg_choice_t), 4, arena);
        }

        ConfigFieldBuilder& add_field(const char *name, T value)
        {
            cfg_choice_t choice = {
                .text = name,
                .value = (size_t)value
            };
            typevec_push(choices, &choice);

            return *this;
        }

        ConfigEnum<T> build_enum(const ConfigGroup& group, const cfg_info_t *info, T initial) const
        {
            cfg_enum_t cfg_enum = {
                .options = (cfg_choice_t*)typevec_data(choices),
                .count = typevec_len(choices),
                .initial = (size_t)initial
            };

            return ConfigEnum<T>(group, info, &cfg_enum);
        }

        ConfigFlags<T> build_flags(const ConfigGroup& group, const cfg_info_t *info, T initial) const
        {
            cfg_flags_t cfg_flags = {
                .options = (cfg_choice_t*)typevec_data(choices),
                .count = typevec_len(choices),
                .initial = (size_t)initial
            };

            return ConfigFlags<T>(group, info, &cfg_flags);
        }

    private:
        arena_t *arena = nullptr;
        typevec_t *choices = nullptr;
    };

    class ReportConfig : public ConfigGroup
    {
    public:
        ReportConfig(config_t *parent);

        ConfigInt warning_level;
        ConfigBool werror;
    };

    enum test_enum_t
    {
        eTestEnumA,
        eTestEnumB,
        eTestEnumC,

        eTestEnumTotal
    };

    enum test_flags_t
    {
        eTestFlagA = 1 << 0,
        eTestFlagB = 1 << 1,
        eTestFlagC = 1 << 2,
    };

    class TestConfig : public ConfigGroup
    {
    public:
        TestConfig(config_t *parent);

        ConfigInt test_int;
        ConfigBool test_bool;
        ConfigString test_string;
        ConfigEnum<test_enum_t> test_enum;
        ConfigFlags<test_flags_t> test_flags;
    };
}
