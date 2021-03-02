#include "test.hpp"
#include "tstream.hpp"

int main() {
    auto handle = StringStream("");
    auto stream = Stream(&handle);

    ASSERT(stream.peek() == END);
    ASSERT(stream.next() == END);
    ASSERT(stream.next() == END);
    ASSERT(stream.next() == END);
}
