#include "editor/draw.h"
#include "editor/alloc.h"

#include "core/macros.h"

#include "os/os.h"

#include "config/config.h"

#include "imgui.h"
#include "stacktrace/stacktrace.h"
#include "std/vector.h"

#include <map>
#include <vector>

struct AllocInfo
{
    size_t size;
    const char *name;
    const void *parent;
};

struct TraceAlloc : public IAlloc
{
    using IAlloc::IAlloc;

    std::map<const void*, AllocInfo> allocs = {};

    void *malloc(size_t size, const char *id, const void *parent) override
    {
        void *ptr = ::malloc(size);

        allocs.emplace(ptr, AllocInfo { size, id, parent });

        return ptr;
    }

    void *realloc(void *ptr, size_t new_size, size_t) override
    {
        void *new_ptr = ::realloc(ptr, new_size);

        auto it = allocs.find(ptr);

        if (it != allocs.end())
        {
            AllocInfo info = it->second;
            info.size = new_size;
            allocs.emplace(new_ptr, info);
            allocs.erase(it);
        }

        return new_ptr;
    }

    void free(void *ptr, size_t) override
    {
        auto it = allocs.find(ptr);

        if (it != allocs.end())
        {
            allocs.erase(it);
        }

        ::free(ptr);
    }
};

struct CompileRun
{
    CompileRun(const char *name)
        : name(name)
        , alloc(name)
    {
        init_config();
    }

    const char *name;
    bool show = true;

    TraceAlloc alloc;

    config_t *config = nullptr;

    cfg_field_t *cfg_int = nullptr;
    cfg_field_t *cfg_bool = nullptr;
    cfg_field_t *cfg_string = nullptr;
    cfg_field_t *cfg_enum = nullptr;
    cfg_field_t *cfg_flags = nullptr;

    cfg_info_t root_info = {
        .name = name,
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
        .arg_long = "test-enum",
        .arg_short = "e"
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
        .arg_long = "test-flags",
        .arg_short = "f"
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

    static const ImGuiTableFlags kTableFlags
        = ImGuiTableFlags_BordersV
        | ImGuiTableFlags_BordersOuterH
        | ImGuiTableFlags_Resizable
        | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_NoHostExtendX
        | ImGuiTableFlags_NoBordersInBody;

    static const ImGuiTreeNodeFlags kGroupNodeFlags
        = ImGuiTreeNodeFlags_SpanAllColumns
        | ImGuiTreeNodeFlags_AllowOverlap;

    static const ImGuiTreeNodeFlags kValueNodeFlags
        = kGroupNodeFlags
        | ImGuiTreeNodeFlags_Leaf
        | ImGuiTreeNodeFlags_Bullet
        | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    const char *get_name(const cfg_field_t *field)
    {
        const cfg_info_t *info = cfg_get_info(field);
        return info->name;
    }

    void get_label(char *buf, size_t size, const cfg_field_t *field)
    {
        snprintf(buf, size, "##%s", get_name(field));
    }

    void draw_info(const cfg_info_t *info)
    {
        ImGui::TextDisabled("%s", info->name);
        if (ImGui::BeginItemTooltip())
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.f);
            const char *brief = info->brief != nullptr ? info->brief : "no brief";
            ImGui::Text("brief: %s", brief);

            if (info->description != nullptr)
            {
                ImGui::Separator();
                ImGui::Text("description: %s", info->description);
            }

            if (info->arg_long != nullptr && info->arg_short != nullptr)
            {
                ImGui::Separator();
                ImGui::Text("args: %s / %s", info->arg_long, info->arg_short);
            }
            else if (info->arg_long != nullptr)
            {
                ImGui::Separator();
                ImGui::Text("long: %s", info->arg_long);
            }
            else if (info->arg_short != nullptr)
            {
                ImGui::Separator();
                ImGui::Text("short: %s", info->arg_short);
            }

            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    void draw_bool(cfg_field_t *field)
    {
        char label[64];
        get_label(label, std::size(label), field);

        bool value = cfg_bool_value(field);
        if (ImGui::Checkbox(label, &value))
        {
            cfg_set_bool(cfg_bool, value);
        }
    }

    void draw_int(const cfg_field_t *field)
    {
        char label[64];
        get_label(label, std::size(label), field);

        const cfg_int_t *cfg = cfg_int_info(field);
        int value = cfg_int_value(field);
        if (ImGui::DragInt(label, &value, 1.f, cfg->min, cfg->max))
        {
            cfg_set_int(cfg_int, value);
        }
    }

    void draw_string(const cfg_field_t *field)
    {
        char label[64];
        get_label(label, std::size(label), field);

        const char *value = cfg_string_value(field);
        char buffer[256] = { 0 };
        strncpy_s(buffer, value, std::size(buffer));
        if (ImGui::InputText(label, buffer, std::size(buffer)))
        {
            cfg_set_string(cfg_string, buffer);
        }
    }

    void draw_value(cfg_field_t *field)
    {
        cfg_type_t type = cfg_get_type(field);
        switch (type)
        {
        case eConfigBool:
            draw_bool(field);
            break;

        case eConfigInt:
            draw_int(field);
            break;

        case eConfigString:
            draw_string(field);
            break;

        default:
            ImGui::TextDisabled("Unknown type");
            break;
        }
    }

    void draw_int_constraints(const cfg_field_t *field)
    {
        const cfg_int_t *cfg = cfg_int_info(field);
        ImGui::Text("(%d, %d)", cfg->min, cfg->max);
    }

    void draw_constraints(const cfg_field_t *field)
    {
        cfg_type_t type = cfg_get_type(field);
        switch (type)
        {
        case eConfigBool:
            break;
        case eConfigInt:
            draw_int_constraints(field);
            break;
        case eConfigString:
            break;
        default:
            ImGui::TextDisabled("Unknown type");
            break;
        }
    }

    void draw_config_entry(cfg_field_t *field)
    {
        const cfg_info_t *info = cfg_get_info(field);
        cfg_type_t type = cfg_get_type(field);

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TreeNodeEx(info->name, kValueNodeFlags);

        ImGui::TableNextColumn();
        ImGui::TextUnformatted(cfg_type_name(type));

        ImGui::TableNextColumn();
        draw_value(field);

        ImGui::TableNextColumn();
        draw_constraints(field);

        ImGui::TableNextColumn();
        draw_info(info);
    }

    void draw_config_group(const config_t *group)
    {
        ImGui::PushID(group);

        ImGui::TableNextColumn();

        const cfg_info_t *info = cfg_group_info(group);
        bool is_group_open = ImGui::TreeNodeEx(info->name, kGroupNodeFlags);

        ImGui::TableNextColumn();
        ImGui::TextDisabled("--");

        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        draw_info(info);

        if (is_group_open)
        {
            vector_t *children = cfg_get_groups(group);
            size_t child_count = vector_len(children);
            for (size_t i = 0; i < child_count; ++i)
            {
                ImGui::TableNextRow();
                config_t *child = reinterpret_cast<config_t*>(vector_get(children, i));
                draw_config_group(child);
            }

            vector_t *fields = cfg_get_fields(group);
            size_t field_count = vector_len(fields);
            for (size_t i = 0; i < field_count; ++i)
            {
                ImGui::TableNextRow();
                cfg_field_t *field = reinterpret_cast<cfg_field_t*>(vector_get(fields, i));
                draw_config_entry(field);
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    void draw_config()
    {
        if (ImGui::BeginTable("Config", 5, kTableFlags))
        {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Value");
            ImGui::TableSetupColumn("Constraints");
            ImGui::TableSetupColumn("Info");

            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            draw_config_group(config);

            ImGui::EndTable();
        }
    }

    void draw_window()
    {
        if (!show) return;

        if (ImGui::Begin(name, &show))
        {
            draw_config();
        }

        ImGui::End();
    }
};

struct EditorUi
{
    bool show_demo_window = false;
    config_t *config = nullptr;
    std::vector<CompileRun> compile_runs;

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
                    ImGui::MenuItem(run.name, nullptr, &run.show);
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

    // init everything that needs to be setup only once
    void init_global()
    {
        os_init();
        stacktrace_init();
    }

    char compile_name[256] = { 0 };

    const char *compile_popup_id = "Compile Run Popup";

    void setup_window()
    {
        if (ImGui::Begin("Setup"))
        {
            if (ImGui::Button("Add Compile Run"))
            {
                ImGui::OpenPopup(compile_popup_id);
            }

            if (ImGui::BeginPopup(compile_popup_id))
            {
                ImGui::InputText("Name", compile_name, std::size(compile_name));

                if (ImGui::Button("Add"))
                {
                    compile_runs.emplace_back(compile_name);
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

int main(int argc, const char **argv)
{
    CTU_UNUSED(argc);
    CTU_UNUSED(argv);

    if (!draw::create())
    {
        return 1;
    }

    EditorUi ui;

    ui.init_global();

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
