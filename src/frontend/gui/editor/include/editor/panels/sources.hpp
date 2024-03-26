#pragma once

#include "core/text.h"

#include "editor/panels/panel.hpp"

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

    constexpr auto kIoDelete = [](io_t *io) { io_close(io); };
    using unique_io_t = std::unique_ptr<io_t, decltype(kIoDelete)>;

    class OsError
    {
        os_error_t error;
        mutable const char *string = nullptr;

    public:
        OsError(os_error_t error) : error(error) {}

        operator os_error_t() const { return error; }

        bool success() const;
        bool failed() const;

        const char *what() const;
    };

    class Io
    {
        unique_io_t io;

        Io(io_t *io) : io(io, kIoDelete) {}

    public:
        static Io file(const char *path);

        OsError error() const;

        size_t size() const;
        const void *map() const;

        std::string_view text() const;

        const char *name() const;

        io_t *get() const { return io.get(); }
    };

    class SourceView final : public IEditorPanel
    {
        std::string directory;
        std::string basename;

        Io io;
        OsError error;
        std::string_view source;

        std::vector<size_t> line_offsets;

        void build_line_offsets();

        // IEditorPanel
        void draw_content() override;

    public:
        SourceView(const fs::path& ospath, panel_info_t setup = {});

        const char *get_basename() const { return basename.c_str(); }
        const char *get_path() const { return directory.c_str(); }
        size_t get_size() const { return source.size(); }
        io_t *get_io() const { return io.get(); }
    };

    class Source final : public IEditorPanel
    {
        std::string path = "";
        const char *basename = nullptr;

        io_t *io = nullptr;
        const char *error_string = nullptr;

        text_view_t source = {};

        // IEditorPanel
        void draw_content() override;

    public:
        Source(std::string_view str, panel_info_t setup = {});

        const char *get_title() const { return basename; }
        const char *get_path() const { return path.c_str(); }
        size_t get_size() const { return source.length; }
        io_t *get_io() const { return io; }
    };

    class SourceList final : public IEditorPanel
    {
        std::string buffer;
        std::vector<Source> sources;

        // IEditorPanel
        void draw_content() override;

    public:
        SourceList(panel_info_t setup = {});

        bool is_empty() const { return sources.empty(); }
        size_t count() const { return sources.size(); }
        const char *get_path(size_t index) const { return sources[index].get_path(); }
        io_t *get_io(size_t index) const { return sources[index].get_io(); }
    };
}
