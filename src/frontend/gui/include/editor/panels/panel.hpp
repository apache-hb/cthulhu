#pragma once

#include "imgui.h"

#include <string>

namespace ed
{
    struct panel_info_t
    {
        bool open_on_start = false;
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

        // draw the panel inside its own window
        virtual bool draw_window();

        // draw only the content of the panel
        void draw();

        // draw a menu item that toggles the visibility of the panel
        virtual bool menu_item(const char *shortcut = nullptr);

        const char *get_title() const { return name.c_str(); }
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
