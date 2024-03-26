// SPDX-License-Identifier: GPL-3.0-only
#include "stdafx.hpp"

#include "editor/panels/panel.hpp"

using namespace ed;

IEditorPanel::IEditorPanel(std::string_view name)
    : name(name)
    , visible(false)
{ }

bool IEditorPanel::draw_window()
{
    if (!visible) return false;

    if (ImGui::Begin(get_title(), &visible))
    {
        draw_content();
    }
    ImGui::End();

    return visible;
}

void IEditorPanel::draw()
{
    ScopeID id(this);
    draw_content();
}

bool IEditorPanel::menu_item(const char *shortcut)
{
    bool result = ImGui::MenuItem(get_title(), shortcut, &visible, enabled);
    if (!enabled && !disabled_reason.empty())
    {
        ImGui::SetItemTooltip("%s", disabled_reason.c_str());
    }
    return result;
}

void ed::draw_seperated(IEditorPanel& panel, const char *title)
{
    const char *name = title ? title : panel.get_title();
    ImGui::SeparatorText(name);
    panel.draw();
}

bool ed::draw_collapsing(IEditorPanel& panel, const char *title, ImGuiTreeNodeFlags flags)
{
    const char *name = title ? title : panel.get_title();
    bool result = ImGui::CollapsingHeader(name, flags);

    if (result)
    {
        panel.draw();
    }

    return result;
}
