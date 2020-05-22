# Types

```md
type: struct | enum | union | any | const | array | pointer | reference | closure
```

## Structs

```md
struct-field: ident `:` type
struct: `struct` `{` struct-field* `}`
```

```ct
type example := struct {
    field1: i32
    field2: f32
    field4: u8[4]
}
```

## Enums

```md
enum-field: ident `:=` expr
enum: `enum` (`:` type)? `{` enum-field* `}`
```

```ct
type example1 := enum {
    field1 := 1
    field2 := 2
    field3 := 3
}

type example2 := enum: u8 {
    field1 := 1
    field2 := 2
    field3 := 3
}
```

* Backing type must be one of `u8`, `u16`, `u32`, `u64`, `i8`, `i16`, `i32`, `i64`
* Backing type is `u32` by default

## Union

```md
union-field: ident `:` type
union: `union` `{` union-field* `}`
```

```ct
type example := union {
    number: f64
    integer: i64
    string: char*
}
```

## Any

```md
any-field: ident `=>` type
any: `any` `{` any-field* `}`
```

```ct
type example := any {
    number => f64
    integer => i64
    string => char*
}
```

## Const

```md
const: `const` `<` type `>`
```

```ct
type example := const<i32>
```

* May not be nested. eg `const<const<i32>>` is semantically invalid

## Pointer

```md
pointer: type `*`
```

```ct
type example := i32*
```

## Reference

```md
reference: type `&`
```

```ct
type example := i32&
```

* May not be nested. eg `i32&&` is semantically invalid
* Functionaly identical to a pointer but must be a valid, non-null pointer

## Function Pointer

```md
closure: `(` type-list? `)` `->` type
```

```ct
type example := (i32) -> bool
```

## Array

```md
array: type `[` expr `]`
```

```ct
type example := u8[4]
```

## Builtin types

* u8
* u16
* u32
* u64
* i8
* i16
* i32
* i64
* void
* f32
* f64
