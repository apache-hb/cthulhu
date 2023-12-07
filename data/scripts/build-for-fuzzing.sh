export CC=afl-clang-lto
export CXX=afl-clang-lto++

meson setup build-fuzz -Dbuildtype=release -Db_lto=true
ninja -C build-fuzz
