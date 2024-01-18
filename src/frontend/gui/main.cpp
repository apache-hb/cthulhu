#include "base/util.h"
#include "setup/setup.h"
#include "editor/compile.hpp"
#include "editor/config.hpp"
#include "editor/draw.hpp"
#include "editor/sources.hpp"
#include "editor/trace.hpp"

#include "core/macros.h"

#include "cthulhu/runtime/interface.h"

#include "editor/panic.hpp"

#include "config/config.h"

#include "imgui.h"

static const version_info_t kVersionInfo = {
    .license = "GPLv3",
    .desc = "Cthulhu Compiler Collection GUI",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 1)
};

class VersionInfo
{
public:
    VersionInfo(version_info_t info)
        : info(info)
    { }

    void draw()
    {
        if (!show) return;

        if (ImGui::Begin("Version info", &show))
        {
            ImGui::Text("Debug: %s", CTU_DEBUG ? "true" : "false");

            ImGui::Text("Framework version: %d.%d.%d", CTU_MAJOR, CTU_MINOR, CTU_PATCH);

            int major = VERSION_MAJOR(info.version);
            int minor = VERSION_MINOR(info.version);
            int patch = VERSION_PATCH(info.version);
            ImGui::Text("Frontend version: %d.%d.%d", major, minor, patch);

            ImGui::Text("Author: %s", info.author);
            ImGui::Text("License: %s", info.license);
            ImGui::Text("Description: %s", info.desc);
        }
        ImGui::End();
    }

public:
    static void draw_version(const char *id, version_t version)
    {
        int major = VERSION_MAJOR(version);
        int minor = VERSION_MINOR(version);
        int patch = VERSION_PATCH(version);

        ImGui::Text("%s: %d.%d.%d", id, major, minor, patch);
    }

    bool show = false;
    const version_info_t info;
};

class CompileRun : public ed::CompileInfo
{
public:
    CompileRun(mediator_t *mediator, const char *name)
        : CompileInfo(mediator, name)
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

class EditorUi
{
public:
    EditorUi()
    {
        mediator = mediator_new(&global);
    }

    bool show_demo_window = false;
    std::vector<CompileRun> compile_runs;

    VersionInfo version_info{ kVersionInfo };

    ed::TraceArena global{ "global", ed::TraceArena::eDrawTree };

    mediator_t *mediator = nullptr;

    void draw_windows()
    {
        draw_demo_window();
        draw_setup_window();

        version_info.draw();

        for (CompileRun& run : compile_runs)
        {
            run.draw();
        }
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

        if (ImGui::BeginMenuBar())
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

                ImGui::SeparatorText("Compiles");
                for (CompileRun& run : compile_runs)
                {
                    ImGui::MenuItem(run.name.c_str(), nullptr, &run.show);
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
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

    char compile_name[256] = { 0 };

    bool compile_run_exists(const char *id) const
    {
        for (const CompileRun& run : compile_runs)
        {
            if (run.name == id)
            {
                return true;
            }
        }

        return false;
    }

    const char *duplicate_compile_name_popup = "Duplicate Name";
    const char *empty_compile_name_popup = "Empty Name";

    void draw_setup_window()
    {
        if (ImGui::Begin("Setup"))
        {
            ImGui::InputText("Name", compile_name, std::size(compile_name));

            if (ImGui::Button("Add"))
            {
                if (ctu_strlen(compile_name) == 0)
                {
                    ImGui::OpenPopup(empty_compile_name_popup);
                }
                else if (compile_run_exists(compile_name))
                {
                    ImGui::OpenPopup(duplicate_compile_name_popup);
                }
                else
                {
                    compile_runs.emplace_back(mediator, compile_name);
                }
            }

            if (ImGui::BeginPopupModal(duplicate_compile_name_popup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Compile run with name '%s' already exists", compile_name);

                if (ImGui::Button("OK", ImVec2(120, 0)))
                {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            if (ImGui::BeginPopupModal(empty_compile_name_popup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Compile run name cannot be empty");

                if (ImGui::Button("OK", ImVec2(120, 0)))
                {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }

        ImGui::End();

        if (ImGui::Begin("Memory"))
        {
            global.draw_info();
        }
        ImGui::End();
    }
};

int main(int argc, const char **argv)
{
    CTU_UNUSED(argc);
    CTU_UNUSED(argv);

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