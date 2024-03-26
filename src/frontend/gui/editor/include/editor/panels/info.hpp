// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "editor/panels/panel.hpp"

typedef struct module_info_t module_info_t;
typedef struct frontend_t frontend_t;
typedef struct language_t language_t;
typedef struct plugin_t plugin_t;
typedef struct target_t target_t;

namespace ed
{
    class ModuleInfoPanel : public IEditorPanel
    {
        const module_info_t& info;

    protected:
        void draw_info();

    public:
        ModuleInfoPanel(const module_info_t& info);
    };

    class FrontendInfoPanel final : public ModuleInfoPanel
    {
        const frontend_t& frontend;

        // IEditorPanel
        void draw_content() override;
    public:
        FrontendInfoPanel(const frontend_t& info);
    };

    class LanguageInfoPanel final : public ModuleInfoPanel
    {
        const language_t& lang;

        std::string builtin;
        std::string extensions;

        // IEditorPanel
        void draw_content() override;
    public:
        LanguageInfoPanel(const language_t& lang);
    };

    class PluginInfoPanel final : public ModuleInfoPanel
    {
        const plugin_t& plugin;

        // IEditorPanel
        void draw_content() override;
    public:
        PluginInfoPanel(const plugin_t& plugin);
    };

    class TargetInfoPanel final : public ModuleInfoPanel
    {
        const target_t& target;

        // IEditorPanel
        void draw_content() override;
    public:
        TargetInfoPanel(const target_t& target);
    };
}