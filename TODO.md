# TODO

## important

* ssa backend
    * wasm
    * x64
    
* deprecate C backend, move to something else
    * blocked by ssa backend

* remove hlir parent data
    * nodes should not care about the context they're used in
    * blocked by C backend

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

## bikeshedding

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
* perhaps custom lexer + parser generator
    * this is alot of effort
    * will need further thought
* VM execution
    * debugging, interactive shell
* replace AST definitions with protobuf for better fuzzing
* support llvm libfuzzer
* async file reading
* simplify `node_t`
