# Cthulhu
Probably better than javascript

## Building

### Windows
```sh
meson build -Dcpp_std=c++latest
ninja -C build
```

### Clang++
```sh
meson build -Dcpp_std=c++2a
ninja -C build
```

### G++
```sh
meson build -Dcpp_std=c++2a
ninja -C build
```

### Testing
simple testing
```sh
ninja test
```
with valgrind 
```
meson test --wrap='valgrind -q --leak-check=full --error-exitcode=1'
```

## Source
- cthulhu
  - reduced cthulhu compiler written in C++
- test
  - unit tests for the C++ compiler
- lang
  - language specification
- tools
  - tools for debugging the C++ compiler and language introspection
