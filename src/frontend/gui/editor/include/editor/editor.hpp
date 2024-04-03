#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include <functional>

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

    std::string_view get_title() const override;
    void draw() override;

    MenuSection(std::string_view it = "")
        : title(it)
    { }
};

struct MenuFlyout final : IMenuItem
{
    std::string title;
    std::vector<std::shared_ptr<IMenuItem>> items;

    std::string_view get_title() const override;
    void draw() override;
};

struct MenuAction final : IMenuItem
{
    std::string title;
    std::function<void()> action;

    std::string_view get_title() const override;
    void draw() override;

    MenuAction(std::string_view it, std::function<void()> act)
        : title(it)
        , action(act)
    { }
};

struct Menu
{
    std::string name;
    std::vector<std::shared_ptr<IMenuItem>> items;

    Menu(std::string_view it)
        : name(it)
    { }

    Menu& action(std::string_view it, auto&& fn)
    {
        items.push_back(std::make_shared<MenuAction>(it, std::forward<decltype(fn)>(fn)));
        return *this;
    }

    Menu& hotkey(ImGuiKeyChord chord);
    Menu& separator();
};

struct Editor
{
    std::string name = "Cthulhu";
    std::vector<std::shared_ptr<IEditorWidget>> widgets;
    std::unordered_map<ImGuiKeyChord, std::shared_ptr<IMenuItem>> shortcuts;
    std::vector<Menu> menus;

    template<std::derived_from<IEditorWidget> T>
    std::shared_ptr<T> widget(auto&&... args)
    {
        auto widget = std::make_shared<T>(std::forward<decltype(args)>(args)...);
        widgets.push_back(widget);
        return widget;
    }

    Menu& menu(std::string_view it);
};

extern Editor g;
