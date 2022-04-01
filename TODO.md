# TODO items

## Blocking items

* split out `HLIR_VALUE` into distinct types
    * `HLIR_LOCAL` type for function locals
    * `HLIR_PARAM` type for function parameters
    * `HLIR_GLOBAL` type for module globals
    * this makes generating wasm possible
    * also makes generating ssa easier when the time comes

## Optional items

* add more builder functions for various hlir types
    * this would make some boilerplate shorter

* add json support to bytecode generation
    * this would allow us to remove json as a target
