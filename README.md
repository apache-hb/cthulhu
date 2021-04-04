# Cthulhu

Probably better than javascript

## Language specification

<https://apache-hb.github.io/ctulang/>

## Building

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
# plain texting
ninja test
# with valgrind 
meson test --wrap='valgrind -q --leak-check=full --error-exitcode=1'
```

## Source

- cthulhu
  - core cthulhu compiler
- docs
  - language specification and documentation
