# Usage {#usage}

The cthulhu framework has 3 tiers of integration.

## Tier 3
Only uses the @ref common module, in this form cthulhu can be used via a meson subproject or pkgconfig
via `cthulhu-common`

## Tier 2
Tier 2 has access to @ref common, @ref runtime, and @ref support.
In this form cthulhu can be consumed as either a meson subproject or via pkgconfig as `cthulhu-runtime` + `cthulhu-support`. These dependencies require `cthulhu-common`.

## Tier 1

### Dynamic loading

If a frontend supports dynamic loading of modules you can provide an out of tree module to it and it can load it. This requires cthulhu to be built and consumed as a shared library, static libraries cause duplication of global state, which causes issues with allocators.

For a module to be consumed by the loader it must provide at least one of the following symbols:
* `const language_t *lang_main(void)`
* `const plugin_t *plugin_main(void)`
* `const target_t *target_main(void)`

Refer to @ref driver, and @ref broker. for more information about the exact details of these types.

### Static loading
Static loading requires writing an in-tree module and building + registering it through meson.

## Installation

Installing cthulhu onto your local system can be performed with meson

```sh
meson setup build
sudo meson install -C build
```

Note that by default these are no relocatable.
More about this at the [meson docs](https://mesonbuild.com/Builtin-options.html#pkgconfig-module).
