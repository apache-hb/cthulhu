#pragma once

#include "editor/panels/arena.hpp"

#include <variant>
#include <vector>

namespace ed
{
    struct Context;
}

extern ed::Context g;

namespace ed
{
    struct Context;
    struct Menu;
    struct MenuItem;
    struct MenuSection;
    struct MenuFlyout;

    template<typename T>
    class IndexOf {
        uint16_t index;
    public:
        IndexOf(uint16_t index)
            : index(index)
        { }

        operator uint16_t() const { return index; }
        uint16_t get() const { return index; }
    };

    template<typename... T>
    using Index = std::variant<IndexOf<T>...>;

    template<typename... T>
    using MaybeIndex = std::variant<Index<T...>, std::monostate>;

    struct MenuSeparator { };

    using MenuBodyIndex = std::variant<
        MenuSeparator,
        IndexOf<MenuItem>,
        IndexOf<MenuSection>,
        IndexOf<MenuFlyout>>;

    struct Menu
    {
        std::string name;
        std::vector<MenuBodyIndex> items;

        void add(MenuBodyIndex index) { items.push_back(index); }
    };

    struct MenuItem
    {
        std::string name;
    };

    struct MenuSection
    {
        std::string name;
        std::vector<MenuBodyIndex> items;

        void add(MenuBodyIndex index) { items.push_back(index); }
    };

    struct MenuFlyout
    {
        std::string name;
        std::vector<MenuBodyIndex> items;

        void add(MenuBodyIndex index) { items.push_back(index); }
    };

    struct Context
    {
        std::vector<ed::TraceArena> arenas;

        std::vector<Menu> menus;
        std::vector<MenuItem> items;
        std::vector<MenuSection> sections;
        std::vector<MenuFlyout> flyouts;

        IndexOf<Menu> add_menu(const std::string& name);
        IndexOf<MenuItem> add_item(const std::string& name);
        IndexOf<MenuSection> add_section(const std::string& name);
        IndexOf<MenuFlyout> add_flyout(const std::string& name);

        template<typename T>
        std::vector<T>& get_array();

        template<> std::vector<Menu>& get_array() { return menus; }
        template<> std::vector<MenuItem>& get_array() { return items; }
        template<> std::vector<MenuSection>& get_array() { return sections; }
        template<> std::vector<MenuFlyout>& get_array() { return flyouts; }
    };

    template<typename T>
    class Holder : public IndexOf<T> {
    public:
        Holder(IndexOf<T> item)
            : IndexOf<T>(item.get())
        { }

        Holder(uint16_t index)
            : IndexOf<T>(index)
        { }

        operator T&() const {
            return g.get_array<T>().at(this->get());
        }

        T *operator->() const {
            return &g.get_array<T>().at(this->get());
        }
    };
}
