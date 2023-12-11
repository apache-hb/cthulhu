## Contributing {#contributing}

### Source tree structure

* `data` - various data files
  * `meson` - cross files for various platforms
  * `scripts` - automation for arduous setup tasks
  * `docs` - documentation

* `driver` - language frontends
  * `pl0` - pl0 frontend, good for referencing how to use the common framework
  * `oberon` - oberon-2 frontend (WIP)
  * `ctu` - cthulhu language frontend (WIP)
  * `jvm` - jvm classfile consumer (TODO)
  * `cc` - C11 frontend (TODO)
  * `example` - example frontend

* `interface` - user facing components used to interact with drivers and the collection
  * `cli` - command line user interface
  * `gui` - graphical user interface (TODO)

* `common` - common code
  * `core` - header only compiler specific
  * `stacktrace` - backtrace retrieval
  * `argparse` - command line parsing
  * `base` - base utils
  * `memory` - memory allocation and arenas
  * `interop` - flex & bison helper functions
  * `platform` - platform detail wrappers
  * `io` - file io abstraction
  * `report` - error reporting tools
  * `scan` - flex & bison scanning tools
  * `std` - collections and data structures

* `cthulhu` - compiler framework library
  * `include/cthulhu` - public interface
    * `emit` - ssa emitter
    * `tree` - common typed ast
    * `util` - common utilities
    * `mediator` - code required to orchestrate communication between languages, the framework, and frontends
    * `check` - validates state for various structures
    * `ssa` - tree to ssa transforms, as well as optimizations

* `subprojects` - dependencies
  * `mini-gmp` - fallback gmp library if system gmp isnt installed

* `tests` - tests
  * `lang` - language specific tests
  * `unit` - compiler code unit tests

### Styleguide

#### declarations
* new and delete methods should be named `type_new` and `type_delete` respectivley
* files should be organised in the same order

```c
// includes

// macros

// typedefs

// forward declarations

// implementations
```

* use `const` whenever its easy to do so
* use `#pragma once` over include guards
* use `#define` for constant values in headers
* use `static const` for constant values in source files
* use `enum` for defining related constant values

#### banned features
* no VLAs & `alloca`, hard to debug, causes crashes very easily
* no `volatile`, it doesnt do what you think it does
* no compiler specific extensions without ifdef guards
* no mutable global state, all code must be reentrant and thread safe
* no inline asm
* no thread local values, these are global mutable state

### TODO

#### next up

* rewrite reporting
    * more detailed location tracking
    * fix current reports
    * support for multiple formats
    * option to disable colouring
    * complete cthulhu frontend
    * proper unit testing

* error registry like msvc

* rewrite location tracking
* fix linking to ucrt with clang-cl causing issues
* more docs

#### important

* build as shared libraries nicely on windows
* validate full tree form inside check
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

* add newtypes to tree

* C frontend
    * perhaps support a small subset of C++ so we can use it internally

#### backlog

* properly prefix the entire compiler
  * will make writing plugins easier in the future

* unit tests for everything in `/common/`
* driver specific config support
* replace mpq with mpfr (rounding is important)
* profile startup time
    * something is afoot since commit c64ef228a2a4edd01bd7eb1544e1a2cc05274e1b

#### bikeshedding

* generic interface compilation option
    * use dynamic libraries rather than static linking

* fix tuning
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
