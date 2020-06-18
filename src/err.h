#pragma once

namespace ct::err {
    template<typename T>
    struct Stream {
        T stream;
        int get() { return stream.get(); }

        bool success = true;

        void fatal(std::string msg) {
            
        }
    };
}