# Cthulhu

Probably better than javascript

## Building
* building requires `meson`, `ninja`, `gmp`, `flex`, `bison`, `pkg-config`, and `build-essential`

```sh
meson build
ninja -C build

# running tests
ninja -C build test

# using the compiler
./build/ctc --help
```

## Layout

* ctu - source
  * ast - generic ast
  * backend - all language backends from transpiling
  * driver - user facing driver and command line
  * frontend - all parser frontends
  * gen - converts generic ast to bytecode
  * lir - generic ast and semantic analysis context
  * pref - bytecode optimisation
  * type - generic type data
  * util - helper library
  
* tools - extra compiler development tools
  * tune.c - performance tuning for optimal map sizes

* tests - test cases
* subprojects - dependencies
  * mini-gmp - fallback in case system gmp is missing
  
## Overview

* lex + parse with flex and bison
* symbol resolution, typechecking in semantic analysis
* convert ast into typed ast for IR
* convert typed ast into SSA
* optimize SSA form
* use backend to produce end library/executable from SSA 

Each stage will try and continue if it encounters any errors but will exit at the end of the stage.
Stages after semantic analysis can expect that input will always be correct.

## Tooling

* syntax highlighting for vscode is available [here](https://github.com/apache-hb/ctu-vscode)

## Licensing

This project is licensed under [AGPL3](./LICENSE).
