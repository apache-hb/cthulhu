# Contributing {#contrib}

## Source tree structure

- `data` - various data files
  - `docs` - documentation
  - `meson` - cross files for various platforms
  - `scripts` - automation for arduous setup tasks

- `src` - all source code
  - `common` - common code. @ref common
    - `arena` - memory allocation arenas. @ref memory
    - `backtrace` - backtrace retrieval. @ref backtrace
    - `base` - base utils. @ref base
    - `config` - configuration values. @ref config
    - `core` - header only compiler specific code. @ref core
    - `endian` - endian handling, byteswaps. @ref endian
    - `fs` - filesystem abstraction. @ref filesystem
    - `interop` - flex & bison helper functions. @ref interop
    - `io` - file io abstraction. @ref io
    - `notify` - error reporting tools. @ref notify
    - `os` - platform detail wrappers. @ref os
    - `scan` - flex & bison scanning tools. @ref location
    - `std` - collections and data structures. @ref standard

  - `cthulhu` - compiler framework library. @ref runtime
    - `broker` - common module communication setup. @ref broker
    - `check` - validates state for various structures. @ref check
    - `emit` - ssa emitter. @ref emit
    - `events` - shared compiler events between languages. @ref events
    - `memory` - global arena allocator. @ref global_memory
    - `ssa` - tree to ssa transformation. @ref ssa
    - `tree` - common typed ast. @ref tree
    - `util` - common utilities. @ref runtime_util

  - `frontend` - user facing components used to interact with drivers and the collection
    - `cli` - command line user interface
    - `example` - example usage of the runtime
    - `gui` - graphical user interface built with imgui (partial)

  - `language` - language frontends
    - `driver` - language helper library. @ref driver
    - `c` - C11 frontend (skeleton)
      - `pre` - C preprocessor
      - `lang` - C language driver
    - `ctu` - cthulhu language frontend (beta)
    - `example` - example frontend
    - `jvm` - jvm classfile consumer (skeleton)
    - `oberon` - oberon-2 frontend (partial support)
    - `pl0` - pl0 frontend, good for referencing how to use the runtime (dare I say production ready)

  - `loader` - language module enumeration and loading
    - `support` - support library for frontends and interating with the broker. @ref loader

  - `plugins` - compiler plugins
    - currently empty

  - `support` - utility libraries. @ref support
    - `argparse` - command line argument parsing. @ref argparse
    - `driver` - support library for creating language drivers. @ref driver
    - `format` - text formatting for displaying to a user. @ref format
    - `gluon` - small build system implementation, similar to meson (skeleton)
    - `json` - json parsing library, does not currently support writing. @ref json
    - `meta` - dsl for generating compiler data structures
    - `setup` - default options for command line tools. @ref setup
    - `toml` - toml parser (TODO)

  - `target` - codegen target libraries
    - `cfamily` - C and C++ codegen
    - `debug` - debug output
    - `hlsl` - hlsl shader target

  - `tools` - supporting tools and test utilities
    - `diagnostic` - diagnostic listing and querying tool
    - `display` - example config display
    - `error` - test tool for examining stacktraces
    - `harness` - test harness for end to end language tests
    - `meson_ctu` - an implementation of meson that is aware of cthulhu
    - `notify` - testing notification formatting

- `subprojects` - dependencies
  - `mini-gmp` - fallback gmp library if system gmp isnt installed

- `tests` - tests
  - `lang` - language specific tests
  - `unit` - compiler code unit tests

### Where to put new code

When adding a new module consider how much of the compiler needs access to it. Treat levels of access as if they were permission levels, with @ref common being the highest level. If a library will only ever be used by a user facing tool, it should be part of the @ref support set of modules. When a module needs to be available to drivers it should be in @ref runtime, the core compiler set. And if a module is going to be used extensively, and perhaps outside of the cthulhu project it should be placed in @ref common.

## Coding rules

- Follow single source of truth
  - This is the single most important rule of the codebase
  - We want as little duplicated logic as possible
  - Follow it to absurdity
  - It is acceptable to make massive changes to move common functionality into libraries
  - I cannot stress how important this rule is

- Be very generous with usage of @ref panic
  - Any function boundary or usage point should be littered with these
  - Do not use asserts to handle user input, asserts should be used for internal invariants
  - For user facing error reporting look at @ref notify

- All features that require platform specific code must have a reasonable fallback implementation.
  - See @ref backtrace for an example of this, doing nothing can be a reasonable fallback.

- Everything must be implemented in standard C11
  - No compiler extensions in common code.

- No io or filesystem access that isnt marhshalled by @ref io or @ref filesystem
  - Makes porting to systems with non tradition IO easier.

- The build process must only rely on C and meson
  - Optional features may require python/C++
  - To aid porting to systems that may not have a big ecosystem
  - We should never rely on an older version of cthulhu, the version - 1 problem is not a fun one.

- All platform specific code must go in the @ref os module

- Forward declare all types where possible rather than including headers
  - This makes renaming types harder but reduces build times significantly
  - Also dont export dependencies unless absolutely needed

- No internal versioning
  - Breaking source and ABI compatibility every commit is fine
  - Once plugins are implemented maybe i'll rethink this

## Memory management strategy

Cthulhu aims to be usable as a library in embedded systems (read as: places without global malloc).
As such we aim to only use user provided allocators, and to not use any global mechanisms for allocation.

<h3>Naming and style for allocating interfaces</h3>
In order of importance:

1. When an object is heap allocated its constructor must be named `<type>_new` and need a @ref arena_t as the last parameter.
  - `map_new`, `vector_new`, `set_new` etc

2. When an object is stack allocated a `<type>_init` function should be provided when construction requires logic

```c
typedef struct text_t
{
  const char *string;
  size_t length; // must be equal to `strlen(string)`
} text_t;

void text_init(text_t *text, const char *string)
{
  size_t length = strlen(string);
  text->string = string;
  text->length = length;
}
```

3. If an object only requires memory then do not provide a `delete` function unless very necessary
  - Types that manage external resources *must* provide a delete function

4. If a type could reasonably be allocated on either the stack or heap a `<type>_init` function should be provided
  - These should take a pointer to an uninitialized instance of the object and populate the required fields
  - Providing both `_new` and `_make` functions may also be good depending on how commonly the type is used
    - These functions should always be wrappers for the `_init` functions logic

```c
typedef struct context_t
{
  int first;
  int second;

  context_t *parent;
} context_t;

inline void context_init(context_t *ctx, context_t *parent)
{
  ctx->first = context_get_field(parent, 0);
  ctx->second = context_get_second(parent, 1);

  ctx->parent = parent;
}
```

## Portability considerations

Cthulhu also aims to be easy to port to more "exotic" systems, as such avoid relying on things that are not totally garunteed by the C specification.

- For example prefer `[u]int_fastNN_t` or `[u]int_leastNN_t` over `[u]intNN_t` as both the former types are always present
- Aim to do all floating point math via `gmp` or `mpfr` as they have consistent rounding rules
- Prefer `size_t` and `ptrdiff_t` over fixed width types when managing sizes
  - The only exception to these rules is `uintptr_t` which is required to build the compiler, if or when its possible to remove this I will

- Avoid using obscure syntax
  - This is more to aid in readability but some compilers dont support all syntax

## Styleguide
* all macros in headers should be prefixed with `CT`
* all macros defined in generated files should be prefixed with `CTU_`
* all macros used in c++ should be prefixed with `CTX_`

* @ref arena_t should always be the last argument to a function
  * the exception to this rule is variadic functions

```c
// wrong
char *action_that_allocates(arena_t *arena, const char *config);

// correct
char *action_that_allocates(const char *config, arena_t *arena);

```
Source files contents should be layed out in the following order

```c
// includes

// macros

// typedefs

// forward declarations

// implementations
```
  - use `const` whenever its easy to do so
  - use `#pragma once` over include guards
  - use `#define` for constant unrelated values in headers
  - use `static const` for constant values in source files
  - use `enum` for defining related constant values

  <h4>Banned features</h4>
  - no VLAs & `alloca`, hard to debug, causes crashes very easily
  - no `volatile`, it doesnt do what you think it does
  - no compiler specific extensions without ifdef guards
  - no mutable global state, all code must be reentrant and thread safe
    - exceptions have been made for the global panic and os error handlers
  - no inline asm
  - no thread local values
  - no non-const static locals

### Flex/Bison styleguide
* `snake_case` for rules
* rules that match 1 or more of a rule should be named `<rule>_seq`
* rules that match a rule with a seperator should be named `<rule>_list`
```c
// for example
expr_list: expr
  | expr_list COMMA expr
  ;

stmt_seq: stmt
  | stmt_seq stmt
  ;
```
