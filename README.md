# Cthulhu
wow this project got out of hand.

## Building

```sh
meson build # configure build directory
ninja -C build # build compiler and drivers
ninja -C build docs # build documentation
ninja -C build test # build and run tests
```

## Dependencies

* build dependencies
  * `meson`
  * `ninja-build`
  * `flex`
  * `bison`
  * `pkg-config`
  * `build-essential`
  * `libgmp-dev` (optional)
  * `doxygen` (optional)

* runtime dependencies
  * `libgmp` (optional)

## Structure

* `data` - data files used to build the compiler
* `driver` - language frontends
  * `pl0` - example pl0 frontend, good for referencing how to use the common framework
  * `ctu` - cthulhu language frontend (TODO)

* `interface`
  * `cmd` - command line user interface
  * `gui` - graphical user interface (TODO)

* `include/cthulhu` - public interface
  * `ast` - tools for generating an ast components used by drivers
  * `emit` - tree writing
  * `hlir` - common typed ast
  * `util` - common utilities

* `src` - common framework implementation

* `subprojects` - 3rd & 1st party dependencies
  * `cjson` - json serialization + deserialization library
  * `mini-gmp` - fallback gmp library if system gmp isnt installed
  * `ct-test` - testing framework
  * `glad` - gl loader for gui interface
  * `glfw` - windowing library for gui interface
  * `imgui` - ui library for gui interface

* `tests` - tests
  * `driver` - language specific tests
  * `interface` - interface specific tests
  * `unit-tests` - compiler code unit tests
  * `harness` - language test harness interface
  * `fuzzing` - fuzzing interface

* `tools` - tools
  * `tune-map` - map runtime perf tuning
