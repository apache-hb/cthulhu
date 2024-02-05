#pragma once

#include "editor/trace.hpp"
#include "editor/sources.hpp"

#include "cthulhu/broker/broker.h"

typedef struct logger_t logger_t;
typedef struct support_t support_t;
typedef struct loader_t loader_t;

namespace ed
{
    class Broker
    {
    public:
        Broker(loader_t *loader, const char *name);

        // editor objects
        ed::TraceArena global{"global", ed::TraceArena::eDrawTree};
        ed::TraceArena gmp{"gmp", ed::TraceArena::eDrawFlat};

        // run state
        std::string name;

        // api objects
        bool setup = false;
        broker_t *broker = nullptr;
        loader_t *loader = nullptr;
        support_t *support = nullptr;

        ed::SourceList sources;

        /// @brief parse a source file
        ///
        /// @param index the index of the source file to parse
        ///
        /// @return nullptr on success, otherwise an error message
        char *parse_source(size_t index);

        void init();

        /// @brief check if there are any reports
        ///
        /// @retval true if there are no reports
        /// @retval false if there are reports
        bool check_reports() const;

    private:
        void init_alloc();
        void init_support();
    };
}
