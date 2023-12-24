#pragma once

#include "editor/trace.hpp"
#include "editor/sources.hpp"

typedef struct mediator_t mediator_t;
typedef struct lifetime_t lifetime_t;
typedef struct logger_t logger_t;

namespace ed
{
    struct CompileInfo
    {
        CompileInfo(mediator_t *mediator, const char *name)
            : name(name)
            , mediator(mediator)
        { }

        // run state
        std::string name;

        // api objects
        bool setup = false;
        mediator_t *mediator = nullptr;
        lifetime_t *lifetime = nullptr;
        logger_t *reports = nullptr;

        // editor objects
        ed::TraceArena global{"global", ed::TraceArena::eDrawTree};
        ed::TraceArena gmp{"gmp", ed::TraceArena::eDrawFlat};

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
        void init_lifetime();
        void init_reports();
    };
}
