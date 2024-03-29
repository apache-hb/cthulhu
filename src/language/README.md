# Language driver layout

all drivers must conform to the same dir structure

* driver_name/
    * src/
    * include/
    * meson.build

## Example module

```c
// src/main.c

/// default language extensions, must be null terminated
static const char *const kLangNames[] = CT_LANG_EXTS("e", "example");

/// exported driver module
const language_t kExampleModule = {
    /// the internal name of the driver
    .id = "example",

    /// the human visible name of the driver
    .name = "Example",

    /// version information
    .version = {
        .license = "GPLv3",
        .desc = "Example language driver",
        .author = "Elliot Haisley",
        .version = CT_NEW_VERSION(1, 0, 1)
    },

    /// default extensions
    .exts = kLangNames,

    /// first time creation function, initialize from gathered config
    .fn_create = ex_create,

    /// final teardown function, clean up memory created during init and runs
    .fn_destroy = ex_destroy,

    /// parse an io handle into an ast
    .fn_parse = ex_parse,

    /// register available passes
    .fn_compile_passes = {
        /// forward symbols to opt in to order independant lookup
        /// across languages
        [eStageForwardSymbols] = ex_forward_symbols,

        /// process imports for a translation unit
        [eStageCompileImports] = ex_compile_imports,

        /// compile any types that need early compilation
        [eStageCompileTypes] = ex_compile_types,

        /// finish compiling symbols into complete trees
        [eStageCompileSymbols] = ex_compile_symbols
    }
};
```

## Build file requirements

to be detected by interfaces each build file must add itself to `langs`

```py
# meson.build
src = [ 'src/main.c' ]

libexample = library('example', src,
    dependencies : [ broker, interop, tree ],
    c_args : user_args,
    include_directories : [ 'src' ]
)

langs += {
    'example': {
        'dep': declare_dependency(link_with : libexample), # dependency object
        'mod': 'kExampleModule', # module definition exported from the object

        # an optional fuzz corpus to opt in to fuzzing tools
        'fuzz_corpus': meson.current_source_dir() / 'fuzz_corpus'
    }
}
```
