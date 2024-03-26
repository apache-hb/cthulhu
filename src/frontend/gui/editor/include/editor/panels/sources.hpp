#pragma once

#include "editor/utils/io.hpp"

#include "io/io.h"

#include <string>
#include <vector>
#include <filesystem>

typedef struct io_t io_t;
typedef struct arena_t arena_t;
typedef struct language_t language_t;
typedef struct lifetime_t lifetime_t;

namespace ed
{
    namespace fs = std::filesystem;

    class SourceView
    {
        std::string directory;
        std::string basename;

        ctu::Io io;
        ctu::OsError error;
        std::string_view source;

        std::vector<size_t> line_offsets;

        void build_line_offsets();

    public:
        SourceView(const fs::path& ospath);

        const char *get_basename() const { return basename.c_str(); }
        const char *get_path() const { return directory.c_str(); }
        size_t get_size() const { return source.size(); }
        io_t *get_io() const { return io.get(); }

        void draw_content();
    };
}
