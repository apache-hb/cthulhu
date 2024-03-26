// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "imgui.h"

#include <string>

namespace ed
{
    struct panel_info_t
    {
        bool open_on_start = false;
    };

    class ScopeID
    {
        ScopeID(const ScopeID&) = delete;
        ScopeID& operator=(const ScopeID&) = delete;

        ScopeID(ScopeID&&) = delete;
        ScopeID& operator=(ScopeID&&) = delete;
    public:
        ScopeID(const void *ptr) { ImGui::PushID(ptr); }
        ScopeID(int i) { ImGui::PushID(i); }
        ScopeID(const char *str) { ImGui::PushID(str); }
        ~ScopeID() { ImGui::PopID(); }
    };

    class IEditorPanel
    {
        std::string name;

        virtual void draw_content() { }

    protected:
        bool visible = true;
        bool enabled = true;
        ImGuiWindowFlags flags = ImGuiWindowFlags_None;

        void set_enabled(bool value) { enabled = value; }

    public:
        IEditorPanel(std::string_view name, panel_info_t setup = {});

        virtual ~IEditorPanel() = default;

        virtual void update() { }

        // draw the panel inside its own window
        virtual bool draw_window();

        // draw only the content of the panel
        void draw();

        // draw a menu item that toggles the visibility of the panel
        virtual bool menu_item(const char *shortcut = nullptr);

        const char *get_title() const { return name.c_str(); }
        bool is_visible() const { return visible; }
        bool is_enabled() const { return enabled; }
    };

    class ImGuiDemoPanel final : public IEditorPanel
    {
    public:
        // IEditorPanel
        bool draw_window() override;

        ImGuiDemoPanel(panel_info_t setup = {});
    };

    class ImPlotDemoPanel final : public IEditorPanel
    {
    public:
        // IEditorPanel
        bool draw_window() override;

        ImPlotDemoPanel(panel_info_t setup = {});
    };

    // draw the content with a SeperatorText(get_title()) before it
    void draw_seperated(IEditorPanel& panel, const char *title = nullptr);

    // draw the content with wrapped in a collapsing header
    bool draw_collapsing(IEditorPanel& panel, const char *title = nullptr, ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None);
}
