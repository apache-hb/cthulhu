# Cthulhu
wow this project got out of hand.

## Dependencies

* build dependencies
  * `meson`
  * `ninja-build`
  * `flex + bison`
    * on windows install the `winflexbison3` package instead of `winflexbison` from choco
  * `pkg-config`
  * `build-essential`
  * `libgmp-dev` (optional)
  * `doxygen` (optional)

* runtime dependencies
  * `libgmp` (optional)

## Building

Requires a C11 compliant compiler targetting a platform with `uintptr_t`

```sh
meson setup build # configure build directory
ninja -C build # build compiler and drivers
ninja -C build docs # build documentation
ninja -C build test # build and run tests
```

## Structure

* `data` - various data files

* `driver` - language frontends
  * `pl0` - example pl0 frontend, good for referencing how to use the common framework
  * `ctu` - cthulhu language frontend (TODO)
  * `cc` - C11 frontend (TODO)

* `interface`
  * `cmd` - command line user interface
  * `gui` - graphical user interface (TODO)

* `common/include` - common code
  * `argparse` - command line parsing library
  * `base` - memory allocation library
  * `interop` - flex & bison helper functions
  * `platform` - platform detail wrappers
  * `io` - file io abstraction
  * `report` - error reporting tools
  * `scan` - flex & bison scanning tools
  * `std` - collections and data structures

* `cthulhu` - compiler framework library
  * `include/cthulhu` - public interface
    * `emit` - tree writing
    * `hlir` - common typed ast
    * `util` - common utilities
    * `ssa` - ssa emitter (TODO)

* `subprojects` - 3rd & 1st party dependencies
  * `cjson` - json serialization + deserialization library
  * `mini-gmp` - fallback gmp library if system gmp isnt installed
  * `ct-test` - testing framework
  * `glad` - gl loader for gui interface
  * `glfw` - windowing library for gui interface
  * `imgui` - ui library for gui interface

* `tests` - tests
  * `lang` - language specific tests
  * `interface` - interface specific tests
  * `unit-tests` - compiler code unit tests
  * `harness` - language test harness interface

* `tools` - tools
  * `tune-map` - map runtime perf tuning
