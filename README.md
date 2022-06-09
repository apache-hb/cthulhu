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
  * `flex + bison`
    * on windows install the `winflexbison3` package over `winflexbison` from choco
  * `pkg-config`
  * `build-essential`
  * `libgmp-dev` (optional)
  * `doxygen` (optional)

* runtime dependencies
  * `libgmp` (optional)

## Structure

* `data` - various data files

* `driver` - language frontends
  * `pl0` - example pl0 frontend, good for referencing how to use the common framework
  * `ctu` - cthulhu language frontend (TODO)

* `interface`
  * `cmd` - command line user interface
  * `gui` - graphical user interface (TODO)

* `common/include` - common code
  * `base` - memory allocation library
  * `std` - collections and data structures
  * `platform` - platform detail wrappers
  * `cmd` - command line parsing library
  * `report` - error reporting tools
  * `argparse` - argument parsing
  * `scan` - flex & bison scanning tools
  * `interop` - flex & bison helper functions

* `cthulhu` - compiler framework library
  * `include/cthulhu` - public interface
    * `emit` - tree writing
    * `hlir` - common typed ast
    * `util` - common utilities

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
