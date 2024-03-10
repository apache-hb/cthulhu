// SPDX-License-Identifier: GPL-3.0-only

// drawing library

#include "backtrace/backtrace.h"
#include "draw/draw.hpp"

// editor functionality

#include "editor/compile.hpp"
#include "editor/panels/info.hpp"
#include "editor/trace.hpp"

// editor panels

#include "editor/panels/sources.hpp"
#include "editor/panels/arena.hpp"

// dear imgui

#include "imgui.h"
#include "imfilebrowser.h"
#include "implot.h"

// cthulhu includes

#include "setup/setup.h"

#include "support/loader.h"
#include "support/support.h"

#include "memory/memory.h"
#include "interop/compile.h"
#include "config/config.h"

#include "io/io.h"

#include "std/typed/vector.h"

#include "core/macros.h"

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

static ed::TraceArena gGlobalArena{"Global Arena", ed::TraceArena::eDrawTree};
static ed::TraceArena gGmpArena{"GMP Arena", ed::TraceArena::eDrawFlat};
static ed::TraceArena gGuiArena{"Dear ImGui Arena", ed::TraceArena::eDrawTree};

namespace ed
{
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

    void install_trace_arenas()
    {
        init_global_arena(gGlobalArena.get_arena());
        init_gmp_arena(gGmpArena.get_arena());
        ImGui::SetAllocatorFunctions(imgui_malloc, imgui_free, gGuiArena.get_arena());
    }
}

class SourceCode
{
    io_t *io = nullptr;
    const char *path = nullptr;
    const char *text = nullptr;
    size_t size = 0;

    io_error_t error = 0;
    char *str = nullptr;
    bool open = true;

    void init()
    {
        path = io_name(io);

        size = io_size(io);
        text = (char*)io_map(io, eOsProtectRead);

        error = io_error(io);
        if (error)
        {
            str = os_error_string(error, get_global_arena());
        }
    }
public:
    SourceCode(io_t *io)
        : io(io)
    {
        init();
    }

    void draw_body()
    {
        if (ImGui::BeginTabItem(path, &open, ImGuiTabItemFlags_None))
        {
            if (error)
            {
                ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Error: %s", str);
            }
            else
            {
                ImGui::TextWrapped("%s", text);
            }

            ImGui::EndTabItem();
        }
    }
};

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
        }
        ImGui::EndTabBar();
    }

public:
    EditorModulePanel(ed::panel_info_t setup = {})
        : IEditorPanel("Modules", setup)
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
    StaticModulePanel(EditorModulePanel& loader, support_t *support, ed::panel_info_t setup = {})
        : IEditorPanel("Modules", setup)
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
    DynamicModulePanel(EditorModulePanel& loader, support_t *support, ed::panel_info_t setup = {})
        : IEditorPanel("Modules", setup)
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

class EditorUi
{
    ed::TraceArenaPanel global_arena_panel { gGlobalArena };
    ed::TraceArenaPanel gmp_arena_panel { gGmpArena };
    ed::TraceArenaPanel imgui_arena_panel { gGuiArena };

    ed::FrontendInfoPanel version_info_panel { kFrontendGui };

    ed::ImGuiDemoPanel imgui_demo_panel;
    ed::ImPlotDemoPanel implot_demo_panel;

    loader_t *loader;
    broker_t *broker;
    support_t *support;

    EditorModulePanel module_panel;
    DynamicModulePanel dynamic_module_panel { module_panel, support };
    StaticModulePanel static_module_panel { module_panel, support };

    ed::IEditorPanel& get_loader_panel()
    {
#if CTU_LOADER_STATIC
        return static_module_panel;
#else
        return dynamic_module_panel;
#endif
    }

    std::vector<SourceCode> sources;

    void draw_source_files()
    {
        for (auto &source : sources)
        {
            source.draw_body();
        }
    }

public:
    EditorUi()
        : loader(loader_new(get_global_arena()))
        , broker(broker_new(&kFrontendGui, get_global_arena()))
        , support(support_new(broker, loader, get_global_arena()))
    {
        file_browser.SetTitle("Open Source Files");
    }

    void draw_windows()
    {
        draw_setup_window();

        version_info_panel.draw_window();
        global_arena_panel.draw_window();
        gmp_arena_panel.draw_window();
        imgui_arena_panel.draw_window();
        imgui_demo_panel.draw_window();
        implot_demo_panel.draw_window();

        module_panel.draw_window();

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

    void dock_space()
    {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));

        ImGui::Begin("Editor", nullptr, kDockWindowFlags);

        ImGui::PopStyleVar(3);

        ImGuiID id = ImGui::GetID("EditorDock");
        ImGui::DockSpace(id, ImVec2(0.f, 0.f), kDockFlags);

        if (ImGui::BeginMainMenuBar())
        {
            ImGui::Text("Cthulhu");
            ImGui::Separator();

            if (ImGui::BeginMenu("Style"))
            {
                if (ImGui::MenuItem("Classic"))
                {
                    ImGui::StyleColorsClassic();
                    ImPlot::StyleColorsClassic();
                }

                if (ImGui::MenuItem("Dark"))
                {
                    ImGui::StyleColorsDark();
                    ImPlot::StyleColorsDark();
                }

                if (ImGui::MenuItem("Light"))
                {
                    ImGui::StyleColorsLight();
                    ImPlot::StyleColorsLight();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Windows"))
            {
                ImGui::SeparatorText("Info");
                version_info_panel.menu_item();
                auto& loader_panel = get_loader_panel();
                loader_panel.menu_item();

                ImGui::SeparatorText("Modules");
                module_panel.menu_item();
                global_arena_panel.menu_item();
                gmp_arena_panel.menu_item();
                imgui_arena_panel.menu_item();

                ImGui::SeparatorText("Demo Windows");
                imgui_demo_panel.menu_item();
                implot_demo_panel.menu_item();

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        ImGui::End();
    }

private:
    void draw_module_loader()
    {
        auto& loader_panel = get_loader_panel();
        loader_panel.draw();
    }

    static constexpr ImGuiWindowFlags kMainFlags = ImGuiWindowFlags_NoDecoration
                                                 | ImGuiWindowFlags_NoMove;

    ImGui::FileBrowser file_browser { ImGuiFileBrowserFlags_MultipleSelection | ImGuiFileBrowserFlags_ConfirmOnEnter };

    bool show_loader = true;

    void draw_setup_window()
    {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        if (ImGui::Begin("Compiler", nullptr, kMainFlags))
        {
            if (ImGui::BeginTabBar("CompilerTabs"))
            {
                if (ImGui::TabItemButton("+", ImGuiTabItemFlags_NoTooltip | ImGuiTabItemFlags_Trailing))
                    file_browser.Open();

                draw_source_files();

                bool *p_show_loader = module_panel.is_empty() ? nullptr : &show_loader;

                // only let the user close the loader once there are modules loaded
                if (module_panel.is_empty() || show_loader)
                {
                    if (ImGui::BeginTabItem("Loader", p_show_loader, ImGuiTabItemFlags_Leading | ImGuiTabItemFlags_NoTooltip))
                    {
                        draw_module_loader();
                        ImGui::EndTabItem();
                    }

                    if (!module_panel.is_empty())
                        show_loader = false;
                }
            }
            ImGui::EndTabBar();
        }
        ImGui::End();

        file_browser.Display();

        if (file_browser.HasSelected())
        {
            for (const auto &file : file_browser.GetMultiSelected())
            {
                char *path = arena_strdup(file.string().c_str(), get_global_arena());
                io_t *io = io_file(path, eOsAccessRead, get_global_arena());
                sources.emplace_back(io);
            }
            file_browser.ClearSelected();
        }
    }
};

int main(int argc, const char **argv)
{
    CT_UNUSED(argc);
    CT_UNUSED(argv);

    setup_global();
    ed::install_trace_arenas();

    if (!draw::create(L"Editor"))
    {
        return 1;
    }

    EditorUi ui;

    while (draw::begin_frame())
    {
        ui.dock_space();
        ui.draw_windows();

        draw::end_frame();
    }

    draw::destroy();
}
