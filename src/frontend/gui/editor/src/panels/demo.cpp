// SPDX-License-Identifier: GPL-3.0-only
#include "stdafx.hpp"

#include "editor/panels/panel.hpp"

using namespace ed;

class ImGuiDemoPanel final : public IEditorPanel
{
public:
    // IEditorPanel
    bool draw_window() override
    {
        if (visible)
        {
            ImGui::ShowDemoWindow(&visible);
        }

        return visible;
    }

    ImGuiDemoPanel()
        : IEditorPanel("ImGui Demo")
    { }
};

class ImPlotDemoPanel final : public IEditorPanel
{
    // IEditorPanel
    bool draw_window() override
    {
        if (visible)
        {
            ImPlot::ShowDemoWindow(&visible);
        }

        return visible;
    }

public:
    ImPlotDemoPanel()
        : IEditorPanel("ImPlot Demo")
    { }
};

IEditorPanel *ed::create_imgui_demo_panel()
{
    return new ImGuiDemoPanel();
}

IEditorPanel *ed::create_implot_demo_panel()
{
    return new ImPlotDemoPanel();
}
