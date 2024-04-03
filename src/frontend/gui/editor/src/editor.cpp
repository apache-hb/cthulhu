// SPDX-License-Identifier: GPL-3.0-or-later
#include "stdafx.hpp"

#include "editor/editor.hpp"

bool IEditorWidget::draw_window()
{
    if (!visible) return false;

    if (ImGui::Begin(get_title(), &visible, flags))
    {
        draw();
    }
    ImGui::End();

    return visible;
}

std::string_view MenuSection::get_title() const
{
    return title;
}

void MenuSection::draw()
{
    if (!title.empty())
    {
        ImGui::SeparatorText(title.c_str());
    }
    else
    {
        ImGui::Separator();
    }

    for (auto &item : items)
    {
        item->draw();
    }
}

std::string_view MenuFlyout::get_title() const
{
    return title;
}

void MenuFlyout::draw()
{
    if (ImGui::BeginMenu(title.c_str()))
    {
        for (auto &item : items)
        {
            item->draw();
        }
        ImGui::EndMenu();
    }
}
