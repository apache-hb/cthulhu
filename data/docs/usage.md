# Usage {#usage}

The cthulhu framework has 3 tiers of integration.

## Tier 3
Only uses the @ref common module, in this form cthulhu can be used via a meson subproject or pkgconfig
via `cthulhu-base`

## Tier 2
Tier 2 has access to @ref common, @ref runtime excluding @ref mediator, and @ref support excluding @ref langs.
In this for cthulhu can be consumed as either a meson subproject or via pkgconfig as `cthulhu-runtime`.
Note that `cthulhu-runtime` requires `cthulhu-base`.

## Tier 1
Tier 1 requires wholesale buyin to meson as a build system, but allows access to the full framework.
This includes language driver registration as well as diagnostic discoverability.
* Once support for consuming language drivers as shared libraries is stable it will also be possible to have tier 1 language drivers without using meson.

Installing cthulhu onto your local system can be done via

```sh
# -Dbuildtype=release is not required but is recommended for best performance
meson setup build -Dbuildtype=release -Dprefix=<your_install_dir>
meson install -C build
```

Installed files will be in the specified prefix, note that by default these are no relocatable.
More about this at the [meson docs](https://mesonbuild.com/Builtin-options.html#pkgconfig-module).
