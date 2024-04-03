#pragma once

#include "imgui.h"
#include "imgui_internal.h"

struct IMenuItem;
struct MenuSection;
struct MenuFlyout;

class IEditorWidget
{
    std::string name = "EditorWidget";
    ImGuiWindowFlags flags = ImGuiWindowFlags_None;
    bool visible = false;
    bool enabled = true;

protected:
    IEditorWidget(std::string_view name)
        : name(name)
    { }

    virtual void draw() = 0;

public:
    virtual ~IEditorWidget() = default;

    void set_title(std::string_view it) { name = it; }
    void set_flags(ImGuiWindowFlags it) { flags = it; }
    void set_visible(bool it) { visible = it; }
    void set_enabled(bool it) { enabled = it; }

    const char *get_title() const { return name.c_str(); }
    ImGuiWindowFlags get_flags() const { return flags; }
    bool is_visible() const { return visible; }
    bool is_enabled() const { return enabled; }

    bool draw_window();
    virtual void update() { }
};

struct IMenuItem
{
    virtual ~IMenuItem() = default;

    virtual std::string_view get_title() const = 0;
    virtual void draw() = 0;
};

struct MenuSection final : IMenuItem
{
    std::string title;
    std::vector<std::shared_ptr<IMenuItem>> items;

    std::string_view get_title() const override;
    void draw() override;
};

struct MenuFlyout final : IMenuItem
{
    std::string title;
    std::vector<std::shared_ptr<IMenuItem>> items;

    std::string_view get_title() const override;
    void draw() override;
};

struct IMenuEntry : IMenuItem
{
    ImGuiKeyChord shortcut = ImGuiKey_None;

    virtual void activate() = 0;
};

struct Menu
{
    std::string name;
    std::vector<std::shared_ptr<IMenuItem>> items;
};

struct Editor
{
    std::string name;
    std::vector<std::shared_ptr<IEditorWidget>> widgets;
    std::vector<std::shared_ptr<IMenuEntry>> entries;
    std::vector<Menu> menus;

    template<std::derived_from<IEditorWidget> T>
    std::shared_ptr<T> add_widget(auto&&... args)
    {
        auto widget = std::make_shared<T>(std::forward<decltype(args)>(args)...);
        widgets.push_back(widget);
        return widget;
    }

    template<std::derived_from<IMenuItem> T>
    std::shared_ptr<T> add_menu_entry();
};

extern Editor g;
