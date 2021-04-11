# Cthulhu

Probably better than javascript

## Language specification

<https://apache-hb.github.io/ctulang/>

## Building

Build `ctc` into `build/tools/ctc`

Run with `ctc file.ct`

### MSVC

```sh
meson build -Dcpp_std=c++latest
ninja -C build
```

### Clang++ & G++

```sh
meson build -Dcpp_std=c++2a
ninja -C build
```

### Testing

```sh
# plain testing
ninja -C build test
# with valgrind 
meson test --wrap='valgrind -q --leak-check=full --error-exitcode=1'
```

## Source

- cthulhu
  - core cthulhu compiler
- docs
  - language specification and documentation

## Licensing

This project is licensed under [AGPL3](./LICENSE).