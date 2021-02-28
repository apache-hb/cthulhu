# Builtin types

## Unsigned types
* sized: `u8`, `u16`, `u32`, `u64`, `usize`
* unsized: `uchar`, `ushort`, `uint`, `ulong`

* `u8` will always be 1 byte wide
* `u16` will always be 2 bytes wide
* `u32` will always be 4 bytes wide
* `u64` will always be 8 bytes wide
* `usize` will be the largest size the largest integer for the target platform

## Signed types
* sized: `i8`, `i16`, `i32`, `i64`, `ssize`
* unsized: `char`, `short`, `int`, `long`

* `i8` will always be 1 byte wide
* `i16` will always be 2 bytes wide
* `i32` will always be 4 bytes wide
* `i64` will always be 8 bytes wide
* `ssize` will be the largest size the largest integer for the target platform


## Other types
* `void`, `bool`

* `void` does not have a size
* `bool` does not have a defined size and may be `true` or `false`

## Arrays
`[type]` is an array of N `type` where `N >= 0`
`[type:expr]` is an array of `expr` items of `type` where `expr` is a constant expression that evaulates to `>0`

## Pointers
`*type` is a pointer to `null` or `N` elements of `type` where `N>0`

## Closures
`type(types)` is a pointer to a function that returns `type` and takes `types` as arguments
