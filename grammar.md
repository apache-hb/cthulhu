## type grammar

struct-body = ident `:` type [`,` struct-body]

struct = `{` [struct-body] `}`


tuple-body = type [`,` tuple-body]

tuple = `(` [tuple-body] `)`


union = `union` struct


variant-body = ident `=>` type [`,` variant-body]

variant = `variant` [`:` type] `{` [variant-body] `}`


enum-body = ident `:=` expr [`,` enum-body]

enum = `enum` [`:` type] `{` [enum-body] `}`


ptr = `*` type


array = `[` type `:` expr `]`


typename = ident

builtin = `u8`   |
          `u16`  |
          `u32`  |
          `u64`  |
          `u128` |
          `i8`   |
          `i16`  |
          `i32`  |
          `i64`  |
          `i128` |
          `f32`  |
          `f64`  |
          `c8`   |
          `c16`  |
          `c32`  |
          `uint` |
          `int`  |
          `bool` |
          `void` |

type = struct | tuple | union | variant | enum | ptr | array | typename | builtin

typedef = `type` ident `=` type


global = `let` ident [`:` type] `=` expr

bin-op = `!` | `~` | `-` | `+`

unary-op = todo

dotted-name = ident [`::` dotted-name]

access = ident [`:` dotted-name]

expr = binary | unary | ternary | dotted-name | access

binary = bin-op expr

unary = expr unary-op expr

ternary = expr `?` expr `:` expr

var = `var` ident `:` type [`=` expr] |
      `var` ident `=` expr

let = `let` ident `:` type [`=` expr] |
      `let` ident `=` expr

return = `return` expr

stmt = expr | let | var | return

func-body-decl = `{` [stmt+] `}` | `=` expr

func = `def` ident `(` [func-args] `)` `->` type func-body-decl

body = typedef | func | global

ast = [body+]