# Cthulhu
wow this project got out of hand.

## Building

* build dependencies
  * `meson`
  * `ninja-build`
  * `flex`
  * `bison`
  * `pkg-config`
  * `build-essential`
  * `libgmp-dev` (optional)

* runtime dependencies
  * `libgmp` (optional)

## Structure

* `data` - data files used to build the compiler
* `driver` - language frontends
  * `pl0` - example pl0 frontend, good for referencing how to use the common framework
  * `ctu` - cthulhu language frontend
* `include/cthulhu` - public interface
  * `ast` - tools for generating an ast components used by drivers
  
* `src` - common framework implementation
