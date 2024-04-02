// SPDX-License-Identifier: GPL-3.0-or-later
#include "stdafx.hpp"

#include "editor/panels/editor.hpp"

using namespace ed;

IndexOf<Menu> Context::add_menu(const std::string& name)
{
    menus.push_back({ name, {} });
    return menus.size() - 1;
}

IndexOf<MenuItem> Context::add_item(const std::string& name)
{
    items.push_back(MenuItem{ name });
    return IndexOf<MenuItem>(items.size() - 1);
}

IndexOf<MenuSection> Context::add_section(const std::string& name)
{
    sections.push_back({ name, {} });
    return IndexOf<MenuSection>(sections.size() - 1);
}

IndexOf<MenuFlyout> Context::add_flyout(const std::string& name)
{
    flyouts.push_back({ name, {} });
    return IndexOf<MenuFlyout>(flyouts.size() - 1);
}

ed::Context g{};
