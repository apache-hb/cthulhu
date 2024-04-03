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

Menu& Menu::hotkey(ImGuiKeyChord chord)
{
    g.shortcuts[chord] = items.back();
    return *this;
}

Menu& Editor::menu(std::string_view name)
{
    auto it = std::find_if(menus.begin(), menus.end(), [&](const auto &menu) {
        return menu.name == name;
    });

    if (it != menus.end())
    {
        return *it;
    }

    return menus.emplace_back(Menu(name));
}

Editor g{};
