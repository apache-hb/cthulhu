// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "imgui.h"

#include "editor/utils/utils.hpp"

#include <string>

namespace ed
{
    class ScopeID
    {
        CTX_NOCOPY(ScopeID);
        CTX_NOMOVE(ScopeID);
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
        std::string disabled_reason;
        ImGuiWindowFlags flags = ImGuiWindowFlags_None;

        void set_enabled(bool value) { enabled = value; }
        void disable(std::string_view reason) { enabled = false; disabled_reason = reason; }

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

    struct menu_section_t
    {
        std::string name;
        bool seperator = true; // if true use a seperator, otherwise use a nested menu
        std::vector<ed::IEditorPanel*> panels;
    };

    struct menu_t
    {
        std::string name;
        std::vector<ed::IEditorPanel*> header;
        std::vector<menu_section_t> sections;
    };

    IEditorPanel *create_imgui_demo_panel();
    IEditorPanel *create_implot_demo_panel();

    // draw the content with a SeperatorText(get_title()) before it
    void draw_seperated(IEditorPanel& panel, const char *title = nullptr);

    // draw the content with wrapped in a collapsing header
    bool draw_collapsing(IEditorPanel& panel, const char *title = nullptr, ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None);
}
