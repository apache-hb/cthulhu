# TODO {#todo}

GFI = good first issue

## First priority

- prefix all config macros with `CTU_` and all other macros with `CT`

- support for feeding sources directly into compilers
    - the intermediate step is annoying to users

- reduce the amount of allocations
    - will need to make some types no longer opaque
    - maybe there is a way to keep encapsulation despite this
    - will need a typed map
    - need to move over to a flyweight pattern for some objects to avoid pointer invalidation

- support building source files as both C and C++
    - we basically require C++20 because of how often designated init is used

- simplify alot of the error formatting code

- update unit tests to require specific error codes

- unify all the format interfaces
    - very annoying to have to pass these massive structs everywhere
    - also make struct constructors

- unit testing for error reporting
- unit testing for location tracking in frontends

- electric fence allocators for unit tests
    - make sure common modules dont use global allocator

- hash consing for driver asts and trees
    - should probably incorporate protobuf into this for fuzzing

- error report merging is really bad
    - make it optional
    - make it better

- allow for swapping between error reporting formats

- complete the C preprocessor
    - at least be able to parse the windows and linux headers

### Important

- fix linking to ucrt with clang-cl causing issues (GFI)

- vendor flex+bison ourselves to always have the best version
    - will also allow us to not use winflexbison from chocolatey
    - needs at least m4, gettext, and glib ported as well

- make the output of rich text notifications reproducible
    - order of segments changes sometimes because of set_add_ptr being used

- find a way to nicely wrap exception info for segfaults
    - need to wrap for client code to work across platforms

- use SetUnhandledExceptionFilter on windows
    - this will allow us to get stack traces on unhandled exceptions

- sigaction on linux
    - not sure how the async signal saftey will go

- build as shared libraries nicely on windows
    - at least get mini gmp working (GFI)

- validate full tree form inside check
    - this should probably be an optional build time option

- ssa backend
    - wasm
    - x64

- detect mutually recursive functions in stacktrace collection
    - this is a bit tricky

- valist forwarding
- enum consts
- full C and fixed width types

- name mangling in the ssa backend
    - nested modules support
    - requires name mangling

- add newtypes to tree (GFI)

- C frontend
    - perhaps support a small subset of C++

- more docs (GFI)

### Backlog

- make backtrace formats prettier
    - fix the frame index formatting
    - figure out the off by one errors in line numbers

- properly prefix the entire compiler (GFI)
  - will make writing plugins easier in the future

- unit tests for everything in `/common/`
- driver specific config support
- replace mpq with mpfr (rounding is important)
- profile startup time during testing (GFI)
    - something is afoot since commit `c64ef228a2a4edd01bd7eb1544e1a2cc05274e1b`

#### Bikeshedding

- generic interface compilation option
    - use dynamic libraries rather than static linking

- fix tuning
- rewrite gui frontend
- add language server support
- move to incremental compilation
- more frontend tests
- continued code cleanup
- add plugin support
- add binary module support
- support [ifc](https://github.com/microsoft/ifc-spec) modules
- build time option for internal compiler timing
- better code coverage with tests
- VM execution
    - debugging, interactive shell
- support llvm libfuzzer
- async file reading

- custom lex + parser file format
    - autogenerate flex and bison
    - autogenerate vscode grammars
    - autogenerate protobuf defs
    - reduces alot of boilerplate

- fuzz protobuf rather than direct source code
- windows COM support
- generate code for other runtimes?
    - JVM
    - CLR
    - python
