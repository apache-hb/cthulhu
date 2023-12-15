#include "editor/draw.hpp"
#include "editor/arena.hpp"
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

#include <setjmp.h>

#include <map>
#include <string>
#include <vector>

static const version_info_t kVersionInfo = {
    .license = "GPLv3",
    .desc = "Cthulhu Compiler Collection GUI",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 1)
};

static jmp_buf gPanicJmp;
static ed::RuntimePanic gPanicInfo;

static void install_panic_handler()
{
    gPanicHandler = [](panic_t panic, const char *fmt, va_list args) {
        gPanicInfo.capture_trace(panic, fmt, args);
        longjmp(gPanicJmp, 1);
    };
}

struct AllocInfo
{
    size_t size;
    const char *name;
    const void *parent;
};

struct TraceArena final : public ed::IArena
{
    /// @brief the draw mode for the gui view of this allocator
    enum DrawType : int
    {
        /// @brief draw the allocations as a tree using parent data
        eDrawTree,

        /// @brief draw the allocations as a flat list
        eDrawFlat
    };

    TraceArena(const char *alloc_name, DrawType default_mode)
        : IArena(alloc_name)
        , draw_mode(default_mode)
    { }

    void *malloc(size_t size) override
    {
        malloc_calls += 1;

        void *ptr = ::malloc(size);

        update_alloc(ptr, size);

        return ptr;
    }

    void *realloc(void *ptr, size_t new_size, size_t) override
    {
        realloc_calls += 1;

        void *new_ptr = ::realloc(ptr, new_size);

        update_alloc(new_ptr, new_size);
        remove_alloc(ptr);

        return new_ptr;
    }

    void free(void *ptr, size_t) override
    {
        free_calls += 1;

        remove_alloc(ptr);

        ::free(ptr);
    }

    void set_name(const void *ptr, const char *new_name) override
    {
        update_name(ptr, new_name);
    }

    void set_parent(const void *ptr, const void *parent) override
    {
        update_parent(ptr, parent);
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

    // TODO: allocation parent tracking is broken

    void update_alloc(const void *ptr, size_t size)
    {
        peak_usage += size;
        allocs[ptr].size = size;
    }

    void update_parent(const void *ptr, const void *parent)
    {
        remove_parents(ptr);
        allocs[ptr].parent = parent;

        // try and figure out if this points into an existing allocation
        auto it = allocs.lower_bound(parent);
        if (it == allocs.end() || it == allocs.begin())
        {
            // this is a new parent
            tree[parent].push_back(ptr);
        }
        else
        {
            --it;
            // this may be a child
            const uint8_t *possible_parent = reinterpret_cast<const uint8_t*>(it->first);
            AllocInfo parent_info = it->second;
            if (possible_parent <= parent && possible_parent + parent_info.size >= parent)
            {
                // this is a child
                tree[possible_parent].push_back(ptr);
            }
            else
            {
                // this is a new parent
                tree[parent].push_back(ptr);
            }
        }
    }

    void update_name(const void *ptr, const char *new_name)
    {
        allocs[ptr].name = new_name;
    }

    void remove_alloc(const void *ptr)
    {
        remove_parents(ptr);

        auto iter = allocs.find(ptr);
        if (iter == allocs.end()) return;

        allocs.erase(iter);
    }

    void remove_parents(const void *ptr)
    {
        auto iter = allocs.find(ptr);
        if (iter == allocs.end()) return;

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

    /// @brief check if the given pointer was not allocated by this allocator
    ///
    /// @param ptr the pointer to check
    ///
    /// @return if the pointer was not allocated by this allocator
    bool is_external(const void *ptr) const
    {
        return allocs.find(ptr) == allocs.end();
    }

private:
    void draw_usage() const
    {
        ImGui::Text("malloc: %zu", malloc_calls);
        ImGui::Text("realloc: %zu", realloc_calls);
        ImGui::Text("free: %zu", free_calls);
        ImGui::Text("peak usage: %zu", peak_usage);
    }

    int draw_mode;

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

    void draw_extern_name(const void *ptr) const
    {
        if (auto it = allocs.find(ptr); it != allocs.end())
        {
            const char *id = it->second.name;
            if (id != nullptr)
            {
                ImGui::Text("%s (external)", id);
            }
            else
            {
                ImGui::TextDisabled("external");
            }
        }
        else
        {
            ImGui::TextDisabled("external");
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

        if (ImGui::BeginItemTooltip())
        {
            ImGui::Text("parent: %p", info.parent);
            ImGui::EndTooltip();
        }
    }

    void draw_tree_group(const void *ptr, const AllocInfo& info) const
    {
        bool is_open = ImGui::TreeNodeEx(ptr, kGroupNodeFlags, "%p", ptr);

        ImGui::TableNextColumn();
        ImGui::Text("%zu", info.size);

        ImGui::TableNextColumn();
        draw_name(info.name);

        if (ImGui::BeginItemTooltip())
        {
            ImGui::Text("parent: %p", info.parent);
            ImGui::EndTooltip();
        }

        if (is_open)
        {
            for (const void *child : tree.at(ptr))
            {
                draw_tree_node(child);
            }

            ImGui::TreePop();
        }
    }

    void draw_tree_node_info(const void *ptr, const AllocInfo& info) const
    {
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

    void draw_tree_node(const void *ptr) const
    {
        if (auto it = allocs.find(ptr); it != allocs.end())
        {
            draw_tree_node_info(ptr, it->second);
        }
    }

    static const ImGuiTableFlags kTableFlags
        = ImGuiTableFlags_BordersV
        | ImGuiTableFlags_BordersOuterH
        | ImGuiTableFlags_Resizable
        | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_NoHostExtendX
        | ImGuiTableFlags_NoBordersInBody;

    void draw_tree() const
    {
        ImGui::SeparatorText("Memory tree view");

        if (ImGui::BeginTable("Allocations", 3, kTableFlags))
        {
            ImGui::TableSetupColumn("Address");
            ImGui::TableSetupColumn("Size");
            ImGui::TableSetupColumn("Name");
            ImGui::TableHeadersRow();

            // first draw all nodes that dont have parents but have children
            for (auto& [root, children] : tree)
            {
                if (has_children(root) && !has_parent(root))
                {
                    draw_tree_node(root);
                }

                if (is_external(root) && !has_parent(root))
                {
                    AllocInfo info = {
                        .name = "extern"
                    };
                    draw_tree_node_info(root, info);
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

        if (ImGui::BeginTable("Allocations", 4, kTableFlags))
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
                    if (ImGui::BeginItemTooltip())
                    {
                        draw_extern_name(info.parent);
                        ImGui::EndTooltip();
                    }
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
    CompileRun(const char *id, mediator_t *instance)
        : name(_strdup(id))
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

    const char *name;
    bool show = true;

    TraceArena alloc{"default", TraceArena::eDrawTree};
    TraceArena gmp_alloc{"gmp", TraceArena::eDrawFlat};

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

    std::vector<char*> sources;

    char source_path[512] = { 0 };

    void draw_sources()
    {
        bool has_new_path = ImGui::InputText("Source", source_path, std::size(source_path), ImGuiInputTextFlags_EnterReturnsTrue);

        if (has_new_path)
        {
            sources.push_back(_strdup(source_path));
            memset(source_path, 0, std::size(source_path));
        }

        size_t remove_index = SIZE_MAX;

        for (size_t i = 0; i < sources.size(); i++)
        {
            ImGui::BulletText("%s", sources[i]);
            ImGui::SameLine();
            if (ImGui::Button("Remove"))
            {
                remove_index = i;
            }
        }

        if (remove_index != SIZE_MAX)
        {
            free(sources[remove_index]);
            sources.erase(sources.begin() + remove_index);
        }
    }

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

    bool execution_error = false;
    ed::RuntimePanic panic_info = {};

    bool do_compile()
    {
        lifetime_configure();
        reports = lifetime_get_reports(lifetime);

        for (size_t i = 0; i < sources.size(); i++)
        {
            if (!parse_file(sources[i]))
            {
                return false;
            }
        }

        return true;
    }

    void draw_compile()
    {
        bool can_compile = true;
        if (sources.size() == 0)
        {
            can_compile = false;
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "No sources");
        }

        ImGui::BeginDisabled(!can_compile || execution_error);
        if (ImGui::Button("Compile"))
        {
            do_compile();
        }
        ImGui::EndDisabled();

        if (compile_preinit_error)
        {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "%s", compile_preinit_error);
        }
    }

    void draw_window()
    {
        if (!show) return;

        if (ImGui::Begin(name, &show))
        {
            draw_compile();

            if (ImGui::CollapsingHeader("Sources", ImGuiTreeNodeFlags_DefaultOpen))
            {
                draw_sources();
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
