# Building Cthulhu {#building}

## Prerequisites

Configuring and building Cthulhu requires the following tools:
    - <a href="https://mesonbuild.com/Getting-meson.html">Meson</a>
    - <a href="https://ninja-build.org/">Ninja</a>
    - <a href="https://github.com/westes/flex">Flex</a> and <a href="https://www.gnu.org/software/bison/">Bison</a>. On windows these can be acquired through <a href="https://community.chocolatey.org/packages/winflexbison3">Chocolatey</a>
    - A C11 compliant compiler.

Cthulhu can optionally use the following tools to enable additional features:
    - <a href="https://www.doxygen.nl/">Doxygen</a> for generating documentation.
    - <a href="https://gmplib.org/">GMP</a> for better performance compile times.
        - Note that if libgmp is used, it must also be available at runtime.
        - The compiler contains a build time fallback implementation of GMP when it is not available at build time.
    - A C++20 compiler for building the GUI tools.
        - Currently these tools are only available on Windows, and require a d3d12 capable GPU.
    - A Java 8+ compiler for building the jvm driver tests and standard library.

Cthulhus current runtime dependencies are:
    - <a href="https://gmp.org/">GMP</a>; only if the compiler is built with GMP support.

## Configuration

All available build options are listed in the `meson_options.txt` file found in the projects root directory.

```sh
meson setup build
ninja -C build
ninja -C build docs # build documentation, requires doxygen
ninja -C build test # run tests
```

### Consuming as a meson subproject

By default as a subproject the stable language drivers are built.
If you want only a specific set of libraries you can disable everything with the following incantaion.

I'll add a `-Deverything=disabled` at some point

```sh
meson setup build -Dlang_ctu=disabled -Dlang_pl0=disabled -Dlang_oberon=disabled -Dlang_example=disabled -Dlang_c=disabled -Dlang_cpp=disabled -Dfrontend_cli=disabled -Dfrontend_example=disabled -Dfrontend_gui=disabled -Dtrace_memory=disabled -Dparanoid=disabled -Dunit_tests=disabled -Ddriver_tests=disabled -Ddoxygen=disabled -Dtool_notify=disabled -Dtool_diagnostic=disabled -Dtool_display=disabled -Dtool_error=disabled
```

This does not disable the targets, so they are still available by `get_variable`. all targets are `build_by_default : false` so meson will compile them on demand.
