#pragma once

#include <string>
#include <vector>

namespace ed
{
    struct SourceList
    {
        void draw();

        std::vector<std::string> paths;

        bool is_empty() const { return paths.empty(); }
        size_t count() const { return paths.size(); }
        const char *get(size_t idx) const { return paths[idx].c_str(); }

    private:
        char buffer[1024] = {};
    };
}
