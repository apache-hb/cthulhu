## Build guide {#building}

### Prerequisites

Configuring and building Cthulhu requires the following tools:
    - <a href="https://mesonbuild.com/Getting-meson.html">Meson</a>
    - <a href="https://ninja-build.org/">Ninja</a>
    - <a href="https://github.com/westes/flex">Flex</a> and <a href="https://www.gnu.org/software/bison/">Bison</a>. On windows these can be acquired through <a href="https://community.chocolatey.org/packages/winflexbison3">Chocolatey</a>
    - A C11 compliant compiler.

Cthulhu can optional use the following tools to enable additional features:
    - <a href="https://www.doxygen.nl/">Doxygen</a> for generating documentation.
    - <a href="https://gmplib.org/">GMP</a> for better performance of the compiler.
        - Note that if libgmp is used, it must also be available at runtime.
        - The compiler contains a build time fallback implementation of GMP when it is not available.
    - A C++20 compiler for building the GUI tools.
        - Currently these tools are only available on Windows, and require a d3d12 capable GPU.
    - A Java 8+ compiler for building the jvm driver tests and standard library.

Cthulhus current runtime dependencies are:
    - <a href="https://gmp.org/">GMP</a>; only if the compiler is built with GMP support.

### Configuration

```sh
meson setup build
ninja -C build
ninja -C build docs # build documentation, requires doxygen
ninja -C build test # run tests
```
