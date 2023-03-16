# TODO

## important

* ssa backend
    * wasm
    * x64
    * c89
    
* deprecate C hlir backend
    * blocked by ssa backend

* rewrite reporting
    * more detailed location tracking
    * fix current reports
    * support for multiple formats
    * option to disable colouring
    * complete cthulhu frontend
    * proper unit testing

* C frontend
    * perhaps support a small subset of C++ so we can use it internally

* unit tests for everything in `/common/`
* complete api docs
* add opaque pointer type to replace eHlirPointer(eHlirUnit)
* driver specific config support
* replace mpq with mpfr (rounding is important)

## bikeshedding

* fix tuning 
* fix sal support
* rewrite gui frontend
* add language server support
* move to incremental compilation
* error registry like msvc
* more frontend tests
* continued code cleanup
* add plugin support back in
* add binary module support back in
* support [ifc](https://github.com/microsoft/ifc-spec) modules
* build time option for internal compiler timing
* better code coverage with tests
* VM execution
    * debugging, interactive shell
* support llvm libfuzzer
* async file reading

* custom lex + parser file format
    * autogenerate flex and bison
    * autogenerate vscode grammars
    * autogenerate protobuf defs 
    * reduces alot of boilerplate

* fuzz protobuf rather than direct source code
