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
* `str` is an immutable sequence of valid unicode extended grapheme sequences (EGCs)

## User defined types

Ignored fields are all named `$` and cannot be accessed.

### Records

`record` types are sets of named types
* fields in a record may be reordered
* only the last field in a record may be an unbounded array
* the size of a record may be larger than the sum of its field sizes
* a record may have 0 or more ignored fields

### Unions

`union` types are sets of named types with one active field at a time
* setting a field invalidates the contents of all other fields
* reading from an unset field is undefined behaviour
* the size of a `union` is will be the size of the largest field
* any field in a union may be an unbounded array
* unions may not have ignored fields

### Variants

`variant` types are sum types
* setting a field invalidates the contents of all other fields
* reading from a field must be checked to prevent undefined behaviour
* the size of a variant may be larger than the largest field
* variant options must not be ignored, but contained fields may contain ignored fields

## Bitfields

* `record`, `union`, and `variants` fields may all be bitfields
* a bitfield must be 1 or more bits long
* bitfields composed of more than 1 bit or bitranges will be concatonated
* loading and storing to bitfields is platform defined behaviour
* a bitfield may not be larger in bits than its underlying type
* bitfields in `union`s may overlap
* bitfields in `record`s and `variant`s may not overlap
* the size of a bitfield must be determinable at compile time

## Error polymorphic variants

Error types are an anonymous sum type of a result value and a polymorphic variant of all possible error types that a function may return.

`type!` types are equivilent to `type!void` and are either `type` or nothing
`!type` types are equivilent to `void!type` and are either `void` or an error
* an error is a sum type expressed as `ok!err` or `ok!(A / B)`
* an error type will be reduced to its smallest set of types such that `(A / A / B)` is equal to `(A / B)`
* an error types ordering does not affect its equality such that `(A / B)` is equal to `(B / A)`
* an error type will always be flattened such that `((A / B) / C)` is equal to `(A / B / C)`
* the left hand side may not be a polymorphic variant
