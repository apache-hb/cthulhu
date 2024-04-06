// SPDX-License-Identifier: GPL-3.0-only
#include "stdafx.hpp"

#include "editor/panels/arena.hpp"
#include "editor/panels/panel.hpp"

struct FrameInfo
{
    std::string name;
    std::string path;
    std::uint_least32_t line = 0;

    void update(std::stacktrace_entry entry)
    {
        if (name.empty())
            name = entry.description();

        if (path.empty())
            path = entry.source_file();

        if (line == 0)
            line = entry.source_line();
    }

    const char *get_name() const
    {
        return (name.empty()) ? "???" : name.c_str();
    }

    const char *get_path() const
    {
        return (path.empty()) ? "<unknown>" : path.c_str();
    }

    std::uint_least32_t get_line() const
    {
        return line;
    }
};

static std::unordered_map<std::stacktrace_entry, FrameInfo> gTraceInfo;
static size_t gCounter = 0;

const FrameInfo& get_frame_info(std::stacktrace_entry entry)
{
    FrameInfo& info = gTraceInfo[entry];
    info.update(entry);
    return info;
}

TraceArena::TraceArena(const char *id, Collect collect)
    : IArena(id)
    , collect(collect)
{ }

void *TraceArena::malloc(size_t size)
{
    malloc_calls += 1;

    void *ptr = ::malloc(size);

    create_alloc(ptr, size);

    return ptr;
}

void *TraceArena::realloc(void *ptr, size_t new_size, size_t)
{
    realloc_calls += 1;

    Memory old_size = allocs[ptr].size;
    if (old_size < new_size)
        peak_memory_usage += (Memory::bytes(new_size) - old_size);

    void *new_ptr = ::realloc(ptr, new_size);

    create_alloc(new_ptr, new_size);
    delete_alloc(ptr);

    return new_ptr;
}

void TraceArena::free(void *ptr, size_t)
{
    free_calls += 1;

    delete_alloc(ptr);

    ::free(ptr);
}

void TraceArena::rename(const void *ptr, const char *new_name)
{
    update_name(ptr, new_name);
}

void TraceArena::reparent(const void *ptr, const void *new_parent)
{
    update_parent(ptr, new_parent);
}

void TraceArena::reset()
{
    malloc_calls = 0;
    realloc_calls = 0;
    free_calls = 0;
    peak_memory_usage = 0;
    live_memory_usage = 0;

    live_allocs.clear();
    tree.clear();
    allocs.clear();
}

size_t TraceArena::add_stacktrace(const std::stacktrace& trace)
{
    // TODO: lets hope these dont collide
    size_t hash = std::hash<std::stacktrace>{}(trace);
    stacktraces[hash] = trace;
    return hash;
}

void TraceArena::create_alloc(void *ptr, size_t size)
{
    peak_memory_usage += size;
    live_memory_usage += size;
    allocs[ptr].id = gCounter++;
    allocs[ptr].size = size;

    if (collect & eCollectTimeStamps)
        allocs[ptr].timestamp = std::chrono::high_resolution_clock::now();

    if (collect & eCollectStackTrace)
        allocs[ptr].trace = add_stacktrace(std::stacktrace::current());

    live_allocs.insert(ptr);
}

void TraceArena::delete_alloc(void *ptr)
{
    remove_parents(ptr);

    auto iter = allocs.find(ptr);
    if (iter == allocs.end()) return;

    live_memory_usage -= iter->second.size;

    live_allocs.erase(ptr);
}

void TraceArena::update_parent(const void *ptr, const void *new_parent)
{
    remove_parents(ptr);
    allocs[ptr].parent = new_parent;

    // try and figure out if this points into an existing allocation
    auto it = allocs.upper_bound(new_parent);
    if (it == allocs.end() || it == allocs.begin())
    {
        // this is a new parent
        tree[new_parent].push_back(ptr);
    }
    else
    {
        // find the best fit, this is the smallest parent that contains the new parent
        const uint8_t *best = nullptr;
        size_t smallest = SIZE_MAX;
        while (it != allocs.begin())
        {
            --it;

            const uint8_t *value = reinterpret_cast<const uint8_t*>(it->first);
            const AllocInfo& parent_info = it->second;

            if (value <= new_parent && value + parent_info.size.as_bytes() >= new_parent)
            {
                size_t size = parent_info.size.as_bytes();
                if (size < smallest)
                {
                    best = value;
                    smallest = size;
                }
            }
        }

        if (best == nullptr)
        {
            // this is a new parent
            tree[new_parent].push_back(ptr);
        }
        else
        {
            // this is a child
            tree[best].push_back(ptr);
        }
    }
}

void TraceArena::update_name(const void *ptr, const char *new_name)
{
    allocs[ptr].name = new_name;
}

void TraceArena::remove_parents(const void *ptr)
{
    auto iter = allocs.find(ptr);
    if (iter == allocs.end()) return;

    for (auto& [_, children] : tree)
    {
        auto it = std::find(children.begin(), children.end(), iter->first);
        if (it != children.end())
        {
            children.erase(it);
            break;
        }
    }
}

bool TraceArena::has_children(const void *ptr) const
{
    if (auto it = tree.find(ptr); it != tree.end())
    {
        return it->second.size() > 0;
    }

    return false;
}

bool TraceArena::has_parent(const void *ptr) const
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
bool TraceArena::is_external(const void *ptr) const
{
    return allocs.find(ptr) == allocs.end();
}

static const ImGuiTableFlags kTraceTableFlags
    = ImGuiTableFlags_BordersV
    | ImGuiTableFlags_BordersOuterH
    | ImGuiTableFlags_Resizable
    | ImGuiTableFlags_RowBg
    | ImGuiTableFlags_NoHostExtendX
    | ImGuiTableFlags_NoBordersInBody;

void TraceArenaWidget::draw_backtrace(const std::stacktrace& trace) const
{
    if (ImGui::BeginTable("Trace", 3, kTraceTableFlags))
    {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Symbol");
        ImGui::TableSetupColumn("File");
        ImGui::TableHeadersRow();

        for (auto frame : trace)
        {
            auto info = get_frame_info(frame);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("0x%p", reinterpret_cast<void*>(frame.native_handle()));
            ImGui::TableNextColumn();
            ImGui::Text("%s", info.get_name());
            ImGui::TableNextColumn();
            ImGui::Text("%s:%zu", info.get_path(), info.get_line());
        }

        ImGui::EndTable();
    }
}

void TraceArenaWidget::draw_name(const AllocInfo& alloc) const
{
    ed::ScopeID scope((int)alloc.id);
    if (!alloc.name.empty())
    {
        ImGui::Text("%s", alloc.name.c_str());
    }
    else
    {
        ImGui::TextDisabled("---");
    }

    if (ImGui::BeginPopupContextItem("TracePopup"))
    {
        const auto& trace = arena.stacktraces[alloc.trace];
        if (!trace.empty())
        {
            draw_backtrace(trace);
        }
        else
        {
            ImGui::TextDisabled("Stacktrace not available");
        }
        ImGui::EndPopup();
    }
}

void TraceArenaWidget::draw_extern_name(const void *ptr) const
{
    if (auto it = arena.allocs.find(ptr); it != arena.allocs.end())
    {
        const auto& alloc = it->second;
        if (!alloc.name.empty())
        {
            ImGui::Text("%s (external)", alloc.name.c_str());
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

void TraceArenaWidget::draw_tree_child(const void *ptr, const AllocInfo& alloc) const
{
    ImGui::TreeNodeEx(ptr, kValueNodeFlags, "%p", ptr);

    ImGui::TableNextColumn();
    ImGui::Text("%s", alloc.size.to_string().c_str());

    ImGui::TableNextColumn();
    draw_name(alloc);

    if (ImGui::BeginItemTooltip())
    {
        ImGui::Text("parent: %p", alloc.parent);
        ImGui::EndTooltip();
    }
}

void TraceArenaWidget::draw_tree_group(const void *ptr, const AllocInfo& alloc) const
{
    bool is_open = ImGui::TreeNodeEx(ptr, kGroupNodeFlags, "%p", ptr);

    ImGui::TableNextColumn();
    ImGui::Text("%s", alloc.size.to_string().c_str());

    ImGui::TableNextColumn();
    draw_name(alloc);

    if (ImGui::BeginItemTooltip())
    {
        ImGui::Text("parent: %p", alloc.parent);
        ImGui::EndTooltip();
    }

    if (is_open)
    {
        for (const void *child : arena.tree.at(ptr))
        {
            draw_tree_node(child);
        }

        ImGui::TreePop();
    }
}

void TraceArenaWidget::draw_tree_node_info(const void *ptr, const AllocInfo& alloc) const
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    if (arena.has_children(ptr))
    {
        draw_tree_group(ptr, alloc);
    }
    else
    {
        draw_tree_child(ptr, alloc);
    }
}

void TraceArenaWidget::draw_tree_node(const void *ptr) const
{
    if (auto it = arena.allocs.find(ptr); it != arena.allocs.end())
    {
        draw_tree_node_info(ptr, it->second);
    }
}

static const ImGuiTableFlags kMemoryTreeTableFlags
    = ImGuiTableFlags_BordersV
    | ImGuiTableFlags_BordersOuterH
    | ImGuiTableFlags_Resizable
    | ImGuiTableFlags_RowBg
    | ImGuiTableFlags_NoHostExtendX
    | ImGuiTableFlags_NoBordersInBody
    | ImGuiTableFlags_ScrollY;

void TraceArenaWidget::draw_tree() const
{
    ImGui::SeparatorText("Memory tree view");

    if (ImGui::BeginTable("Allocations", 3, kMemoryTreeTableFlags))
    {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Size");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        // first draw all nodes that dont have parents but have children
        for (auto& [root, children] : arena.tree)
        {
            if (arena.has_children(root) && !arena.has_parent(root))
            {
                draw_tree_node(root);
            }

            if (arena.is_external(root) && !arena.has_parent(root))
            {
                AllocInfo it = { .name = "extern" };
                draw_tree_node_info(root, it);
            }
        }

        // draw all nodes that don't have parents and don't have children
        for (auto& [ptr, alloc] : arena.allocs)
        {
            if (alloc.parent != nullptr || arena.has_children(ptr)) continue;

            draw_tree_node(ptr);
        }

        ImGui::EndTable();
    }
}

static const ImGuiTableFlags kMemoryFlatTableFlags
    = ImGuiTableFlags_BordersV
    | ImGuiTableFlags_BordersOuterH
    | ImGuiTableFlags_Resizable
    | ImGuiTableFlags_RowBg
    | ImGuiTableFlags_NoHostExtendX
    | ImGuiTableFlags_NoBordersInBody
    | ImGuiTableFlags_ScrollY;

void TraceArenaWidget::draw_flat() const
{
    ImGui::SeparatorText("Memory view");

    if (ImGui::BeginTable("Allocations", 4, kMemoryFlatTableFlags))
    {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Size");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Parent");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        for (const auto& [ptr, alloc] : arena.allocs)
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%p", ptr);

            ImGui::TableNextColumn();
            ImGui::Text("%s", alloc.size.to_string().c_str());

            ImGui::TableNextColumn();
            draw_name(alloc);

            ImGui::TableNextColumn();
            if (alloc.parent)
            {
                ImGui::Text("%p", alloc.parent);
                if (ImGui::BeginItemTooltip())
                {
                    draw_extern_name(alloc.parent);
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

void TraceArenaWidget::draw()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            if (ImGui::MenuItem("Collect Stack Trace", nullptr, arena.collect & TraceArena::eCollectStackTrace))
            {
                arena.collect ^= TraceArena::eCollectStackTrace;
            }

            if (ImGui::MenuItem("Collect Time Stamps", nullptr, arena.collect & TraceArena::eCollectTimeStamps))
            {
                arena.collect ^= TraceArena::eCollectTimeStamps;
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::TextWrapped("Memory usage: (%zu mallocs, %zu reallocs, %zu frees, %s peak, %s live)", arena.malloc_calls, arena.realloc_calls, arena.free_calls, arena.peak_memory_usage.to_string().c_str(), arena.live_memory_usage.to_string().c_str());

    if (ImGui::Button("Reset Stats"))
    {
        arena.reset();
    }
    ImGui::SameLine();
    ImGui::Text("Visualize as:");
    ImGui::SameLine();

    // body
    ImGui::RadioButton("Tree", &mode, eDrawTree);
    ImGui::SameLine();
    ImGui::RadioButton("Flat", &mode, eDrawFlat);

    if (mode == eDrawTree)
    {
        draw_tree();
    }
    else
    {
        draw_flat();
    }
}

bool TraceArenaWidget::draw_window()
{
    if (!visible) return false;

    bool result = ImGui::Begin(get_title(), &visible, ImGuiWindowFlags_MenuBar);
    if (result)
    {
        draw();
    }
    ImGui::End();

    return result;
}
