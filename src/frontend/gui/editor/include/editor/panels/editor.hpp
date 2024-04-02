#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include "flecs.h"

struct MainMenu { flecs::entity entity; };
struct Window { };
struct Menu { };
struct MenuItem { };
struct MenuSection { };

struct Separator { };

struct Title {
    std::string title;
    const char *c_str() const { return title.c_str(); }
};

struct ShortCut {
    ImGuiKeyChord chord;
    const char *c_str() const { return ImGui::GetKeyChordName(chord); }
};

struct Children {
    flecs::query<> query;
};

struct Priority {
    int value;

    friend constexpr int cmp(const Priority &lhs, const Priority &rhs) {
        return lhs.value - rhs.value;
    }
};
