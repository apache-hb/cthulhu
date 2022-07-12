# TODO

* remove hlir parent data
    * nodes should not care about the context they're used in
* ssa backend
    * wasm
    * x64
* deprecate C backend, move to something else
* rewrite reporting
    * more detailed location tracking
    * fix current reports
    * support for multiple formats
    * option to disable colouring
    * complete cthulhu frontend
    * proper unit testing
* C frontend
    * perhaps support a small subset of C++ so we can use it internally
* rewrite gui frontend
* add language server support
* move to incremental compilation
* error registry like msvc
* unit tests for everything in `/common/`
* more frontend tests
* continued code cleanup
* add plugin support back in
* complete api docs
* add binary module support back in
* support [ifc](https://github.com/microsoft/ifc-spec) modules
* custom allocators
* build time option for internal compiler timing
* better code coverage with tests
* perhaps custom lexer + parser generator
    * this is alot of effort
    * will need further thought
* VM execution
    * debugging, interactive shell
