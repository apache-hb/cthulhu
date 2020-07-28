# Cthulhu

```sh
meson build
ninja -C build
./build/cti
```

REPL supports expressions and thats about it

## Adding builtins
1. update `cthulhu/keys.inc` to add a custom flag.
2. update `cthulhu/keys.inc` to add keywords related to that flag.