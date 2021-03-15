#include "stream.h"

Stream::Stream(StreamHandle* handle)
    : handle(handle)
    , ahead(handle->next())
{ }

char Stream::next() {
    char c = ahead;
    
    if (c) {
        ahead = handle->next();
    }

    return c;
}

char Stream::peek() {
    return ahead;
}
