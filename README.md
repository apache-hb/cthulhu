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
  * `ctu` - cthulhu language frontend
* `include/cthulhu` - public interface
  * `ast` - tools for generating an ast components used by drivers
  * `driver` - common frontend command line and parsing
  * `emit` - tree writing
  * `hlir` - common typed ast
  * `loader` - binary loading and unloading
  * `ssa` - future ssa interface
  * `util` - common utilities
* `src` - common framework implementation
* `subprojects` - 3rd & 1st party dependencies
  * `aws` - C implementation of the aws lambda runtime api
  * `cjson` - json serialization + deserialization library
  * `mini-gmp` - fallback gmp library if system gmp isnt installed
  * `miniz` - zip compression library
* `tests` - langauge tests
* `tools` - tools
  * `create-lambda` - create a lambda from a function
  * `tune-map` - map runtime perf tuning
