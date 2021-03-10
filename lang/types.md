# Builtin types

## Scalar types

### Unsigned types
* `uchar`, `ushort`, `uint`, `ulong`, `usize`

* `uchar` will always be large enough to at least represent the range `0 to 255`
* `ushort` will always be large enough to at least represent the range `0 to 65,535`
* `uint` will always be large enough to at least represent the range `0 to 4,294,967,295`
* `ulong` will always be large enough to at least represent the range `0 to 18,446,744,073,709,551,615`
* `usize` will be the largest size the largest integer for the target platform

### Signed types
* `char`, `short`, `int`, `long`, `ssize`

* `char` will always be large enough to at least represent the range `-128 to 127`
* `short` will always be large enough to at least represent the range `-32,768 to 32,767`
* `int` will always be large enough to at least represent the range `-2,147,483,648 to 2,147,483,647`
* `long` will always be large enough to at least represent the range `-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807`
* `ssize` will be the largest size the largest integer for the target platform

## Other types
* `void`, `bool`

* `void` does not have a size
* `bool` does not have a defined size and may be `true` or `false`
* expressions may implicity evaluation to `bool` if they result in a scalar type
  * `0` will evauluate to `false`
  * anything else will evaluate to `true`

### Arrays
`[type]` is an array of N `type` where `N >= 0`
`[type:expr]` is an array of `expr` items of `type` where `expr` is a constant expression that evaulates to `>0`

### Pointers
`*type` is a pointer to `null` or `N` elements of `type` where `N>0`

### Closures
`(types) -> type` is a pointer to a function that returns `type` and takes `types` as arguments

## Strings
* `utf` is a type that will always be large enough to represent a unicode code point
* `str` is an immutable sequence of valid utf code points encoded as utf8
