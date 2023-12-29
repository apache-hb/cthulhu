#pragma once

#include "editor/arena.hpp"

#include <map>
#include <unordered_set>
#include <vector>

namespace ed
{
    struct alloc_info_t
    {
        size_t size;
        const char *name;
        const void *parent;
    };

    class TraceArena final : public IArena
    {
    public:
        /// @brief the draw mode for the gui view of this allocator
        enum draw_mode_t : int
        {
            /// @brief draw the allocations as a tree using parent data
            eDrawTree,

            /// @brief draw the allocations as a flat list
            eDrawFlat
        };

        TraceArena(const char *alloc_name, draw_mode_t default_mode);

        // IArena
        void *malloc(size_t size) override;
        void *realloc(void *ptr, size_t new_size, size_t) override;
        void free(void *ptr, size_t) override;

        void set_name(const void *ptr, const char *new_name) override;
        void set_parent(const void *ptr, const void *parent) override;

        // TraceArena
        void draw_info();
        void reset();

    private:
        // DrawType
        // stored as an int for direct use with imgui
        int draw_mode;

        // number of calls to each function
        size_t malloc_calls = 0;
        size_t realloc_calls = 0;
        size_t free_calls = 0;

        // peak memory usage
        size_t peak_usage = 0;

        using AllocMap = std::map<const void*, alloc_info_t>;
        using AllocTree = std::map<const void*, std::vector<const void*>>;
        using AllocMapIter = AllocMap::iterator;

        std::unordered_set<void *> live_allocs;

        // all live allocations
        AllocMap allocs = {};

        // tree of allocations
        AllocTree tree = {};

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

        // tree drawing
        void draw_extern_name(const void *ptr) const;
        void draw_tree_child(const void *ptr, const alloc_info_t& info) const;
        void draw_tree_group(const void *ptr, const alloc_info_t& info) const;
        void draw_tree_node_info(const void *ptr, const alloc_info_t& info) const;
        void draw_tree_node(const void *ptr) const;
        void draw_tree() const;

        // flat output
        void draw_flat() const;
    };
}
