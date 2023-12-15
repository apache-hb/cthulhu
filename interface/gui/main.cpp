#include "editor/draw.hpp"
#include "editor/sources.hpp"
#include "editor/trace.hpp"
#include "editor/config.hpp"

#include "core/macros.h"
#include "core/version_def.h"

#include "cthulhu/mediator/interface.h"

#include "editor/panic.hpp"
#include "memory/memory.h"
#include "support/langs.h"

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

static std::jmp_buf gPanicJmp;
static ed::PanicInfo gPanicInfo = {};

static void install_panic_handler()
{
    gPanicHandler = [](panic_t panic, const char *fmt, va_list args) {
        gPanicInfo.capture_trace(panic, fmt, args);
        std::longjmp(gPanicJmp, 1);
    };
}

struct CompileRun
{
    CompileRun(const char *id, mediator_t *instance)
        : name(id)
    {
        init_config();

        mediator = instance;
    }

    mediator_t *mediator = nullptr;

    void lifetime_configure()
    {
        alloc.install();
        gmp_alloc.install_gmp();

        lifetime = lifetime_new(mediator, &alloc);

        langs_t langs = get_langs();
        // TODO: configure languages as well
        for (size_t i = 0; i < langs.size; i++)
        {
            lifetime_add_language(lifetime, langs.langs + i);
        }
    }

    std::string name;
    bool show = true;

    ed::TraceArena alloc{"default", ed::TraceArena::eDrawTree};
    ed::TraceArena gmp_alloc{"gmp", ed::TraceArena::eDrawFlat};

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
        config = config_new(&alloc, &root_info);

        config_t *test_group = config_group(config, &test_group_info);

        cfg_int = config_int(test_group, &test_int_info, test_int_config);
        cfg_bool = config_bool(test_group, &test_bool_info, test_bool_config);
        cfg_string = config_string(test_group, &test_string_info, test_string_config);
        cfg_enum = config_enum(test_group, &test_enum_info, test_enum_config);
        cfg_flags = config_flags(test_group, &test_flags_info, test_flags_config);
    }

    ed::SourceList sources;

    lifetime_t *lifetime = nullptr;
    reports_t *reports = nullptr;

    char *compile_preinit_error = nullptr;

    bool parse_file(const char *path)
    {
        const char *ext = str_ext(path);
        if (ext == nullptr)
        {
            compile_preinit_error = format("could not determine file extension for '%s'", path);
            return false;
        }

        const language_t *lang = lifetime_get_language(lifetime, ext);
        if (lang == nullptr)
        {
            const char *basepath = str_filename(path);
            compile_preinit_error = format("could not find language for `%s` by extension `%s`", basepath, ext);
            return false;
        }

        io_t *io = io_file(path, eAccessRead);
        if (os_error_t err = io_error(io); err != 0)
        {
            compile_preinit_error = format("could not open file '%s' (os error: %s)", path, os_error_string(err));
            return false;
        }

        lifetime_parse(lifetime, lang, io);
        return true;
    }

    ed::PanicInfo panic_info = {};

    bool do_compile()
    {
        if (std::setjmp(gPanicJmp))
        {
            return false;
        }

        lifetime_configure();
        reports = lifetime_get_reports(lifetime);

        for (size_t i = 0; i < sources.count(); i++)
        {
            if (!parse_file(sources.get(i)))
            {
                return false;
            }
        }

        return true;
    }

    void draw_compile()
    {
        bool can_compile = true;
        if (sources.is_empty())
        {
            can_compile = false;
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "No sources");
        }

        ImGui::BeginDisabled(!can_compile || panic_info.has_info());
        if (ImGui::Button("Compile"))
        {
            bool ok = do_compile();

            // do this here instead of in do_compile
            // because setjmp does not agree with copy constructors
            if (!ok && gPanicInfo.has_info())
            {
                panic_info = gPanicInfo;
                gPanicInfo.reset();
            }
        }
        ImGui::EndDisabled();

        if (panic_info.has_info())
        {
            panic_info.draw();
        }
        else if (compile_preinit_error)
        {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "%s", compile_preinit_error);
        }
    }

    void draw_window()
    {
        if (!show) return;

        if (ImGui::Begin(name.c_str(), &show))
        {
            draw_compile();

            if (ImGui::CollapsingHeader("Sources", ImGuiTreeNodeFlags_DefaultOpen))
            {
                sources.draw();
            }

            if (ImGui::CollapsingHeader("Config", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ed::draw_config_panel(config);
            }

            if (ImGui::BeginTabBar("Memory", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Default"))
                {
                    alloc.draw_info();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("GMP"))
                {
                    gmp_alloc.draw_info();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }

        ImGui::End();
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
                    compile_runs.emplace_back(compile_name, mediator);
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
            run.draw_window();
        }
    }
};

// init everything that needs to be setup only once
void init_global()
{
    install_panic_handler();

    os_init();
    stacktrace_init();
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
