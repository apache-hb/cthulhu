// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "editor/arena.hpp"
#include "editor/memory.hpp"

#include <unordered_set>
#include <vector>
#include <stacktrace>

struct AllocInfo
{
    size_t id; ///< the id of the allocation
    Memory size; ///< the size of the allocation
    std::chrono::steady_clock::time_point timestamp; ///< the time of the allocation
    size_t trace; ///< hash of the stack trace

    std::string name; ///< the name of the allocation
    const void *parent; ///< the parent of the allocation
};

class TraceArena final : public IArena
{
public:
    using AllocMap = std::map<const void*, AllocInfo>;
    using AllocTree = std::map<const void*, std::vector<const void*>>;
    using AllocMapIter = AllocMap::iterator;

    // IArena
    void *malloc(size_t size) override;
    void *realloc(void *ptr, size_t new_size, size_t size) override;
    void free(void *ptr, size_t size) override;

    void rename(const void *ptr, const char *new_name) override;
    void reparent(const void *ptr, const void *parent) override;

    // number of calls to each function
    size_t malloc_calls = 0;
    size_t realloc_calls = 0;
    size_t free_calls = 0;

    // usage stats in bytes
    Memory peak_memory_usage = 0;
    Memory live_memory_usage = 0;

    // all currently live allocations
    std::unordered_set<void *> live_allocs;

    // TODO: this is a little silly, but its an easy way to avoid having to
    // deal with iterator invalidation
    std::unordered_map<size_t, std::stacktrace> stacktraces;

    size_t add_stacktrace(const std::stacktrace& trace);

    int collect;

    // all allocations
    AllocMap allocs;

    // tree of allocations
    AllocTree tree;

    // allocation tracking
    void create_alloc(void *ptr, size_t size);
    void delete_alloc(void *ptr);

    void update_parent(const void *ptr, const void *parent);
    void update_name(const void *ptr, const char *new_name);
    void remove_parents(const void *ptr);

    // internal queries
    bool has_children(const void *ptr) const;
    bool has_parent(const void *ptr) const;
    bool is_external(const void *ptr) const;

public:
    enum Collect
    {
        eCollectNone = 0,

        eCollectStackTrace = 1 << 0,
        eCollectTimeStamps = 1 << 1,
    };

    TraceArena(const char *id, Collect collect);

    // TraceArena
    void reset();
};

class TraceArenaWidget
{
    TraceArena& arena;
    int mode;

    // tree drawing
    void draw_backtrace(const std::stacktrace& trace) const;
    void draw_name(const AllocInfo& info) const;
    void draw_extern_name(const void *ptr) const;
    void draw_tree_child(const void *ptr, const AllocInfo& info) const;
    void draw_tree_group(const void *ptr, const AllocInfo& info) const;
    void draw_tree_node_info(const void *ptr, const AllocInfo& info) const;
    void draw_tree_node(const void *ptr) const;
    void draw_tree() const;

    // flat output
    void draw_flat() const;

public:
    /// @brief the draw mode for the gui view of this allocator
    enum draw_mode_t : int
    {
        /// @brief draw the allocations as a tree using parent data
        eDrawTree,

        /// @brief draw the allocations as a flat list
        eDrawFlat,
    };

    TraceArenaWidget(TraceArena& arena, int mode)
        : arena(arena)
        , mode(mode)
    { }

    bool visible = false;

    const char *get_title() const { return arena.get_name(); }

    void draw();
    bool draw_window();
};
