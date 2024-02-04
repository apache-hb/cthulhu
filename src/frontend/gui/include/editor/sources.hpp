#pragma once

#include "core/text.h"
#include <string>
#include <vector>

typedef struct io_t io_t;
typedef struct arena_t arena_t;
typedef struct language_t language_t;
typedef struct lifetime_t lifetime_t;

namespace ed
{
    class Source
    {
    public:
        Source(const char *str, arena_t *arena);

        const char *get_title() const { return basename; }
        const char *get_path() const { return path.c_str(); }
        size_t get_size() const { return source.length; }

        void draw();

        std::string path = "";
        const char *basename = nullptr;

        io_t *io = nullptr;
        const char *error_string = nullptr;

        text_view_t source = {};
    };

    class SourceList
    {
    public:
        SourceList(arena_t *arena)
            : arena(arena)
        { }

        void draw();

        std::vector<Source> paths;

        bool is_empty() const { return paths.empty(); }
        size_t count() const { return paths.size(); }
        const char *get_path(size_t index) const { return paths[index].get_path(); }
        io_t *get_io(size_t index) const { return paths[index].io; }

    private:
        arena_t *arena;
        char buffer[1024] = {};
    };
}
