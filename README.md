# Cthulhu

Probably better than javascript

## Building
* building requires `meson` `ninja` and a C99 compliant compiler
* building also requires `flex` and `bison`
    - windows users can use `win_flex` and `win_bison` from choco

## Overview

* lex + parse with flex and bison
* symbol resolution, typechecking in semantic analysis
* convert ast into typed ast for IR
* convert typed ast into SSA
* optimize SSA form (TODO)
* use backend to produce end library/executable from SSA

Each stage will try and continue if it encounters any errors but will exit at the end of the stage.
Stages after semantic analysis can expect that input will always be correct.

## Licensing

This project is licensed under [AGPL3](./LICENSE).
