# Cthulhu

Probably better than javascript

## Building
* building requires `meson`, `ninja`, `gmp` and a C99 compliant compiler
* building also requires `flex` and `bison`
    - windows users can use `win_flex` and `win_bison` from choco
* building on windows requires `wsl` to be installed

### Linux steps

```sh
meson build
ninja -C build

# running tests
ninja -C build test

# using the compiler
./build/ctc --help
```

### Windows steps

```sh
# inside wsl

# gcc-11 is required to build gmp without hitting compiler bugs
sudo add-apt-repository 'deb http://mirrors.kernel.org/ubuntu hirsute main universe'

sudo apt update
sudo apt upgrade
sudo apt install build-essential lzip gcc-11 -y

# download gmp and place it in gmp/
wget https://gmplib.org/download/gmp/gmp-6.2.1.tar.lz -O gmp.tar.lz
mkdir gmp
tar -xf gmp.tar.jz -C gmp --strip-components 1

# the build library and headers will be placed here
export PREFIX=$(pwd)/libgmp

cd gmp
./configure \
  --prefix=$PREFIX/mingw \
  --build=x86_64-w64-mingw32 \
  --host=x86_64-w64-mingw32 \
  --disable-shared \
  --enable-static \
  CC=gcc-11

make -j$(nproc)
make check
make install
```

## Overview

* lex + parse with flex and bison
* symbol resolution, typechecking in semantic analysis
* convert ast into typed ast for IR
* convert typed ast into SSA
* optimize SSA form
* use backend to produce end library/executable from SSA 
  * assembler backend (TODO)

Each stage will try and continue if it encounters any errors but will exit at the end of the stage.
Stages after semantic analysis can expect that input will always be correct.

## Tooling

* syntax highlighting for vscode is available [here](https://github.com/apache-hb/ctu-vscode)

## Licensing

This project is licensed under [AGPL3](./LICENSE).
