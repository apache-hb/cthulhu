#pragma once

#include "editor/panels/panel.hpp"

#include "editor/arena.hpp"

#include "backtrace/backtrace.h"

#include <unordered_map>
#include <unordered_set>
#include <array>
#include <map>
#include <vector>

namespace ed
{
    template<typename T, size_t N>
    struct small_vector_t
    {
        std::array<T, N> data;
        size_t size = 0;

        void add(const T& value)
        {
            data[size++] = value;
        }

        bool empty() const
        {
            return size == 0;
        }

        bool full() const
        {
            return size == N;
        }
    };

    class TraceArena final : public IArena, public IEditorPanel
    {
        using backtrace_t = small_vector_t<bt_address_t, 64>;
        static backtrace_t get_backtrace();

        struct alloc_info_t
        {
            size_t event; // the order in which the allocation was made
            size_t size; // the size of the allocation
            size_t trace; // index into traces

            const char *name; // the name of the allocation
            const void *parent; // the parent of the allocation
        };

        // use maps rather than unordered_map for deterministic output
        using AllocMap = std::map<const void*, alloc_info_t>;
        using AllocTree = std::map<const void*, std::vector<const void*>>;
        using AllocMapIter = AllocMap::iterator;

        // IArena
        void *malloc(size_t size) override;
        void *realloc(void *ptr, size_t new_size, size_t) override;
        void free(void *ptr, size_t) override;

        void set_name(const void *ptr, const char *new_name) override;
        void set_parent(const void *ptr, const void *parent) override;

        // actually a DrawType, stored as an int for direct use with imgui
        int draw_mode;

        size_t counter = 0;

        // number of calls to each function
        size_t malloc_calls = 0;
        size_t realloc_calls = 0;
        size_t free_calls = 0;

        // usage stats in bytes
        size_t peak_memory_usage = 0;
        size_t live_memory_usage = 0;

        // all currently live allocations
        std::unordered_set<void *> live_allocs;

        struct frame_info_t
        {
            size_t count = 0; // number of times this frame has been seen

            frame_resolve_t resolve = eResolveNothing;
            std::string name;
            std::string path;
            size_t line = 0;
        };

        mutable std::unordered_map<bt_address_t, frame_info_t> info;

        const frame_info_t& get_frame_info(bt_address_t address) const;

        std::vector<backtrace_t> traces;
        size_t add_backtrace(const backtrace_t& trace);

        // all allocations
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
        void draw_backtrace(bt_address_t address) const;
        void draw_name(const alloc_info_t& info) const;
        void draw_extern_name(const void *ptr) const;
        void draw_tree_child(const void *ptr, const alloc_info_t& info) const;
        void draw_tree_group(const void *ptr, const alloc_info_t& info) const;
        void draw_tree_node_info(const void *ptr, const alloc_info_t& info) const;
        void draw_tree_node(const void *ptr) const;
        void draw_tree() const;

        // flat output
        void draw_flat() const;

        // IEditorPanel
        void draw_content() override;

    public:
        /// @brief the draw mode for the gui view of this allocator
        enum draw_mode_t : int
        {
            /// @brief draw the allocations as a tree using parent data
            eDrawTree,

            /// @brief draw the allocations as a flat list
            eDrawFlat
        };

        TraceArena(const char *id, draw_mode_t default_mode, panel_info_t setup = {});

        // TraceArena
        void draw_info();
        void reset();
    };
}
