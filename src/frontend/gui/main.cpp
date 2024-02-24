// SPDX-License-Identifier: GPL-3.0-only

#include "interop/compile.h"
#include "io/io.h"
#include "setup/setup.h"
#include "editor/compile.hpp"
#include "editor/config.hpp"
#include "editor/draw.hpp"
#include "editor/sources.hpp"
#include "editor/trace.hpp"

#include "core/macros.h"

#include "editor/panic.hpp"

#include "config/config.h"

#include "imgui.h"
#include "std/typed/vector.h"
#include "support/loader.h"
#include "support/support.h"

static const frontend_t kFrontendGui = {
    .info = {
        .id = "frontend-gui",
        .name = "Cthulhu GUI",
        .version = {
            .license = "GPLv3",
            .desc = "Cthulhu Compiler Collection GUI",
            .author = "Elliot Haisley",
            .version = CT_NEW_VERSION(0, 0, 1),
        },
    },
};

class CompileRun : public ed::Broker
{
public:
    CompileRun(loader_t *loader, const char *name)
        : Broker(loader, name)
    {
        init_config();
    }

    bool show = true;
    ed::CompileError error = {};

    cfg_group_t *config = nullptr;
    cfg_field_t *cfg_int = nullptr;
    cfg_field_t *cfg_bool = nullptr;
    cfg_field_t *cfg_string = nullptr;
    cfg_field_t *cfg_enum = nullptr;
    cfg_field_t *cfg_flags = nullptr;

    cfg_info_t root_info = {
        .name = name.c_str(),
        .brief = "Compile run configuration"
    };

    cfg_info_t test_group_info = {
        .name = "test_group",
        .brief = "Test group to demonstrate config"
    };

    constexpr static const char *kTestIntArgsLong[] = { "test-int", NULL };

    cfg_info_t test_int_info = {
        .name = "test_int",
        .brief = "Test integer",
        .long_args = kTestIntArgsLong
    };

    cfg_int_t test_int_config = {
        .initial = 4,
        .min = 3,
        .max = 99
    };

    constexpr static const char *kTestBoolArgs[] = { "test-bool", NULL };

    cfg_info_t test_bool_info = {
        .name = "test_bool",
        .brief = "Test boolean",
        .long_args = kTestBoolArgs
    };

    constexpr static const char *kTestStringArgs[] = { "test-string", NULL };

    cfg_info_t test_string_info = {
        .name = "test_string",
        .brief = "Test string",
        .long_args = kTestStringArgs
    };

    constexpr static const char *kTestEnumArgs[] = { "test-enum", NULL };

    cfg_info_t test_enum_info = {
        .name = "test_enum",
        .brief = "Test enum",
        .long_args = kTestEnumArgs
    };

    cfg_choice_t test_enum_choices[3] = {
        { "one", 1 },
        { "two", 2 },
        { "three", 3 }
    };

    cfg_enum_t test_enum_config = {
        .options = test_enum_choices,
        .count = std::size(test_enum_choices),
        .initial = 2
    };

    constexpr static const char *kTestFlagsArgs[] = { "test-flags", NULL };

    cfg_info_t test_flags_info = {
        .name = "test_flags",
        .brief = "Test flags",
        .long_args = kTestFlagsArgs
    };

    cfg_choice_t test_flags_choices[3] = {
        { "bibble", (1 << 0) },
        { "bojangles", (1 << 1) },
        { "lettuce", (1 << 2) }
    };

    cfg_flags_t test_flags_config = {
        .options = test_flags_choices,
        .count = std::size(test_flags_choices),
        .initial = (1 << 0) | (1 << 2)
    };

    void init_config()
    {
        config = config_root(&root_info, &global);

        cfg_group_t *test_group = config_group(config, &test_group_info);

        cfg_int = config_int(test_group, &test_int_info, test_int_config);
        cfg_bool = config_bool(test_group, &test_bool_info, true);
        cfg_string = config_string(test_group, &test_string_info, "hello world");
        cfg_enum = config_enum(test_group, &test_enum_info, test_enum_config);
        cfg_flags = config_flags(test_group, &test_flags_info, test_flags_config);
    }

    bool show_config = false;
    bool show_memory = false;

    void draw_config()
    {
        if (!show_config) return;

        char label[128] = {};
        (void)snprintf(label, std::size(label), "%s Config", name.c_str());

        if (ImGui::Begin(label, &show_config))
        {
            ed::draw_config_panel(config);
        }
        ImGui::End();
    }

    void draw_memory()
    {
        if (!show_memory) return;

        char label[128] = {};

        (void)snprintf(label, std::size(label), "%s Global memory", name.c_str());
        if (ImGui::Begin(label, &show_memory))
        {
            global.draw_info();
        }
        ImGui::End();

        (void)snprintf(label, std::size(label), "%s GMP memory", name.c_str());
        if (ImGui::Begin(label, &show_memory))
        {
            gmp.draw_info();
        }
        ImGui::End();
    }

    void draw()
    {
        if (!show) return;

        if (ImGui::Begin(name.c_str(), &show, ImGuiWindowFlags_MenuBar))
        {
            if (ImGui::BeginMenuBar())
            {
                ImGui::TextUnformatted(name.c_str());
                ImGui::Separator();

                if (ImGui::BeginMenu("View"))
                {
                    ImGui::MenuItem("Config", nullptr, &show_config);
                    ImGui::MenuItem("Memory", nullptr, &show_memory);
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            draw_compile();

            if (ImGui::CollapsingHeader("Sources", ImGuiTreeNodeFlags_DefaultOpen))
            {
                sources.draw();
            }
        }

        ImGui::End();

        draw_config();
        draw_memory();
    }

public:
    void draw_compile()
    {
        bool can_compile = true;
        if (sources.is_empty())
        {
            can_compile = false;
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "No sources");
        }
        else if (error.has_error())
        {
            can_compile = false;
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Error");
        }

        ImGui::BeginDisabled(!can_compile);
        if (ImGui::Button("Compile"))
        {
            error = ed::run_compile(*this);
        }
        ImGui::EndDisabled();

        draw_error();
    }

    void draw_error()
    {
        switch (error.code)
        {
        case ed::eCompilePanic:
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Panic");
            error.panic.draw();
            break;

        case ed::eCompileError:
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Error: %s", error.error.c_str());
            break;

        default:
            break;
        }
    }
};

class ModuleInfo
{
    const module_info_t& info;

    static void draw_version(const char *id, version_t version)
    {
        int major = CT_VERSION_MAJOR(version);
        int minor = CT_VERSION_MINOR(version);
        int patch = CT_VERSION_PATCH(version);

        ImGui::Text("%s: %d.%d.%d", id, major, minor, patch);
    }

    void draw_diagnostics(diagnostic_list_t list) const
    {
        if (ImGui::TreeNode((void*)this, "Diagnostics: %zu", list.count))
        {
            for (size_t i = 0; i < list.count; i++)
            {
                const diagnostic_t *diag = list.diagnostics[i];
                char label[128];
                (void)std::snprintf(label, std::size(label), "Diagnostic %s | %s", diag->id, severity_name(diag->severity));
                ImGui::SeparatorText(label);
                ImGui::TextWrapped("%s", diag->brief ? diag->brief : "no brief");
                ImGui::TextWrapped("%s", diag->description ? diag->description : "no description");
            }
            ImGui::TreePop();
        }
    }

public:
    const char *get_name() const { return info.name; }
    ModuleInfo(const module_info_t& info)
        : info(info)
    { }

    void draw_info() const
    {
        ImGui::Text("ID: %s", info.id);

        version_info_t version = info.version;
        ImGui::Text("License: %s", version.license);
        ImGui::Text("Description: %s", version.desc);
        ImGui::Text("Author: %s", version.author);
        draw_version("Version", version.version);
        draw_diagnostics(info.diagnostics);
    }

    void draw_body() const
    {
        ImGui::Text("Name: %s", info.name);
        draw_info();
    }

    void draw_window()
    {
        if (!show) return;

        if (ImGui::Begin(info.name, &show))
        {
            draw_body();
        }
        ImGui::End();
    }

    bool show = true;
};

class LanguageModule : public ModuleInfo
{
    const language_t *lang;

    std::string builtin_name;
    std::string exts;

public:
    LanguageModule(const language_t *lang)
        : ModuleInfo(lang->info)
        , lang(lang)
    {
        language_info_t builtins = lang->builtin;
        builtin_name.resize(builtins.name.length);
        for (size_t i = 0; i < builtins.name.length; i++)
        {
            char c = builtins.name.text[i];
            if (c == '\0')
            {
                builtin_name[i] = '/';
            }
            else
            {
                builtin_name[i] = c;
            }
        }

        for (size_t i = 0; lang->exts[i]; i++)
        {
            if (i > 0)
                exts += ", ";
            exts += lang->exts[i];
        }
    }

    void draw_body()
    {
        module_info_t mod = lang->info;
        if (ImGui::CollapsingHeader(mod.name))
        {
            ModuleInfo::draw_info();
            ImGui::Text("Default extensions: %s", exts.c_str());
            ImGui::Text("Context size: %zu", lang->context_size);
            ImGui::Text("AST size: %zu", lang->ast_size);

            if (ImGui::TreeNode((void*)&lang->builtin, "Builtin"))
            {
                ImGui::BulletText("Builtin module: %s", builtin_name.c_str());
                language_info_t builtin = lang->builtin;
                for (size_t i = 0; i < builtin.length; i++)
                {
                    const char *name = builtin.names ? builtin.names[i] : "unknown";
                    ImGui::BulletText("Initial size %s: %zu", name, builtin.decls[i]);
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNode((void*)lang, "Callbacks"))
            {
                ImGui::BulletText("Create %p", lang->fn_create);
                ImGui::BulletText("Destroy %p", lang->fn_destroy);

                ImGui::BulletText("Preparse %p", lang->fn_preparse);
                ImGui::BulletText("Postparse %p", lang->fn_postparse);
                ImGui::TreePop();
            }

            const scan_callbacks_t *scan = lang->scanner;
            if (ImGui::TreeNode((void*)&lang->scanner, "Scanner %p", scan))
            {
                if (scan != nullptr)
                {
                    ImGui::BulletText("Init %p", scan->init);
                    ImGui::BulletText("Parse %p", scan->parse);
                    ImGui::BulletText("Scan %p", scan->scan);
                    ImGui::BulletText("Destroy buffer %p", scan->destroy_buffer);
                    ImGui::BulletText("Destroy %p", scan->destroy);
                }
                else
                {
                    ImGui::Text("No scanner");
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode((void*)&lang->fn_passes, "Passes"))
            {
                for (size_t i = 0; i < ePassCount; i++)
                {
                    broker_pass_t pass = static_cast<broker_pass_t>(i);
                    ImGui::BulletText("Pass %s: %p", broker_pass_name(pass), lang->fn_passes[i]);
                }
                ImGui::TreePop();
            }
        }
    }
};

class PluginModule : public ModuleInfo
{
    const plugin_t *plugin;

public:
    PluginModule(const plugin_t *plugin)
        : ModuleInfo(plugin->info)
        , plugin(plugin)
    { }

    void draw_body()
    {
        module_info_t mod = plugin->info;
        char label[128] = {};
        (void)snprintf(label, std::size(label), "Plugin: %s", mod.name);
        ImGui::SeparatorText(label);

        ModuleInfo::draw_info();
        ImGui::Text("Create %p", plugin->fn_create);
        ImGui::Text("Destroy %p", plugin->fn_destroy);

        ImGui::SeparatorText("Events");
        event_list_t events = plugin->events;
        for (size_t i = 0; i < events.count; i++)
        {
            ImGui::BulletText("Event %zu: %d", i, events.events[i].event);
        }
    }
};

class TargetModule : public ModuleInfo
{
    const target_t *target;

public:
    TargetModule(const target_t *target)
        : ModuleInfo(target->info)
        , target(target)
    { }

    void draw_body()
    {
        module_info_t mod = target->info;
        char label[128] = {};
        (void)snprintf(label, std::size(label), "Target: %s", mod.name);
        ImGui::SeparatorText(label);

        ModuleInfo::draw_info();
        ImGui::Text("Create %p", target->fn_create);
        ImGui::Text("Destroy %p", target->fn_destroy);
        ImGui::Text("Tree output %s", target->fn_tree ? "supported" : "unsupported");
        ImGui::Text("SSA output %s", target->fn_ssa ? "supported" : "unsupported");
    }
};

template<typename T>
void draw_module(const T& mod)
{
    if (ImGui::CollapsingHeader(mod.get_name()))
    {
        mod.draw_info();
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

    void init(arena_t *arena)
    {
        path = io_name(io);

        size = io_size(io);
        text = (char*)io_map(io, eOsProtectRead);

        error = io_error(io);
        if (error)
        {
            str = os_error_string(error, arena);
        }
    }
public:
    SourceCode(io_t *io, arena_t *arena)
        : io(io)
    {
        init(arena);
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

class EditorUi
{
    ed::TraceArena global{ "global", ed::TraceArena::eDrawTree };
    bool show_demo_window = false;
    bool show_version_window = false;
    bool default_modules_loaded = false;
    bool show_module_info = false;

    loader_t *loader;
    broker_t *broker;
    support_t *support;

    std::vector<LanguageModule> languages;
    std::vector<PluginModule> plugins;
    std::vector<TargetModule> targets;

    std::vector<SourceCode> sources;

    void draw_source_files()
    {
        for (auto &source : sources)
        {
            source.draw_body();
        }
    }

    void add_module(const loaded_module_t &mod)
    {
        if (mod.type & eModLanguage)
        {
            languages.emplace_back(mod.lang);
        }

        if (mod.type & eModPlugin)
        {
            plugins.emplace_back(mod.plugin);
        }

        if (mod.type & eModTarget)
        {
            targets.emplace_back(mod.target);
        }
    }

    ModuleInfo version_info{ kFrontendGui.info };

    static void draw_runtime_version()
    {
        ImGui::Text("Debug: %s", CTU_DEBUG ? "true" : "false");
        ImGui::Text("Runtime version: %d.%d.%d", CTU_MAJOR, CTU_MINOR, CTU_PATCH);
    }

    void draw_version_info()
    {
        if (!show_version_window) return;

        if (ImGui::Begin("Version", &show_version_window))
        {
            version_info.draw_body();
            draw_runtime_version();
        }
        ImGui::End();
    }

    void load_default_modules()
    {
        support_load_default_modules(support);
        default_modules_loaded = true;

        typevec_t *mods = support_get_modules(support);
        size_t len = typevec_len(mods);

        for (size_t i = 0; i < len; i++)
        {
            loaded_module_t mod = {};
            typevec_get(mods, i, &mod);
            add_module(mod);
        }
    }

    void draw_module_info()
    {
        if (!show_module_info) return;

        if (ImGui::Begin("Modules", &show_module_info))
        {
            if (ImGui::BeginTabBar("ModuleTabs"))
            {
                if (ImGui::BeginTabItem("Languages"))
                {
                    for (auto &lang : languages)
                    {
                        draw_module(lang);
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Plugins"))
                {
                    for (auto &plugin : plugins)
                    {
                        draw_module(plugin);
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Targets"))
                {
                    for (auto &target : targets)
                    {
                        draw_module(target);
                    }

                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }

public:
    EditorUi()
        : loader(loader_new(&global))
        , broker(broker_new(&kFrontendGui, &global))
        , support(support_new(broker, loader, &global))
    { }

    void draw_windows()
    {
        draw_demo_window();
        draw_setup_window();
        draw_version_info();
        draw_module_info();
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
                }

                if (ImGui::MenuItem("Dark"))
                {
                    ImGui::StyleColorsDark();
                }

                if (ImGui::MenuItem("Light"))
                {
                    ImGui::StyleColorsLight();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Windows"))
            {
                ImGui::SeparatorText("ImGui");
                ImGui::MenuItem("Dear ImGui Demo", nullptr, &show_demo_window);

                ImGui::SeparatorText("Info");
                ImGui::MenuItem("Version", nullptr, &version_info.show);

                ImGui::SeparatorText("Modules");
                ImGui::MenuItem("Module Info", nullptr, &show_module_info);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        ImGui::End();
    }

private:
    void draw_demo_window()
    {
        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }
    }

    bool show_loader = true;

    void draw_module_loader()
    {
        if (!default_modules_loaded)
        {
            if (ImGui::Button("Load default modules"))
            {
                load_default_modules();
            }
        }

        ImGui::InputText("Load shared module", module_path, std::size(module_path));
        ImGui::CheckboxFlags("Language", &mask, eModLanguage);
        ImGui::SameLine(); ImGui::CheckboxFlags("Plugin", &mask, eModPlugin);
        ImGui::SameLine(); ImGui::CheckboxFlags("Target", &mask, eModTarget);
        ImGui::BeginDisabled(mask == eModNone || module_path[0] == '\0');
        if (ImGui::Button("Load"))
        {
            loaded_module_t mod = {};
            if (support_load_module(support, module_type_t(mask), module_path, &mod))
            {
                add_module(mod);
            }
            else
            {
                error = load_error_string(mod.error);
                os_error = os_error_string(mod.os, &global);
            }
        }
        ImGui::EndDisabled();

        if (error.length() > 0)
        {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Error: %s", error.c_str());
        }

        if (os_error.length() > 0)
        {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Error: %s", os_error.c_str());
        }
    }

    static constexpr ImGuiWindowFlags kMainFlags = ImGuiWindowFlags_NoDecoration
                                                 | ImGuiWindowFlags_NoMove;

    char module_path[256] = { 0 };
    int mask = eModNone;

    std::string error;
    std::string os_error;

    char source_path[512] = { 0 };

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
                    ImGui::OpenPopup("AddSource");

                if (ImGui::BeginPopup("AddSource"))
                {
                    ImGui::InputText("Path", source_path, std::size(source_path));
                    if (ImGui::Button("Add"))
                    {
                        io_t *io = io_file(source_path, eOsAccessRead, &global);
                        sources.emplace_back(io, &global);
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                draw_source_files();

                bool *p_show_loader = (sources.empty() && !default_modules_loaded) ? nullptr : &show_loader;

                if (ImGui::BeginTabItem("Loader", p_show_loader, ImGuiTabItemFlags_Leading | ImGuiTabItemFlags_NoTooltip))
                {
                    draw_module_loader();
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
};

int main(int argc, const char **argv)
{
    CT_UNUSED(argc);
    CT_UNUSED(argv);

    if (!draw::create(L"Editor"))
    {
        return 1;
    }

    setup_global();
    ed::install_panic_handler();

    EditorUi ui;

    while (draw::begin_frame())
    {
        ui.dock_space();
        ui.draw_windows();

        draw::end_frame();
    }

    draw::destroy();
}
