# Styleguide

## Naming

* all functions are `snake_case`
* all constants are `SCREAMING_SNAKE_CASE`
* all macros are `SCREAMING_SNAKE_CASE`
* all types and typedefs are `snake_case` and suffixed with `_t`

## declarations
* pointer stars are on the right side of declarations
```c
void *data; // do this
void* data; // not this
```
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
* use west const
* use `#pragma once` over include guards

## banned features
* no VLAs & `alloca`, hard to debug, easy to crash
* no volatile, it doesnt do what you think it does
* no compiler specific extensions
* no mutable global state, all code must be reentrant and thread safe
* no inline asm
