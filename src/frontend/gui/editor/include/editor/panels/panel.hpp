// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "imgui.h"

#include "editor/utils/utils.hpp"

#include <string>

namespace ed
{
    class ScopeID
    {
        CTU_NOCOPY(ScopeID);
        CTU_NOMOVE(ScopeID);
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
        IEditorPanel(std::string_view name);

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

    IEditorPanel *create_imgui_demo_panel();
    IEditorPanel *create_implot_demo_panel();

    // draw the content with a SeperatorText(get_title()) before it
    void draw_seperated(IEditorPanel& panel, const char *title = nullptr);

    // draw the content with wrapped in a collapsing header
    bool draw_collapsing(IEditorPanel& panel, const char *title = nullptr, ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None);
}
