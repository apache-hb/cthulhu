#include "editor/compile.hpp"
#include "editor/config.hpp"
#include "editor/draw.hpp"
#include "editor/sources.hpp"
#include "editor/trace.hpp"

#include "core/macros.h"
#include "core/version_def.h"

#include "cthulhu/mediator/interface.h"

#include "editor/panic.hpp"
#include "memory/memory.h"
#include "support/langs.h"

#include "report/report.h"

#include "scan/node.h"

#include "os/os.h"
#include "io/io.h"

#include "config/config.h"

#include "imgui.h"

#include "stacktrace/stacktrace.h"
#include "std/str.h"
#include "std/vector.h"

#include <csetjmp>

static const version_info_t kVersionInfo = {
    .license = "GPLv3",
    .desc = "Cthulhu Compiler Collection GUI",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 1)
};

struct CompileRun : ed::CompileInfo
{
    CompileRun(mediator_t *mediator, const char *name)
        : CompileInfo(mediator, name)
    {
        init_config();
    }

    bool show = true;
    ed::CompileError error = {};

    config_t *config = nullptr;
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

    cfg_info_t test_int_info = {
        .name = "test_int",
        .brief = "Test integer",
        .description = "This is a test integer",
        .arg_long = "test-int",
        .arg_short = "i"
    };

    cfg_int_t test_int_config = {
        .initial = 4,
        .min = 3,
        .max = 99
    };

    cfg_info_t test_bool_info = {
        .name = "test_bool",
        .brief = "Test boolean",
        .description = "This is a test boolean",
        .arg_long = "test-bool",
        .arg_short = "b"
    };

    cfg_bool_t test_bool_config = {
        .initial = true
    };

    cfg_info_t test_string_info = {
        .name = "test_string",
        .brief = "Test string",
        .description = "This is a test string",
        .arg_long = "test-string",
        .arg_short = "s"
    };

    cfg_string_t test_string_config = {
        .initial = "hello"
    };

    cfg_info_t test_enum_info = {
        .name = "test_enum",
        .brief = "Test enum",
        .description = "This is a test enum",
        .arg_short = "num"
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

    cfg_info_t test_flags_info = {
        .name = "test_flags",
        .brief = "Test flags",
        .description = "This is a test flags",
        .arg_long = "vegetables"
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
        config = config_new(&global, &root_info);

        config_t *test_group = config_group(config, &test_group_info);

        cfg_int = config_int(test_group, &test_int_info, test_int_config);
        cfg_bool = config_bool(test_group, &test_bool_info, test_bool_config);
        cfg_string = config_string(test_group, &test_string_info, test_string_config);
        cfg_enum = config_enum(test_group, &test_enum_info, test_enum_config);
        cfg_flags = config_flags(test_group, &test_flags_info, test_flags_config);
    }

    bool show_config = false;
    bool show_memory = false;

    void draw_config()
    {
        if (!show_config) return;

        char label[128] = {};
        snprintf(label, std::size(label), "%s Config", name.c_str());

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

        snprintf(label, std::size(label), "%s Global memory", name.c_str());
        if (ImGui::Begin(label, &show_memory))
        {
            global.draw_info();
        }
        ImGui::End();

        snprintf(label, std::size(label), "%s GMP memory", name.c_str());
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

private:
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

struct EditorUi
{
    bool show_demo_window = false;
    std::vector<CompileRun> compile_runs;

    mediator_t *mediator = nullptr;

    void init_mediator()
    {
        mediator = mediator_new_noinit("editor", kVersionInfo);
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
            ImGui::Text("Game2005");
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

    void demo_window()
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

    void setup_window()
    {
        if (ImGui::Begin("Setup"))
        {
            ImGui::InputText("Name", compile_name, std::size(compile_name));

            if (ImGui::Button("Add"))
            {
                if (strlen(compile_name) == 0)
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
    }

    void draw_compiles()
    {
        for (CompileRun& run : compile_runs)
        {
            run.draw();
        }
    }
};

// init everything that needs to be setup only once
void init_global()
{
    ed::install_panic_handler();

    os_init();
    stacktrace_init();
    scan_init();
}

int main(int argc, const char **argv)
{
    CTU_UNUSED(argc);
    CTU_UNUSED(argv);

    if (!draw::create(L"Editor"))
    {
        return 1;
    }

    init_global();

    EditorUi ui;

    init_global_alloc(ctu_default_alloc());
    ui.init_mediator();

    while (draw::begin_frame())
    {
        ui.dock_space();
        ui.demo_window();
        ui.setup_window();
        ui.draw_compiles();

        draw::end_frame();
    }

    draw::destroy();
}
