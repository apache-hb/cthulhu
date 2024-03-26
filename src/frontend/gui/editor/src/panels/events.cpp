#include "editor/utils.hpp"
#include "stdafx.hpp"

#include "editor/panels/events.hpp"

#include "os/os.h"

#if CTU_EVENTS

struct file_event_t
{
    enum { eOpen, eClose, eRead, eWrite } type;
    size_t size;
};

static const char *file_event_name(int type)
{
    switch (type)
    {
    case file_event_t::eOpen: return "Open";
    case file_event_t::eClose: return "Close";
    case file_event_t::eRead: return "Read";
    case file_event_t::eWrite: return "Write";
    default: return "Unknown";
    }
}

struct library_event_t
{
    enum { eOpen, eClose, eSymbol } type;

    std::string name;
    const os_symbol_t *sym;
};

static const char *library_event_name(int type)
{
    switch (type)
    {
    case library_event_t::eOpen: return "Open";
    case library_event_t::eClose: return "Close";
    case library_event_t::eSymbol: return "Symbol";
    default: return "Unknown";
    }
}

template<typename T>
struct tracked_t
{
    using object_t = T;
    const T *object;

    const T *get_object() const
    {
        return object;
    }
};

struct file_info_t : tracked_t<os_file_t>
{
    std::string path;
    std::vector<file_event_t> events;
    std::unordered_set<const os_mapping_t*> mappings;
};

struct library_info_t : tracked_t<os_library_t>
{
    std::string path;
    std::vector<library_event_t> events;
};

struct mapping_info_t : tracked_t<os_mapping_t>
{
    size_t file;
    size_t size;
    bool active = true;
};

template<typename T>
concept Tracked = requires(const T t)
{
    typename T::object_t;
    { t.get_object() } -> std::same_as<const typename T::object_t*>;
};

template<Tracked T>
struct tracker_t
{
    using object_t = typename T::object_t;

    std::vector<T> data;
    std::unordered_map<const object_t*, size_t> lookup;

    T &get(const object_t *object)
    {
        return data[lookup[object]];
    }

    T& add(const T &value)
    {
        lookup[value.get_object()] = data.size();
        return data.emplace_back(value);
    }
};

static size_t gActiveMemory = SIZE_MAX;
static MemoryEditor gMemoryEditor;

static tracker_t<file_info_t> gFiles;
static tracker_t<library_info_t> gLibraries;
static tracker_t<mapping_info_t> gMappings;

static void on_file_open(const os_file_t *file)
{
    auto& it = gFiles.add({ {file}, os_file_name(file) });
    it.events.push_back({ file_event_t::eOpen });
}

static void on_file_close(const os_file_t *file)
{
    gFiles.get(file).events.push_back({ file_event_t::eClose });
}

static void on_file_read(const os_file_t *file, size_t size)
{
    gFiles.get(file).events.push_back({ file_event_t::eRead, size });
}

static void on_file_write(const os_file_t *file, size_t size)
{
    gFiles.get(file).events.push_back({ file_event_t::eWrite, size });
}

static void on_library_open(const os_library_t *lib)
{
    gLibraries.add({ {lib}, os_library_name(lib) });
}

static void on_library_close(const os_library_t *lib)
{
    gLibraries.get(lib).events.push_back({ library_event_t::eClose });
}

static void on_library_symbol(const os_library_t *lib, const char *name, const os_symbol_t *sym)
{
    gLibraries.get(lib).events.push_back({ library_event_t::eSymbol, name, sym });
}

static void on_mapping_open(const os_file_t *file, const os_mapping_t *map)
{
    auto& fd = gFiles.get(file);
    gMappings.add({ {map}, gFiles.lookup[file], os_mapping_size(map) });
    fd.mappings.insert(map);
}

static void on_mapping_close(const os_mapping_t *map)
{
    gMappings.get(map).active = false;
}

static const ImGuiTreeNodeFlags kGroupNodeFlags
    = ImGuiTreeNodeFlags_SpanAllColumns
    | ImGuiTreeNodeFlags_AllowOverlap;

static const ImGuiTreeNodeFlags kValueNodeFlags
    = kGroupNodeFlags
    | ImGuiTreeNodeFlags_Leaf
    | ImGuiTreeNodeFlags_Bullet
    | ImGuiTreeNodeFlags_NoTreePushOnOpen;

static const ImGuiTableFlags kTraceTableFlags
    = ImGuiTableFlags_BordersV
    | ImGuiTableFlags_BordersOuterH
    | ImGuiTableFlags_Resizable
    | ImGuiTableFlags_RowBg
    | ImGuiTableFlags_NoHostExtendX
    | ImGuiTableFlags_NoBordersInBody;

static void draw_files()
{
    if (ImGui::BeginTable("Files", 3, kTraceTableFlags))
    {
        ImGui::TableSetupColumn("Path");
        ImGui::TableSetupColumn("Events");
        ImGui::TableSetupColumn("Mappings");
        ImGui::TableHeadersRow();

        for (const auto &file : gFiles.data)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            bool is_open = ImGui::TreeNodeEx(file.path.c_str(), kGroupNodeFlags);

            ImGui::TableNextColumn();
            ImGui::Text("%zu", file.events.size());

            ImGui::TableNextColumn();
            ImGui::Text("%zu", file.mappings.size());

            if (is_open)
            {
                for (const auto& event : file.events)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::TreeNodeEx(&event, kValueNodeFlags, "%s", file_event_name(event.type));

                    ImGui::TableNextColumn();
                    if (event.type == file_event_t::eRead || event.type == file_event_t::eWrite)
                    {
                        ImGui::Text("%zu", event.size);
                    }
                    else
                    {
                        ImGui::TextUnformatted("");
                    }

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("");
                }

                ImGui::TreePop();
            }
        }

        ImGui::EndTable();
    }
}

static void draw_libraries()
{
    if (ImGui::BeginTable("Libraries", 2, kTraceTableFlags))
    {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Events");
        ImGui::TableHeadersRow();

        for (const auto &lib : gLibraries.data)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            bool is_open = ImGui::TreeNodeEx(lib.path.c_str(), kGroupNodeFlags);

            ImGui::TableNextColumn();
            ImGui::Text("%zu", lib.events.size());

            if (is_open)
            {
                for (const auto& event : lib.events)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::TreeNodeEx(&event, kValueNodeFlags, "%s", library_event_name(event.type));

                    ImGui::TableNextColumn();
                    if (event.type == library_event_t::eSymbol)
                    {
                        ImGui::Text("%s", event.name.c_str());
                        ImGui::SetItemTooltip("%p", (void*)event.sym);
                    }
                    else
                    {
                        ImGui::TextUnformatted("");
                    }
                }

                ImGui::TreePop();
            }
        }

        ImGui::EndTable();
    }
}

static void draw_mappings()
{
    if (ImGui::BeginTable("Mappings", 3, kTraceTableFlags))
    {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Details");
        ImGui::TableSetupColumn("Memory");
        ImGui::TableHeadersRow();

        for (const auto &mapping : gMappings.data)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            const auto& file = gFiles.data[mapping.file];
            ImGui::Text("%s", file.path.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%zu bytes (%s)", mapping.size, mapping.active ? "active" : "inactive");

            ImGui::TableNextColumn();
            if (mapping.active)
            {
                auto label = ed::strfmt<256>("%p", os_mapping_data((os_mapping_t*)mapping.get_object()));
                if (ImGui::Button(label))
                {
                    gActiveMemory = gMappings.lookup[mapping.get_object()];
                }
            }
            else
            {
                ImGui::TextDisabled("Inactive");
            }
        }

        ImGui::EndTable();
    }
}

class EventsPanel final : public ed::IEditorPanel
{
    void draw_events()
    {
        ImGui::Text("%zu files | %zu libraries | %zu mappings", gFiles.data.size(), gLibraries.data.size(), gMappings.data.size());

        ImGui::Separator();

        if (ImGui::BeginTabBar("Trackers"))
        {
            if (ImGui::BeginTabItem("Files"))
            {
                draw_files();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Libraries"))
            {
                draw_libraries();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Mappings"))
            {
                draw_mappings();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

public:
    EventsPanel() : IEditorPanel("Events") { }

    void draw_content() override
    {
        draw_events();
    }

    void update() override
    {
        if (gActiveMemory == SIZE_MAX)
            return;

        const auto& mapping = gMappings.data[gActiveMemory];
        if (!mapping.active)
        {
            gActiveMemory = SIZE_MAX;
            return;
        }

        bool open = true;
        if (ImGui::Begin("Memory Editor", &open))
        {
            gMemoryEditor.DrawContents(os_mapping_data((os_mapping_t*)mapping.get_object()), mapping.size);
        }
        ImGui::End();

        if (!open)
        {
            gActiveMemory = SIZE_MAX;
        }
    }
};

void ed::init_events()
{
    os_events_t events = {
        .on_file_open = on_file_open,
        .on_file_close = on_file_close,
        .on_file_read = on_file_read,
        .on_file_write = on_file_write,

        .on_library_open = on_library_open,
        .on_library_close = on_library_close,
        .on_library_symbol = on_library_symbol,

        .on_mapping_open = on_mapping_open,
        .on_mapping_close = on_mapping_close,
    };

    gOsEvents = events;
}

#else

void ed::init_events()
{

}

class EventsPanel final : public ed::IEditorPanel
{
public:
    EventsPanel()
        : IEditorPanel("Events")
    {
        disable("Events are disabled, reconfigure with -Devents=enabled");
    }

    void draw_content() override { }
};

#endif

ed::IEditorPanel *ed::create_events_panel()
{
    return new EventsPanel();
}
