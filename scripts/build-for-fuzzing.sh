export CC=afl-clang-fast
export CXX=afl-clang-fast++

meson build-fuzz
ninja -C build-fuzz