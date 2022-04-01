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

* src - private source & header files
  * ast - flex and bison glue code
  * driver - common frontend code
  * lir - lisp like intermediate format
  * ssa - ssa intermediate format
  * util - utility code
* include/cthulhu - public headers
  * ast - flex and bison glue
  * driver - common frontend code
  * lir - lisp like intermediate format
  * ssa - ssa intermediate format
  * util - utility code
* driver - compiler drivers
  * pl0 - pl0 language driver
  
# License
Currently licensed under [AGPL3](LICENSE). Money can change this.
