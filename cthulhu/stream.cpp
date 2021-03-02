#include "stream.hpp"

namespace cthulhu {
    Stream::Stream(StreamHandle* handle)
        : handle(handle)
        , ahead(handle->next())
    { }

    c32 Stream::next() {
        c32 c = ahead;
        ahead = handle->next();
        return c;
    }

    c32 Stream::peek() {
        return ahead;
    }
}
