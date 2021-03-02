#include "tstream.hpp"

int main() {
    auto handle = FileStream("data/empty.txt");
    auto stream = Stream(&handle);

    ASSERT(stream.peek() == END);
    ASSERT(stream.next() == END);
    ASSERT(stream.next() == END);
    ASSERT(stream.next() == END);
}
