#include "tstream.hpp"

int main() {
    auto handle = FileStream("data/plain/hello.txt");
    auto stream = Stream(&handle);

    ASSERT(stream.peek() == 'h');
    ASSERT(stream.next() == 'h');
    ASSERT(stream.next() == 'e');
    ASSERT(stream.next() == 'l');
    ASSERT(stream.next() == 'l');
    ASSERT(stream.next() == 'o');
    ASSERT(stream.next() == END);
    ASSERT(stream.next() == END);
}
