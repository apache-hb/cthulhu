# Cthulhu

Probably better than javascript

## Building
* building requires `meson`, `ninja`, `gmp` and a C99 compliant compiler
* building also requires `flex` and `bison`
    - windows users can use `win_flex` and `win_bison` from choco

```sh
meson build
ninja -C build

# running tests
ninja -C build test

# using the compiler
./build/ctc --help
```

## Overview

* lex + parse with flex and bison
* symbol resolution, typechecking in semantic analysis
* convert ast into typed ast for IR
* convert typed ast into SSA
* optimize SSA form
* use backend to produce end library/executable from SSA 
  * assembler backend (TODO)

Each stage will try and continue if it encounters any errors but will exit at the end of the stage.
Stages after semantic analysis can expect that input will always be correct.

## Tooling

* syntax highlighting for vscode is available [here](https://github.com/apache-hb/ctu-vscode)

## Licensing

This project is licensed under [AGPL3](./LICENSE).
