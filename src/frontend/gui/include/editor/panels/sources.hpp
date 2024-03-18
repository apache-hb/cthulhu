#pragma once

#include "core/text.h"

#include "editor/panels/panel.hpp"

#include <string>
#include <vector>

typedef struct io_t io_t;
typedef struct arena_t arena_t;
typedef struct language_t language_t;
typedef struct lifetime_t lifetime_t;

namespace ed
{
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
