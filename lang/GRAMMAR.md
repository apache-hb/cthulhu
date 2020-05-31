# Grammar

ident-list: ident (`,` ident)*

path: ident (`::` ident)*



import-args: `...` | ident-list

import-decl: `import` path `(` import-args `)` `;`


entry-attrib: `entry` `(` ident `)`
extern-attrib: `extern` `(` ident `)`
section-attrib: `section` `(` string `)`

inline-opt: `(` (`always` | `never`) `)`
inline-attrib: `inline` inline-opt?
attrib: `@` (entry-attrib | extern-attrib | inline-attrib | section-attrib)




unary-op: `-` | `+` | `!` | `~` | `*` | `&`

unary: unary-op expr

binary: expr binary-op expr

closure-args: ident (`,` closure-args)?
closure: `{` closure-args? stmt* `}`

asm-body:
    lots of instructions

asm-args: `(` ident `)`
asm-expr: `@` `asm` asm-args? `{` asm-body* `}`

expr: unary | binary | ident | closure | asm-expr

stmt: return-stmt | stmt-list | if-stmt | for-stmt | while-stmt | switch-stmt | `continue` `;` | `break` `;` | match-stmt

switch-case: `case` expr stmt-list

switch-stmt: `switch` expr `{` switch-case* (`default` stmt-list)? `}`

match-field: expr `=>` stmt-list | expr (`,` expr)* `=>` stmt-list

match-stmt: `match` expr `{` match-field* (`else` `=>` stmt-list) `}`

while-stmt: `while` expr stmt-list

for-stmt: `for` ident `:` expr stmt-list

else-stmt: `else` stmt-list
elif-stmt: `else` `if` expr stmt-list
if-stmt: `if` expr stmt-list elif-stmt* else-stmt?

return-stmt: `return` expr? `;`

stmt-list: `{` stmt* `}`



func-args-list: typename ident (`,` func-args-list)?
func-args: `(` func-args-list? `)`
func-ret: `:` type
func-body: `:=` expr `;` | stmt-list
func-decl: attrib* `def` ident func-args? func-ret? func-body


typename-args: (typename | expr) (`,` typename-args)?
typename: ident | ident `<` typename-args `>` | typename `::` typename

array: simple-type `[` expr? `]`

pointer: simple-type `*`

const: `const` `(` simple-type `)`

builtin:
    `u8` | `u16` | `u32` | `u64` | `usize` | `uint` |
    `i8` | `i16` | `i32` | `i64` | `isize` | `int`  |
    `c8` | `f32` | `f64` | `void`

simple-type: typename | array | pointer | const | builtin


closure-args: simple-type (`,` simple-type)*
closure: simple-type `(` closure-args? `)`

type: simple-type | closure


struct-field: simple-type ident `;`
struct-decl: attrib* `struct` ident `{` struct-field* `}`

alias-decl: `type` ident `:=` type `;`


union-field: simple-type ident `;`
union-decl: attrib* `union` ident `{` union-field* `}`

enum-field: ident `:=` expr `;`
enum-decl: attrib* `enum` ident (`:` simple-type)? `{` enum-field* `}`

object-decl: attrib* `object` ident

body-decl:
    func-decl | struct-decl | alias-decl |
    union-decl | object-decl



unit: import-decl* body-decl*