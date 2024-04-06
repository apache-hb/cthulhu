// SPDX-License-Identifier: GPL-3.0-only
#include "notify/notify.h"
#include "scan/node.h"
#include "std/vector.h"
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

#include "json/json.hpp"
#include "backtrace/backtrace.h"
#include "setup/setup2.h"

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

static TraceArena gGlobalArena{"Global Arena", TraceArena::eCollectStackTrace};
static TraceArena gGmpArena{"GMP Arena", TraceArena::eCollectStackTrace};
static TraceArena gGuiArena{"Dear ImGui Arena", TraceArena::eCollectNone};

static std::vector<TraceArenaWidget> gTraceWidgets = {
    { gGlobalArena, TraceArenaWidget::eDrawTree },
    { gGmpArena, TraceArenaWidget::eDrawFlat },
    { gGuiArena, TraceArenaWidget::eDrawFlat },
};

static void install_trace_arenas()
{
    init_global_arena(gGlobalArena.get_arena());
    init_gmp_arena(gGmpArena.get_arena());

    ImGui::SetAllocatorFunctions(
        /*alloc_func=*/ [](size_t size, void *user) {
            arena_t *arena = static_cast<arena_t*>(user);
            return ARENA_OPT_MALLOC(size, "ImGui::MemAlloc", nullptr, arena);
        },
        /*free_func=*/ [](void *ptr, void *user) {
            arena_t *arena = static_cast<arena_t*>(user);
            arena_opt_free(ptr, CT_ALLOC_SIZE_UNKNOWN, arena);
        },
        /*user_data=*/ gGuiArena.get_arena()
    );
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

struct Compiler
{
    std::string name;
    TraceArena& arena;
    loader_t *loader;
    broker_t *broker;
    support_t *support;

    Compiler(TraceArena& trace, std::string_view title)
        : name(title)
        , arena(trace)
        , loader(loader_new(arena.get_arena()))
        , broker(broker_new(&kFrontendGui, arena.get_arena()))
        , support(support_new(broker, loader, arena.get_arena()))
    {
        support_load_default_modules(support);

        gTraceWidgets.push_back({ arena, TraceArenaWidget::eDrawTree });
    }

    bool visible = false;

    void draw_window()
    {
        if (!visible)
            return;

        if (ImGui::Begin(name.c_str(), &visible))
        {

        }
        ImGui::End();
    }
};

static std::vector<Compiler> gCompilers;

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

static void draw_log_event(const event_t *event)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::TextUnformatted(event->diagnostic->id);

    ImGui::TableNextColumn();
    where_t where = node_get_location(&event->node);
    const scan_t *scan = node_get_scan(&event->node);
    ImGui::Text("%s:%" PRI_LINE ":%" PRI_COLUMN, scan_path(scan), where.first_line, where.first_column);

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(event->message);

    ImGui::TableNextColumn();
    if (event->notes)
        ImGui::Text("%zu notes", vector_len(event->notes));
    else
        ImGui::TextUnformatted("None");
}

static const ImGuiTableFlags kLogTableFlags
    = ImGuiTableFlags_BordersV
    | ImGuiTableFlags_BordersOuterH
    | ImGuiTableFlags_Resizable
    | ImGuiTableFlags_RowBg
    | ImGuiTableFlags_NoHostExtendX
    | ImGuiTableFlags_NoBordersInBody
    | ImGuiTableFlags_ScrollY;

static void draw_log_content(logger_t *logger)
{
    typevec_t *events = logger_get_events(logger);
    size_t len = typevec_len(events);
    ImGui::Text("Events: %zu", len);

    if (ImGui::BeginTable("Events", 4, kLogTableFlags))
    {
        ImGui::TableSetupColumn("Diagnostic");
        ImGui::TableSetupColumn("Where");
        ImGui::TableSetupColumn("Message");
        ImGui::TableSetupColumn("Related");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < len; i++)
            draw_log_event((event_t*)typevec_offset(events, i));

        ImGui::EndTable();
    }
}

static ImGui::FileBrowser gOpenFile { ImGuiFileBrowserFlags_MultipleSelection | ImGuiFileBrowserFlags_ConfirmOnEnter | ImGuiFileBrowserFlags_CloseOnEsc };

struct JsonFile
{
    std::string path;
    std::string basename;

    io_t *io;
    ctu::OsError error;
    std::string_view source;

    ctu::json::Json value;

    JsonFile(const fs::path& fp, ctu::json::JsonParser& parser, arena_t *arena)
        : path(fp.string())
        , basename(fp.filename().string())
        , io(io_file(path.c_str(), eOsAccessRead, arena))
        , error(io_error(io))
    {
        if (error.failed())
            return;

        const void *data = io_map(io, eOsProtectRead);
        size_t size = io_size(io);
        source = std::string_view(static_cast<const char*>(data), size);
        value = parser.parse(io);
    }

    void draw_source()
    {
        ImGui::TextUnformatted(source.data(), source.data() + source.size());
    }

    static const char *get_kind_name(json_kind_t kind)
    {
        switch (kind)
        {
        case eJsonNull: return "Null";
        case eJsonBoolean: return "Boolean";
        case eJsonInteger: return "Integer";
        case eJsonFloat: return "Float";
        case eJsonString: return "String";
        case eJsonArray: return "Array";
        case eJsonObject: return "Object";
        default: return "Unknown";
        }
    }

    static void draw_json_number(const ctu::json::Json& value)
    {
        mpz_t digit;
        value.as_integer(digit);
        char buffer[1024];
        mpz_get_str(buffer, 10, digit);
        ImGui::TextUnformatted(buffer);
        mpz_clear(digit);
    }

    static const ImGuiTreeNodeFlags kGroupNodeFlags
        = ImGuiTreeNodeFlags_SpanAllColumns
        | ImGuiTreeNodeFlags_AllowOverlap;

    static const ImGuiTreeNodeFlags kValueNodeFlags
        = kGroupNodeFlags
        | ImGuiTreeNodeFlags_Leaf
        | ImGuiTreeNodeFlags_Bullet
        | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    static void draw_json_array(const std::string& key, const ctu::json::Json& value)
    {
        bool is_open = ImGui::TreeNodeEx(key.c_str(), kGroupNodeFlags, "%s", key.c_str());

        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Array");

        ImGui::TableNextColumn();
        if (is_open)
        {
            for (size_t i = 0; i < value.length(); i++)
            {
                draw_json_item(std::format("[{}]", i).c_str(), value.get(i));
            }
            ImGui::TreePop();
        }
    }

    static void draw_json_object(const std::string& key, const ctu::json::Json& object)
    {
        bool is_open = ImGui::TreeNodeEx(key.c_str(), kGroupNodeFlags, "%s", key.c_str());

        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Object");

        ImGui::TableNextColumn();
        if (is_open)
        {
            // TODO: make iterators work
            auto iter = object.as_object().iter();
            while (iter.has_next())
            {
                auto [entry, value] = iter.next();
                draw_json_item(std::string{entry}, value);
            }
            ImGui::TreePop();
        }
    }

    static void draw_json_value(const std::string& key, const ctu::json::Json& value)
    {
        ImGui::TreeNodeEx(key.c_str(), kValueNodeFlags, "%s", key.c_str());

        ImGui::TableNextColumn();
        ImGui::TextUnformatted(get_kind_name(value.get_kind()));

        ImGui::TableNextColumn();
        switch (value.get_kind())
        {
        case eJsonNull:
            ImGui::TextUnformatted("null");
            break;

        case eJsonBoolean:
            ImGui::TextUnformatted(value.as_bool() ? "true" : "false");
            break;

        case eJsonInteger:
        case eJsonFloat:
            draw_json_number(value);
            break;

        case eJsonString: {
            std::string_view text = value.as_string();
            ImGui::TextUnformatted(text.data(), text.data() + text.size());
            break;
        }
        default:
            break;
        }
    }

    static void draw_json_item(const std::string& key, const ctu::json::Json& value)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        if (value.is_object())
        {
            draw_json_object(key, value);
        }
        else if (value.is_array())
        {
            draw_json_array(key, value);
        }
        else
        {
            draw_json_value(key, value);
        }
    }

    static const ImGuiTableFlags kTreeTableFlags
        = ImGuiTableFlags_BordersV
        | ImGuiTableFlags_BordersOuterH
        | ImGuiTableFlags_Resizable
        | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_NoHostExtendX
        | ImGuiTableFlags_NoBordersInBody
        | ImGuiTableFlags_ScrollY;

    void draw_json_document()
    {
        if (ImGui::BeginTable("Document", 3, kTreeTableFlags))
        {
            ImGui::TableSetupColumn("Key");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Value");
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            draw_json_item("$", value);

            ImGui::EndTable();
        }
    }

    void draw_content()
    {
        if (error.failed())
        {
            ImGui::Text("Failed to open file: %s", error.what());
            return;
        }

        if (value.is_valid())
        {
            if (ImGui::BeginChild("JsonSource", ImVec2(200, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX))
            {
                draw_source();
            }
            ImGui::EndChild();
            ImGui::SameLine();

            if (ImGui::BeginChild("JsonDocument", ImVec2(0, 0), ImGuiChildFlags_Border))
            {
                draw_json_document();
            }
            ImGui::EndChild();
        }
        else
        {
            ImGui::Text("Failed to parse JSON");
            draw_source();
        }
    }
};

struct JsonEditor
{
    TraceArena arena{"JSON", TraceArena::eCollectStackTrace};
    ctu::json::JsonParser parser{arena.get_arena()};
    bool visible = false;
    std::vector<JsonFile> documents;

    JsonEditor() { }

    void draw_window()
    {
        if (!visible)
            return;

        if (ImGui::Begin("JSON Editor", &visible))
        {
            if (ImGui::BeginTabBar("JsonTabs"))
            {
                if (ImGui::BeginTabItem("Logs"))
                {
                    draw_log_content(parser.get_logger());
                    ImGui::EndTabItem();
                }

                for (auto &doc : documents)
                {
                    if (ImGui::BeginTabItem(doc.basename.c_str()))
                    {
                        doc.draw_content();
                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    void add(const fs::path &path)
    {
        documents.push_back({ path, parser, arena.get_arena() });
    }
};

static JsonEditor gJsonEditor;
static TraceArenaWidget gJsonWidget{gJsonEditor.arena, TraceArenaWidget::eDrawTree};

static bool gImGuiDemoWindow = false;
static bool gImPlotDemoWindow = false;

static void DrawEditorWidgets()
{
    if (gImGuiDemoWindow)
    {
        ImGui::ShowDemoWindow(&gImGuiDemoWindow);
    }

    if (gImPlotDemoWindow)
    {
        ImPlot::ShowDemoWindow(&gImPlotDemoWindow);
    }

    for (auto &widget : gTraceWidgets)
    {
        widget.draw_window();
    }

    for (Compiler& instance : gCompilers)
    {
        instance.draw_window();
    }

    gOpenFile.Display();

    if (gOpenFile.HasSelected())
    {
        for (const fs::path &file : gOpenFile.GetMultiSelected())
        {
            gJsonEditor.add(file);
        }

        gOpenFile.ClearSelected();

        gJsonEditor.visible = true;
    }

    gJsonEditor.draw_window();
    gJsonWidget.draw_window();
}

static void DrawFileMenu()
{
    if (ImGui::MenuItem("New File"))
    {

    }

    ImGui::Separator();

    if (ImGui::MenuItem("Open File"))
    {
        gOpenFile.Open();
    }

    if (ImGui::MenuItem("Open Folder"))
    {

    }

    ImGui::Separator();
    if (ImGui::MenuItem("Exit"))
    {
        draw::close();
    }
}

static void DrawEditMenu()
{
    ImGui::MenuItem("Undo", "Ctrl+Z");
    ImGui::MenuItem("Redo", "Ctrl+Y");
    ImGui::Separator();
    ImGui::MenuItem("Cut", "Ctrl+X");
    ImGui::MenuItem("Copy", "Ctrl+C");
    ImGui::MenuItem("Paste", "Ctrl+V");
    ImGui::Separator();
    ImGui::MenuItem("Find", "Ctrl+F");
    ImGui::MenuItem("Replace", "Ctrl+H");
}

enum EditorStyle
{
    eStyleDark,
    eStyleLight,
    eStyleClassic,
};

static EditorStyle gStyle = eStyleDark;

static void DrawViewMenu()
{
    if (ImGui::BeginMenu("Memory Statistics", CTU_TRACE_MEMORY))
    {
        for (auto &widget : gTraceWidgets)
        {
            ImGui::MenuItem(widget.get_title(), nullptr, &widget.visible);
        }

        ImGui::MenuItem("JSON Editor", nullptr, &gJsonWidget.visible, !gJsonEditor.documents.empty());
        if (gJsonEditor.documents.empty())
        {
            ImGui::SetItemTooltip("Open a JSON file to enable this widget");
        }

        ImGui::EndMenu();
    }

    if constexpr (!CTU_TRACE_MEMORY)
    {
        ImGui::SetItemTooltip("reconfigure with -DCTU_TRACE_MEMORY=enabled");
    }

    ImGui::Separator();
    ImGui::MenuItem("ImGui Demo", nullptr, &gImGuiDemoWindow);
    ImGui::MenuItem("ImPlot Demo", nullptr, &gImPlotDemoWindow);

    ImGui::SeparatorText("Theme");
    if (ImGui::MenuItem("Dark", nullptr, gStyle == eStyleDark))
    {
        ImGui::StyleColorsDark();
        ImPlot::StyleColorsDark();
        gStyle = eStyleDark;
    }

    if (ImGui::MenuItem("Light", nullptr, gStyle == eStyleLight))
    {
        ImGui::StyleColorsLight();
        ImPlot::StyleColorsLight();
        gStyle = eStyleLight;
    }

    if (ImGui::MenuItem("Classic", nullptr, gStyle == eStyleClassic))
    {
        ImGui::StyleColorsClassic();
        ImPlot::StyleColorsClassic();
        gStyle = eStyleClassic;
    }
}

static void DrawHelpMenu()
{
    ImGui::MenuItem("About");
    ImGui::MenuItem("Documentation");
    ImGui::Separator();
    ImGui::MenuItem("Search menus");
    ImGui::Separator();
    ImGui::MenuItem("Report Issue");
    ImGui::MenuItem("Check for Updates");
}

static void DrawMainMenuBar(const char *title)
{
    if (ImGui::BeginMainMenuBar())
    {
        ImGui::TextUnformatted(title);
        ImGui::Separator();

        if (ImGui::BeginMenu("File"))
        {
            DrawFileMenu();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            DrawEditMenu();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            DrawViewMenu();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            DrawHelpMenu();
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

int main(int argc, const char **argv)
{
    CT_UNUSED(argc);
    CT_UNUSED(argv);

    setup_default(nullptr);
    install_trace_arenas();

    TraceArena trace{"Compiler", TraceArena::eCollectStackTrace};
    gCompilers.emplace_back(Compiler(trace, "Compiler"));

    gOpenFile.SetTitle("Open File");
    gOpenFile.SetTypeFilters({ ".json" });

    draw::config_t config = {
        .title = L"Editor",
        .hwaccel = false,
    };

    if (!draw::create(config))
    {
        return 1;
    }

    while (draw::begin_frame())
    {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        DrawMainMenuBar("Cthulhu");
        DrawEditorWidgets();
        draw::end_frame();
    }

    draw::destroy();
}
