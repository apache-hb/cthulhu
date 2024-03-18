#include "stdafx.hpp"

#include "editor/panels/panel.hpp"

using namespace ed;

bool ImGuiDemoPanel::draw_window()
{
    if (visible)
    {
        ImGui::ShowDemoWindow(&visible);
    }

    return visible;
}

ImGuiDemoPanel::ImGuiDemoPanel(panel_info_t setup)
    : IEditorPanel("ImGui Demo", setup)
{ }

bool ImPlotDemoPanel::draw_window()
{
    if (visible)
    {
        ImPlot::ShowDemoWindow(&visible);
    }

    return visible;
}

ImPlotDemoPanel::ImPlotDemoPanel(panel_info_t setup)
    : IEditorPanel("ImPlot Demo", setup)
{ }
