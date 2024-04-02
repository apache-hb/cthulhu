// SPDX-License-Identifier: GPL-3.0-only
#include "editor/panels/editor.hpp"
#include "editor/panels/theme.hpp"
#include "stdafx.hpp"

// drawing library

#include "draw/draw.hpp"

// editor functionality

#include "editor/compile.hpp"
#include "editor/panels/info.hpp"
#include "editor/panels/arena.hpp"

// editor panels

#include "editor/panels/sources.hpp"

// cthulhu includes

#include "backtrace/backtrace.h"

#include "setup/setup.h"

#include "support/loader.h"
#include "support/support.h"

#include "memory/memory.h"
#include "config/config.h"

#include "std/typed/vector.h"

#include "core/macros.h"

namespace fs = std::filesystem;

static const frontend_t kFrontendGui = {
    .info = {
        .id = "frontend/gui",
        .name = "Cthulhu GUI",
        .version = {
            .license = "GPLv3",
            .desc = "Cthulhu Compiler Collection GUI",
            .author = "Elliot Haisley",
            .version = CT_NEW_VERSION(0, 0, 1),
        },
    },
};

static ed::TraceArena gGlobalArena{"Global Arena", ed::TraceArena::eDrawTree, true};
static ed::TraceArena gGmpArena{"GMP Arena", ed::TraceArena::eDrawFlat, true};
static ed::TraceArena gGuiArena{"Dear ImGui Arena", ed::TraceArena::eDrawTree, false};

static void *imgui_malloc(size_t size, void *user)
{
    arena_t *arena = reinterpret_cast<arena_t*>(user);
    return ARENA_OPT_MALLOC(size, "ImGui::MemAlloc", NULL, arena);
}

static void imgui_free(void *ptr, void *user)
{
    arena_t *arena = reinterpret_cast<arena_t*>(user);
    arena_opt_free(ptr, CT_ALLOC_SIZE_UNKNOWN, arena);
}

static void install_trace_arenas()
{
    init_global_arena(gGlobalArena.get_arena());
    init_gmp_arena(gGmpArena.get_arena());
    ImGui::SetAllocatorFunctions(imgui_malloc, imgui_free, gGuiArena.get_arena());
}

class EditorModulePanel : public ed::IEditorPanel
{
    std::vector<ed::LanguageInfoPanel> languages;
    std::vector<ed::PluginInfoPanel> plugins;
    std::vector<ed::TargetInfoPanel> targets;

    void draw_content() override
    {
        if (ImGui::BeginTabBar("ModuleTabs"))
        {
            if (ImGui::BeginTabItem("Languages"))
            {
                for (auto &lang : languages)
                {
                    ed::draw_collapsing(lang);
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Plugins"))
            {
                for (auto &plugin : plugins)
                {
                    ed::draw_collapsing(plugin);
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Targets"))
            {
                for (auto &target : targets)
                {
                    ed::draw_collapsing(target);
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }

public:
    EditorModulePanel()
        : IEditorPanel("Modules")
    {
        set_enabled(false);
    }

    void add_module(const loaded_module_t &mod)
    {
        if (mod.type & eModLanguage)
        {
            languages.emplace_back(*mod.lang);
        }

        if (mod.type & eModPlugin)
        {
            plugins.emplace_back(*mod.plugin);
        }

        if (mod.type & eModTarget)
        {
            targets.emplace_back(*mod.target);
        }
    }

    void load_default_modules(support_t *support)
    {
        support_load_default_modules(support);

        typevec_t *mods = support_get_modules(support);
        size_t len = typevec_len(mods);

        for (size_t i = 0; i < len; i++)
        {
            loaded_module_t mod = {};
            typevec_get(mods, i, &mod);
            add_module(mod);
        }

        set_enabled(true);
    }

    bool menu_item(const char *shortcut = nullptr) override
    {
        bool result = IEditorPanel::menu_item(shortcut);
        if (!enabled)
            ImGui::SetItemTooltip("Load a module to enable this panel");

        return result;
    }

    bool is_empty() const
    {
        return languages.empty() && plugins.empty() && targets.empty();
    }
};

class StaticModulePanel final : public ed::IEditorPanel
{
    EditorModulePanel& loader;
    support_t *support;

public:
    StaticModulePanel(EditorModulePanel& loader, support_t *support)
        : IEditorPanel("Loader")
        , loader(loader)
        , support(support)
    { }

    void draw_content() override
    {
        if (loader.is_empty())
        {
            if (ImGui::Button("Load default modules"))
            {
                loader.load_default_modules(support);
                visible = false;
                enabled = false;
            }
        }
        else
        {
            ImGui::Text("Default modules loaded");
        }
    }

    bool menu_item(const char *shortcut = nullptr) override
    {
        if (!loader.is_empty())
            return false;

        return IEditorPanel::menu_item(shortcut);
    }
};

class DynamicModulePanel final : public ed::IEditorPanel
{
    EditorModulePanel& loader;
    support_t *support;

    std::string path;
    int mask = eModLanguage;

    ImGui::FileBrowser file_browser { ImGuiFileBrowserFlags_MultipleSelection | ImGuiFileBrowserFlags_ConfirmOnEnter };

    std::string error;
    std::string os_error;

public:
    DynamicModulePanel(EditorModulePanel& loader, support_t *support)
        : IEditorPanel("Loader")
        , loader(loader)
        , support(support)
    {
        file_browser.SetTitle("Open Shared Module");
        file_browser.SetTypeFilters({ ".dll", ".so", ".dylib" });
    }

    void draw_content() override
    {
        ImGui::CheckboxFlags("Language", &mask, eModLanguage);
        ImGui::SameLine(); ImGui::CheckboxFlags("Plugin", &mask, eModPlugin);
        ImGui::SameLine(); ImGui::CheckboxFlags("Target", &mask, eModTarget);
        if (ImGui::Button("Load Module"))
        {
            file_browser.Open();
        }

        file_browser.Display();

        if (file_browser.HasSelected())
        {
            for (const auto &file : file_browser.GetMultiSelected())
            {
                loaded_module_t mod = {};
                if (support_load_module(support, module_type_t(mask), file.string().c_str(), &mod))
                {
                    bt_update();
                    loader.add_module(mod);
                }
                else
                {
                    error = load_error_string(mod.error);
                    os_error = os_error_string(mod.os, get_global_arena());
                }
            }
            file_browser.ClearSelected();
        }

        if (error.length() > 0)
        {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Load Error: %s", error.c_str());
        }

        if (os_error.length() > 0)
        {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "OS Error: %s", os_error.c_str());
        }
    }
};

void draw_menu_items(ed::menu_section_t& section)
{
    for (ed::IEditorPanel *panel : section.panels)
    {
        panel->menu_item();
    }
}

static void seperator_opt(const std::string& str)
{
    if (str.empty())
    {
        ImGui::Separator();
    }
    else
    {
        ImGui::SeparatorText(str.c_str());
    }
}

class EditorUi
{
    static constexpr size_t kMaxPanels = 1024;

    loader_t *loader;
    broker_t *broker;
    support_t *support;

    ed::FrontendInfoPanel version_info_panel { kFrontendGui };

    EditorModulePanel module_panel;
    DynamicModulePanel dynamic_module_panel { module_panel, support };
    StaticModulePanel static_module_panel { module_panel, support };

    auto& get_loader_panel()
    {
#if CT_BUILD_STATIC
        return static_module_panel;
#else
        return dynamic_module_panel;
#endif
    }

    std::vector<ed::menu_t> menus;

    void add_menu(const ed::menu_t &menu)
    {
        menus.push_back(menu);
    }

    void init()
    {
        ed::menu_t windows_menu = {
            .name = "Windows",
            .header = { &version_info_panel, &module_panel },
            .sections = {
                ed::menu_section_t {
                    .name = "Memory",
                    .panels = { &gGlobalArena, &gGmpArena, &gGuiArena }
                },
                ed::menu_section_t {
                    .name = "Demo",
                    .panels = { ed::create_imgui_demo_panel(), ed::create_implot_demo_panel() }
                }
            }
        };

        ed::menu_t styles_menu = {
            .name = "Styles",
            .header = {
                ed::dark_theme(),
                ed::light_theme(),
                ed::classic_theme(),
            }
        };

        add_menu(windows_menu);
        add_menu(styles_menu);
    }

    void draw_menubar()
    {
        for (auto &menu : menus)
        {
            if (ImGui::BeginMenu(menu.name.c_str()))
            {
                for (auto &item : menu.header)
                {
                    item->menu_item();
                }

                for (auto &section : menu.sections)
                {
                    if (section.panels.empty())
                        continue;

                    if (section.seperator)
                    {
                        seperator_opt(section.name);
                        draw_menu_items(section);
                        continue;
                    }

                    if (ImGui::BeginMenu(section.name.c_str()))
                    {
                        draw_menu_items(section);
                        ImGui::EndMenu();
                    }
                }

                ImGui::EndMenu();
            }
        }
    }

    std::vector<std::unique_ptr<ed::SourceView>> sources;

    void draw_source_files()
    {
        for (auto &source : sources)
        {
            source->draw_content();
        }
    }

public:
    EditorUi()
        : loader(loader_new(get_global_arena()))
        , broker(broker_new(&kFrontendGui, get_global_arena()))
        , support(support_new(broker, loader, get_global_arena()))
    {
        file_browser.SetTitle("Open Source Files");

        init();
    }

    void draw_windows()
    {
        draw_loader_window();

        draw_compiler_tabs();

        for (auto& menu : menus)
        {
            for (auto& panel : menu.header)
            {
                panel->draw_window();
                panel->update();
            }

            for (auto& section : menu.sections)
            {
                for (auto& panel : section.panels)
                {
                    panel->draw_window();
                    panel->update();
                }
            }
        }

        auto& loader_panel = get_loader_panel();
        if (!module_panel.is_empty())
            loader_panel.draw_window();
    }

    static const ImGuiDockNodeFlags kDockFlags
        = ImGuiDockNodeFlags_PassthruCentralNode;

    static const ImGuiWindowFlags kDockWindowFlags
        = ImGuiWindowFlags_MenuBar
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoBackground
        | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoNavFocus
        | ImGuiWindowFlags_NoDocking;

    void dockspace()
    {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    }

    void mainmenu()
    {
        if (ImGui::BeginMainMenuBar())
        {
            ImGui::Text("Cthulhu");
            ImGui::Separator();

            draw_menubar();

            ImGui::EndMainMenuBar();
        }
    }

private:
    void draw_module_loader()
    {
        auto& loader_panel = get_loader_panel();
        loader_panel.draw();
    }

    static constexpr ImGuiWindowFlags kMainFlags
        = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoMove;

    ImGui::FileBrowser file_browser { ImGuiFileBrowserFlags_MultipleSelection | ImGuiFileBrowserFlags_ConfirmOnEnter };

    void draw_loader_window()
    {
        auto& panel = get_loader_panel();
        if (!panel.is_enabled())
            return;

        const ImGuiViewport *viewport = ImGui::GetMainViewport();

        // center the window
        ImGui::SetNextWindowPos(
            ImVec2(viewport->Pos + viewport->Size / 2.f),
            ImGuiCond_Appearing,
            ImVec2(0.5f, 0.5f));

        if (ImGui::Begin("Loader", nullptr, kMainFlags))
        {
            panel.draw();
        }
        ImGui::End();
    }

    void draw_compiler_tabs()
    {
        if (ImGui::Begin("Compiler"))
        {
            if (ImGui::BeginTabBar("CompilerTabs"))
            {
                if (ImGui::TabItemButton("+", ImGuiTabItemFlags_NoTooltip | ImGuiTabItemFlags_Trailing))
                    file_browser.Open();

                size_t index = SIZE_MAX;

                for (size_t i = 0; i < sources.size(); i++)
                {
                    bool open = true;
                    auto &source = sources[i];
                    if (ImGui::BeginTabItem(source->get_basename(), &open))
                    {
                        source->draw_content();
                        ImGui::EndTabItem();
                    }

                    if (!open)
                    {
                        index = i;
                    }
                }

                if (index != SIZE_MAX)
                {
                    // TODO: remove the source file
                    sources.erase(sources.begin() + (ptrdiff_t)index);
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();

        file_browser.Display();

        if (file_browser.HasSelected())
        {
            for (const fs::path &file : file_browser.GetMultiSelected())
            {
                sources.emplace_back(std::make_unique<ed::SourceView>(file));
            }

            file_browser.ClearSelected();
        }
    }
};

enum {
    kMenuFile,
    kMenuEdit,
    kMenuView,
    kMenuDebug,
    kMenuHelp,
};

static void init_menu(flecs::world& ecs)
{
    auto mainmenu = ecs.entity("Main Menu")
        .add<MainMenu>();

    ecs.set<MainMenu>({ mainmenu });

    auto menu = ecs.prefab("Menu")
        .add<Menu>()
        .set_override(Title { })
        .set_override(Priority{ 0 });

    auto item = ecs.prefab("Item")
        .add<MenuItem>()
        .set_override(Title { })
        .set_override(Priority{ 0 });

    auto section = ecs.prefab("Section")
        .add<MenuSection>()
        .set_override(Title { })
        .set_override(Priority{ 0 });

    {
        auto file = ecs.entity("File")
            .set(Title{ "File" })
            .set(Priority{ kMenuFile })
            .is_a(menu)
            .child_of(mainmenu);

        enum {
            eNew,
            eOpen,
            eSave,
            eSaveAs,
            eSeparator,
            eOpenRecent,
        };

        ecs.entity("New")
            .set(Title{ "New" })
            .set(ShortCut{ ImGuiMod_Ctrl | ImGuiKey_N })
            .set(Priority{ eNew })
            .is_a(item)
            .child_of(file);

        {
            auto open = ecs.entity("Open")
                .set(Priority{ eOpen })
                .child_of(file)
                .is_a(section);

            ecs.entity("Open")
                .set(Title{ "Open" })
                .set(ShortCut{ ImGuiMod_Ctrl | ImGuiKey_O })
                .set(Priority{ eOpen })
                .is_a(item)
                .child_of(open);

            ecs.entity("Open Recent")
                .set(Title{ "Open Recent" })
                .set(Priority{ eOpenRecent })
                .is_a(item)
                .child_of(open);
        }

        {
            auto ss = ecs.entity("Save")
                .add<Separator>()
                .set(Priority{ eSeparator })
                .is_a(section)
                .child_of(file);

            ecs.entity("Save")
                .set(Title{ "Save" })
                .set(ShortCut{ ImGuiMod_Ctrl | ImGuiKey_S })
                .set(Priority{ eSave })
                .is_a(item)
                .child_of(ss);

            ecs.entity("Save As")
                .set(Title{ "Save As" })
                .set(ShortCut{ ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S })
                .set(Priority{ eSaveAs })
                .is_a(item)
                .child_of(ss);
        }
    }

    {
        enum {
            eUndo,
            eRedo,
            eSeparator,
            eCut,
            eCopy,
            ePaste,
        };

        auto edit = ecs.entity("Edit")
            .set(Title{ "Edit" })
            .set(Priority{ kMenuEdit })
            .is_a(menu)
            .child_of(mainmenu);

        ecs.entity("Undo")
            .set(Title{ "Undo" })
            .set(ShortCut{ ImGuiMod_Ctrl | ImGuiKey_Z })
            .set(Priority{ eUndo })
            .is_a(item)
            .child_of(edit);

        ecs.entity("Redo")
            .set(Title{ "Redo" })
            .set(ShortCut{ ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z })
            .set(Priority{ eRedo })
            .is_a(item)
            .child_of(edit);

        ecs.entity("Separator")
            .add<Separator>()
            .set(Priority{ eSeparator })
            .child_of(edit);

        ecs.entity("Cut")
            .set(Title{ "Cut" })
            .set(ShortCut{ ImGuiMod_Ctrl | ImGuiKey_X })
            .set(Priority{ eCut })
            .is_a(item)
            .child_of(edit);

        ecs.entity("Copy")
            .set(Title{ "Copy" })
            .set(ShortCut{ ImGuiMod_Ctrl | ImGuiKey_C })
            .set(Priority{ eCopy })
            .is_a(item)
            .child_of(edit);

        ecs.entity("Paste")
            .set(Title{ "Paste" })
            .set(ShortCut{ ImGuiMod_Ctrl | ImGuiKey_V })
            .set(Priority{ ePaste })
            .is_a(item)
            .child_of(edit);
    }

    {
        auto view = ecs.entity("View")
            .set(Title{ "View" })
            .set(Priority{ kMenuView })
            .is_a(menu)
            .child_of(mainmenu);

        enum {
            eThemes,
            eDark,
            eLight,
            eClassic,
        };

        {
            auto themes = ecs.entity("Themes")
                .set(Title{ "Themes" })
                .add<Separator>()
                .set(Priority{ eThemes })
                .is_a(section)
                .child_of(view);

            ecs.entity("Dark")
                .set(Title{ "Dark" })
                .set(Priority{ eDark })
                .is_a(item)
                .child_of(themes);

            ecs.entity("Light")
                .set(Title{ "Light" })
                .set(Priority{ eLight })
                .is_a(item)
                .child_of(themes);

            ecs.entity("Classic")
                .set(Title{ "Classic" })
                .set(Priority{ eClassic })
                .is_a(item)
                .child_of(themes);
        }
    }

    {
        auto debug = ecs.entity("Debug")
            .set(Title{ "Debug" })
            .set(Priority{ kMenuDebug })
            .is_a(menu)
            .child_of(mainmenu);

        enum {
            eMemory,
            eMemoryArena,
            eGlobalArena,
            eGmpArena,
            eDearImGuiArena,
        };

        {
            auto memory = ecs.entity("Memory")
                .set(Title{ "Memory" })
                .set(Priority{ eMemory })
                .is_a(section)
                .child_of(debug);

            ecs.entity("Global Arena")
                .set(Title{ "Global Arena" })
                .set(Priority{ eGlobalArena })
                .is_a(item)
                .child_of(memory);

            ecs.entity("GMP Arena")
                .set(Title{ "GMP Arena" })
                .set(Priority{ eGmpArena })
                .is_a(item)
                .child_of(memory);

            ecs.entity("Dear ImGui Arena")
                .set(Title{ "Dear ImGui Arena" })
                .set(Priority{ eDearImGuiArena })
                .is_a(item)
                .child_of(memory);
        }
    }

    {
        auto help = ecs.entity("Help")
            .is_a(menu)
            .set(Title{ "Help" })
            .set(Priority{ kMenuHelp });

        enum {
            eAbout,
            eSearchMenus,
            eDemo,
            eDearImGuiDemo,
            eImPlotDemo,
        };

        ecs.entity("About")
            .set(Title{ "About" })
            .set(Priority{ eAbout })
            .is_a(item)
            .child_of(help);

        ecs.entity("Search Menus")
            .set(Title{ "Search Menus" })
            .set(Priority{ eSearchMenus })
            .is_a(item)
            .child_of(help);

        {
            auto demo = ecs.entity("Demo")
                .add<Separator>()
                .set(Title{ "Demo" })
                .set(Priority{ eDemo })
                .is_a(section)
                .child_of(help);

            ecs.entity("Dear ImGui Demo")
                .set(Title{ "Dear ImGui Demo" })
                .set(Priority{ eDearImGuiDemo })
                .is_a(item)
                .child_of(demo);

            ecs.entity("ImPlot Demo")
                .set(Title{ "ImPlot Demo" })
                .set(Priority{ eImPlotDemo })
                .is_a(item)
                .child_of(demo);
        }
    }
}

static flecs::query<> get_children(flecs::entity e)
{
    if (const Children *it = e.get<Children>())
    {
        return it->query;
    }

    flecs::query<> query = e.world().query_builder<const Priority>()
        .order_by<Priority>([](flecs::entity_t, const Priority *lp, flecs::entity_t, const Priority *rp) -> int {
            return cmp(*lp, *rp);
        })
        .term(flecs::ChildOf, e)
        .build();

    e.set<Children>({ query });

    return query;
}

static void draw_menuitem(const flecs::entity e)
{
    const Title* title = e.get<Title>();
    if (e.has<Separator>())
    {
        if (title != nullptr)
        {
            ImGui::SeparatorText(title->c_str());
        }
        else
        {
            ImGui::Separator();
        }
    }

    if (e.has<MenuSection>())
    {
        get_children(e).each(draw_menuitem);
    }
    else if (e.has<MenuItem>())
    {
        const ShortCut* shortcut = e.get<ShortCut>();
        const char *str = (shortcut != nullptr) ? shortcut->c_str() : nullptr;

        ImGui::MenuItem(title->c_str(), str, false);
    }
}

static void draw_menu(const flecs::entity e)
{
    const Title* title = e.get<Title>();
    if (ImGui::BeginMenu(title->c_str()))
    {
        get_children(e).each(draw_menuitem);
        ImGui::EndMenu();
    }
}

static void draw_mainmenu(const flecs::world& ecs)
{
    const MainMenu *mm = ecs.get<MainMenu>();

    ImGui::Text("Cthulhu");
    ImGui::Separator();

    get_children(mm->entity).each(draw_menu);
}

int main(int argc, const char **argv)
{
    CT_UNUSED(argc);
    CT_UNUSED(argv);

    setup_global();
    install_trace_arenas();

    draw::config_t config = {
        .title = L"Editor",
        .hardware_acceleration = false,
    };

    if (!draw::create(config))
    {
        return 1;
    }

    flecs::world ecs;
    ecs.import<flecs::monitor>();

    init_menu(ecs);

    EditorUi ui;

    ecs.system("NewFrame")
        .kind(flecs::PreFrame)
        .iter([&](flecs::iter& it) {
            bool begin = draw::begin_frame();
            if (!begin)
                it.world().quit();
            else
                ui.dockspace();
        });

    ecs.system("MainMenu")
        .no_readonly()
        .kind(flecs::OnUpdate)
        .iter([](flecs::iter& it) {
            if (it.world().should_quit())
                return;

            if (ImGui::BeginMainMenuBar())
            {
                draw_mainmenu(it.world());
                ImGui::EndMainMenuBar();
            }
        });

    ecs.system("EndFrame")
        .kind(flecs::PostFrame)
        .iter([](flecs::iter& it) {
            if (it.world().should_quit())
                return;

            draw::end_frame();
        });

    int ret = ecs.app().enable_rest().run();

    draw::destroy();

    return ret;
}
