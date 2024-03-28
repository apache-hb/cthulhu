// SPDX-License-Identifier: GPL-3.0-or-later
#include "stdafx.hpp"

#include "editor/panels/theme.hpp"

enum theme_t {
    eThemeDark,
    eThemeLight,
    eThemeClassic,

    eThemeCount
};

static theme_t gTheme = eThemeDark;

template<typename F>
class ThemeMenuItem final : public ed::IEditorPanel
{
    theme_t theme;
    F func;

public:
    ThemeMenuItem(const char *name, theme_t theme, F&& func)
        : IEditorPanel(name)
        , theme(theme)
        , func(std::forward<F>(func))
    { }

    bool is_selected() const
    {
        return gTheme == theme;
    }

    bool menu_item(const char *) override
    {
        bool result = ImGui::MenuItem(get_title(), nullptr, is_selected(), !is_selected());
        if (result)
        {
            gTheme = theme;
            func();
        }
        return result;
    }
};

static ed::IEditorPanel *theme_menu(const char *name, theme_t theme, auto&& func)
{
    return new ThemeMenuItem(name, theme, std::forward<decltype(func)>(func));
}

ed::IEditorPanel *ed::dark_theme()
{
    return theme_menu("Dark", eThemeDark, [] {
        ImGui::StyleColorsDark();
        ImPlot::StyleColorsDark();
    });
}

ed::IEditorPanel *ed::light_theme()
{
    return theme_menu("Light", eThemeLight, [] {
        ImGui::StyleColorsLight();
        ImPlot::StyleColorsLight();
    });
}

ed::IEditorPanel *ed::classic_theme()
{
    return theme_menu("Classic", eThemeClassic, [] {
        ImGui::StyleColorsClassic();
        ImPlot::StyleColorsClassic();
    });
}
