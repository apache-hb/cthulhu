#pragma once

#include "fwd.hpp"

namespace cthulhu {
    static constexpr c32 END = c32(-1);

    struct StreamHandle {
        virtual ~StreamHandle() { }
        virtual c32 next() = 0;
    };

    struct Stream {
        Stream(StreamHandle* handle);

        c32 next();
        c32 peek();
    private:
        StreamHandle* handle;
        c32 ahead;
    };
}
