# Builtin types

## Scalar types

### Unsigned types
* `uchar`, `ushort`, `uint`, `ulong`, `usize`

* `uchar` must be large enough to at least represent the range `0 to 255`
* `ushort` must be large enough to at least represent the range `0 to 65,535`
* `uint` must be large enough to at least represent the range `0 to 4,294,967,295`
* `ulong` must be large enough to at least represent the range `0 to 18,446,744,073,709,551,615`
* `usize` must be the largest unsigned native integer size the target platform supports

### Signed types
* `char`, `short`, `int`, `long`, `ssize`

* `char` must be large enough to at least represent the range `-128 to 127`
* `short` must be large enough to at least represent the range `-32,768 to 32,767`
* `int` must be large enough to at least represent the range `-2,147,483,648 to 2,147,483,647`
* `long` must be large enough to at least represent the range `-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807`
* `ssize` must be the largest signed native integer size the target platform supports

## Other types
* `void`, `bool`

* `void` does not have a size
* `void` may not be assigned to a variable
* `bool` does not have a defined size and may be `true` or `false`
* expressions may implicity evaluation to `bool` if they result in an integer type
  * `0` evaluates to `false`
  * `true` is any value that does not evaluate to `false`
  * `null` pointers evaluate to `false`, all other pointers evaluate to `true`

### Arrays
`[type]` is an unbounded array of N `type` where `N >= 0`
`[type:expr]` is an array of `expr` items of `type` where `expr` is a constant expression that evaulates to `>0`

### Pointers
`*type` is a pointer to `null` or `N` elements of `type` where `N>0`

### Closures
`(types) -> type` is a pointer to a function that returns `type` and takes `types` as arguments

## Strings
* `utf` is a type that will always be large enough to represent any valid unicode code point
* `str` is an immutable sequence of valid unicode code points encoded as utf8

## User defined types

### Records

`record` types are sets of named types
* fields in a record may be reordered
* only the last field in a record may be an unbounded array
* the size of a record may be larger than the sum of its field sizes

### Unions

`union` types are sets of named types with one active field at a time
* setting a field invalidates the contents of all other fields
* reading from an unset field is undefined behaviour
* the size of a `union` is will be the size of the largest field
* any field in a union may be an unbounded array

### Variants

`variant` types are enumerations with optional associated data
* setting a field invalidates the contents of all other fields
* reading from a field must be checked to prevent undefined behaviour
* the size of a variant may be larger than the largest field
