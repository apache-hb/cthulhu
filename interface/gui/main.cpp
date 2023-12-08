#include "editor/draw.hpp"
#include "editor/arena.hpp"
#include "editor/config.hpp"

#include "core/macros.h"

#include "os/os.h"

#include "config/config.h"

#include "imgui.h"

#include "stacktrace/stacktrace.h"
#include "std/str.h"
#include "std/vector.h"

#include <map>
#include <vector>

struct AllocInfo
{
    size_t size;
    const char *name;
    const void *parent;
};

struct TraceAlloc final : public ed::IAlloc
{
    using IAlloc::IAlloc;

    void *malloc(size_t size, const char *id, const void *parent) override
    {
        malloc_calls += 1;

        void *ptr = ::malloc(size);

        add_alloc(ptr, AllocInfo { size, id, parent });

        return ptr;
    }

    void *realloc(void *ptr, size_t new_size, size_t) override
    {
        realloc_calls += 1;

        void *new_ptr = ::realloc(ptr, new_size);

        if (auto it = allocs.find(ptr); it != allocs.end())
        {
            AllocInfo info = it->second;

            peak_usage -= info.size;
            info.size = new_size;

            add_alloc(new_ptr, info);
            remove_alloc(it);
        }

        return new_ptr;
    }

    void free(void *ptr, size_t) override
    {
        free_calls += 1;

        if (auto it = allocs.find(ptr); it != allocs.end())
        {
            remove_alloc(it);
        }

        ::free(ptr);
    }

    void draw_info()
    {
        draw_usage();
        draw_body();
    }

private:
    // number of calls to each function
    size_t malloc_calls = 0;
    size_t realloc_calls = 0;
    size_t free_calls = 0;

    // peak memory usage
    size_t peak_usage = 0;

    using AllocMap = std::map<const void*, AllocInfo>;
    using AllocTree = std::map<const void*, std::vector<const void*>>;
    using AllocMapIter = AllocMap::iterator;

    // all live allocations
    AllocMap allocs = {};

    // tree of allocations
    AllocTree tree = {};

    void add_alloc(const void *ptr, AllocInfo info)
    {
        peak_usage += info.size;
        allocs.emplace(ptr, info);

        if (info.parent)
        {
            tree[info.parent].push_back(ptr);
        }
    }

    void remove_alloc(AllocMapIter iter)
    {
        allocs.erase(iter);

        for (auto& [parent, children] : tree)
        {
            auto it = std::find(children.begin(), children.end(), iter->first);
            if (it != children.end())
            {
                children.erase(it);
                break;
            }
        }
    }

    bool has_children(const void *ptr) const
    {
        if (auto it = tree.find(ptr); it != tree.end())
        {
            return it->second.size() > 0;
        }

        return false;
    }

    bool has_parent(const void *ptr) const
    {
        if (auto it = allocs.find(ptr); it != allocs.end())
        {
            return it->second.parent != nullptr;
        }

        return false;
    }

private:
    void draw_usage() const
    {
        ImGui::Text("malloc: %zu", malloc_calls);
        ImGui::Text("realloc: %zu", realloc_calls);
        ImGui::Text("free: %zu", free_calls);
        ImGui::Text("peak usage: %zu", peak_usage);
    }

    enum : int { eDrawTree, eDrawFlat };
    int draw_mode = eDrawTree;

    void draw_body()
    {
        ImGui::RadioButton("Tree", &draw_mode, eDrawTree);
        ImGui::SameLine();
        ImGui::RadioButton("Flat", &draw_mode, eDrawFlat);

        if (draw_mode == eDrawTree)
        {
            draw_tree();
        }
        else
        {
            draw_flat();
        }
    }

    static void draw_name(const char *name)
    {
        if (name)
        {
            ImGui::Text("%s", name);
        }
        else
        {
            ImGui::TextDisabled("---");
        }
    }

    static const ImGuiTreeNodeFlags kGroupNodeFlags
        = ImGuiTreeNodeFlags_SpanAllColumns
        | ImGuiTreeNodeFlags_AllowOverlap;

    static const ImGuiTreeNodeFlags kValueNodeFlags
        = kGroupNodeFlags
        | ImGuiTreeNodeFlags_Leaf
        | ImGuiTreeNodeFlags_Bullet
        | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    void draw_tree_child(const void *ptr, const AllocInfo& info) const
    {
        ImGui::TreeNodeEx(ptr, kValueNodeFlags, "%p", ptr);

        ImGui::TableNextColumn();
        ImGui::Text("%zu", info.size);

        ImGui::TableNextColumn();
        draw_name(info.name);
    }

    void draw_tree_group(const void *ptr, const AllocInfo& info) const
    {
        bool is_open = ImGui::TreeNodeEx(ptr, kGroupNodeFlags, "%p", ptr);

        ImGui::TableNextColumn();
        ImGui::Text("%zu", info.size);

        ImGui::TableNextColumn();
        draw_name(info.name);

        if (is_open)
        {
            for (const void *child : tree.at(ptr))
            {
                draw_tree_node(child);
            }

            ImGui::TreePop();
        }
    }

    void draw_tree_node(const void *ptr) const
    {
        if (auto it = allocs.find(ptr); it != allocs.end())
        {
            const AllocInfo& info = it->second;

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (has_children(ptr))
            {
                draw_tree_group(ptr, info);
            }
            else
            {
                draw_tree_child(ptr, info);
            }
        }
    }

    void draw_tree() const
    {
        ImGui::SeparatorText("Memory tree view");

        if (ImGui::BeginTable("Allocations", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Address");
            ImGui::TableSetupColumn("Size");
            ImGui::TableSetupColumn("Name");
            ImGui::TableHeadersRow();

            // first draw all nodes that dont have parents but have children
            for (auto& [ptr, children] : tree)
            {
                if (has_children(ptr) && !has_parent(ptr))
                {
                    draw_tree_node(ptr);
                }
            }

            // draw all nodes that don't have parents and don't have children
            for (auto& [ptr, info] : allocs)
            {
                if (info.parent != nullptr || has_children(ptr)) continue;

                draw_tree_node(ptr);
            }

            ImGui::EndTable();
        }
    }

    void draw_flat() const
    {
        ImGui::SeparatorText("Memory view");

        if (ImGui::BeginTable("Allocations", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Address");
            ImGui::TableSetupColumn("Size");
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Parent");
            ImGui::TableHeadersRow();

            for (auto& [ptr, info] : allocs)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::Text("%p", ptr);

                ImGui::TableNextColumn();
                ImGui::Text("%zu", info.size);

                ImGui::TableNextColumn();
                draw_name(info.name);

                ImGui::TableNextColumn();
                if (info.parent)
                {
                    ImGui::Text("%p", info.parent);
                }
                else
                {
                    ImGui::TextDisabled("null");
                }
            }

            ImGui::EndTable();
        }
    }
};

struct CompileRun
{
    CompileRun(const char *id)
        : name(_strdup(id))
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

    void draw_window()
    {
        if (!show) return;

        if (ImGui::Begin(name, &show))
        {
            if (ImGui::CollapsingHeader("Config", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ed::draw_config_panel(config);
            }

            if (ImGui::CollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen))
            {
                alloc.draw_info();
            }
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

    char compile_name[256] = { 0 };

    bool compile_run_exists(const char *id) const
    {
        for (const CompileRun& run : compile_runs)
        {
            if (str_equal(run.name, id))
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
                    compile_runs.emplace_back(compile_name);
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
