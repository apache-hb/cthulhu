#include "editor/trace.hpp"

#include "imgui/imgui.h"

using namespace ed;

TraceArena::TraceArena(const char *alloc_name, DrawType default_mode)
    : IArena(alloc_name)
    , draw_mode(default_mode)
{ }

void *TraceArena::malloc(size_t size)
{
    malloc_calls += 1;

    void *ptr = ::malloc(size);

    update_alloc(ptr, size);

    return ptr;
}

void *TraceArena::realloc(void *ptr, size_t new_size, size_t)
{
    realloc_calls += 1;

    void *new_ptr = ::realloc(ptr, new_size);

    update_alloc(new_ptr, new_size);
    remove_alloc(ptr);

    return new_ptr;
}

void TraceArena::free(void *ptr, size_t)
{
    free_calls += 1;

    remove_alloc(ptr);

    ::free(ptr);
}

void TraceArena::set_name(const void *ptr, const char *new_name)
{
    update_name(ptr, new_name);
}

void TraceArena::set_parent(const void *ptr, const void *parent)
{
    update_parent(ptr, parent);
}

void TraceArena::draw_info()
{
    // usage
    ImGui::Text("malloc: %zu", malloc_calls);
    ImGui::Text("realloc: %zu", realloc_calls);
    ImGui::Text("free: %zu", free_calls);
    ImGui::Text("peak usage: %zu", peak_usage);

    // body
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

void TraceArena::update_alloc(const void *ptr, size_t size)
{
    peak_usage += size;
    allocs[ptr].size = size;
}

void TraceArena::update_parent(const void *ptr, const void *parent)
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

void TraceArena::update_name(const void *ptr, const char *new_name)
{
    allocs[ptr].name = new_name;
}

void TraceArena::remove_alloc(const void *ptr)
{
    remove_parents(ptr);

    auto iter = allocs.find(ptr);
    if (iter == allocs.end()) return;

    allocs.erase(iter);
}

void TraceArena::remove_parents(const void *ptr)
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

void TraceArena::draw_extern_name(const void *ptr) const
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

void TraceArena::draw_tree_child(const void *ptr, const AllocInfo& info) const
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

void TraceArena::draw_tree_group(const void *ptr, const AllocInfo& info) const
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

void TraceArena::draw_tree_node_info(const void *ptr, const AllocInfo& info) const
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

void TraceArena::draw_tree_node(const void *ptr) const
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

void TraceArena::draw_tree() const
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

void TraceArena::draw_flat() const
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
