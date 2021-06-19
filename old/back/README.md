# arch specific code gen

targets are formatted as 

`backend-arch-fmt-type`

## backends
* `cthulhu` native cthulhu backend
* `gcc` gcc toolchain backend
* `llvm` llvm toolchain backend
* `c89` bring your own C89 compiler
  * inline asm is disabled on this target

## arches

all target archs

### x86 arches
* `i8086` x86 16 bit backend
* `x86` x86 32 bit backend
* `x64` x86 64 bit backend

## formats
* `elf` executable link format for linux
* `coff` common object file format for windows

## types
* `exec` executable program
* `shared` shared object for dynamic linking
* `static` library for static linking

## currently supported targets

* `cthulhu-(i8086|x86|x64)-(elf|coff)-(exec|shared|static)`
* `gcc-(x86|x64)-elf-(exec|shared|static)`
* `llvm-(x86|x64)-elf-(exec|shared|static)`
* `c89-any-any-any` these will depend on what your compiler supports

# how a backend works
* a backend is supplied with a valid ir object
* its up to the backend how to do regalloc and liveness
* the frontend gets given a blob by the backend
* the frontend then goes and gives that blob to a formatter
* the formatter emits a final file