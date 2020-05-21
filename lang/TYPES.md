# Types

```md
type: struct | enum | union | any | const | array | pointer | reference | closure
```

## Structs

```md
struct-field: ident `:` type
struct: `struct` `{` struct-field* `}`
```

## Enums

```md
enum-field: ident `:=` expr
enum: `enum` `{` enum-field* `}`
```

## Union

```md
union-field: ident `:` type
union: `union` `{` union-field* `}`
```

## Any

```md
any-field: ident `=>` type
any: `any` `{` any-field* `}`
```

## Const

```md
const: `const` `<` type `>`
```

## Pointer

```md
pointer: type `*`
```

## Reference

```md
reference: type `&`
```

## Function Pointer

```md
closure: `(` type-list? `)` `->` type
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
* bool
* f32
* f64