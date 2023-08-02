# TODO

## next up

* rewrite reporting
    * more detailed location tracking
    * fix current reports
    * support for multiple formats
    * option to disable colouring
    * complete cthulhu frontend
    * proper unit testing

* error registry like msvc

* rewrite location tracking

* more docs

## important

* memory arenas

* ssa backend
    * wasm
    * x64

* valist forwarding
* enum consts
* full C and fixed width types

* name mangling in the ssa backend
    * nested modules support
    * requires name mangling

* deprecate C hlir backend
    * blocked by ssa backend

* add newtypes to hlir

* C frontend
    * perhaps support a small subset of C++ so we can use it internally

* unit tests for everything in `/common/`
* driver specific config support
* replace mpq with mpfr (rounding is important)
* profile startup time
    * something is afoot since commit c64ef228a2a4edd01bd7eb1544e1a2cc05274e1b

## bikeshedding

* generic interface compilation option
    * use dynamic libraries rather than static linking

* fix tuning
* fix sal support
* rewrite gui frontend
* add language server support
* move to incremental compilation
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
* windows COM support
* generate code for other runtimes?
    * JVM
    * CLR
    * python
