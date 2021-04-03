# Modules & Includes

Every source file is its own module defined by the name of the source file. All symbols inside a module are visible by any other module that decides to include the submodule.

## Including modules

Submodules can be included at the top of a module with the `using` statement. 
* All includes must be listed before any declarations.
* Modules can include 0 or more submodules.
* Modules may choose to include all symbols from a submodule, may alias symbols in their current module or may namespace the included submodule locally.
* Submodules included symbols are not visible outside of the current module when the current module is included as a submodule.

### Namespaced import

```ct
# plain namespace import
using A;

# aliased namespace import
using A as B;
```

Accessing any symbol from module `A` when using a plain import requires the symbol to be resolved with `A::symbol`.
Accessing any symbol from module `A` with using an aliased import requires the symbol to be resolved with `B::symbol` where `symbol` is a symbol in module `A`.

### Wildcard import

```ct
using A(...);
```

Accessing any symbol from module `A` when using a wildcard import requires no prefix.
If multiple wildcard imports are used and clashing symbols are contained within each a diagnostic may be issued and the compilation will fail.

### Narrow import

```ct
using A(A1);

using A(A1, A2);

using A(A1 as A2);
```

Accessing specified symbols from modules when using narrow imports requires no namespace prefix.
Clashing symbols may not be imported without aliasing.

## Searching for modules

including the module `A` will search first for a file named `A.ct`, and then `A/A.ct`. the first matching file will be used, if neither file is found then compilation is aborted.
including the module `A::B::C` will search first for `A/B/C.ct`, then for `A/B/C/C.ct`.
