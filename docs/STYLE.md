# Styleguide

## declarations
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
* use `#define` for constant values
* use `enum` for defining related constant values

## banned features
* no VLAs & `alloca`, hard to debug, easy to crash
* no `volatile`, it doesnt do what you think it does
* no compiler specific extensions without ifdef guards
* no mutable global state, all code must be reentrant and thread safe
* no inline asm
