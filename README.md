# Cthulhu
Probably better than javascript

### Building
```sh
meson build
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