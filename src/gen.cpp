#pragma once

namespace ct {
    template<typename O>
    struct Gen {
        template<typename... T>
        Gen(T &&... args)
            : out(args...)
        { }
        O out;
    };

    template<typename O>
    struct X86 : Gen<O> {

    };
}