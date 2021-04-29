#include "cthulhu.h"

namespace cthulhu {
    stream::stream(stream::handle* handle)
        : source(handle)
        , ahead(handle->next())
    { }
    
    char stream::peek() {
        return ahead;
    }

    char stream::next() {
        char c = ahead;
        ahead = source->next();
        return c;
    }
}