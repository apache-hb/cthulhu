# Grammar

ident-list: ident (`,` ident)*

path: ident (`::` ident)*



import-args: `...` | ident-list
import-decl: `import` path `(` import-args `)` `;`


packed-attrib: `@` `packed` (`(` expr`)`)?
align-attrib: `@` `align` `(` expr `)`
attrib: packed-attrib | align-attrib | flags-attrib

builtin:
    `u8` | `u16` | `u32` | `u64` |
    `i8` | `i16` | `i32` | `i64` |
    `f32` | `f64` | `void` |
    `isize` | `usize` |
    `c8` | `c16` | `c32` |
    `int` | `uint`

pointer: type `*`

const: `const` `(` type `)`

array: type `[` expr `]`

typename: path

struct-attrib: packed-attrib | align-attrib
struct-body: ident `:` type `;`
struct-decl: `struct` `{` struct-body* `}`
struct: attrib* struct-decl

enum-field: ident `:=` expr `,`
enum-decl: `enum` (`:` type)? `{` enum-field* `}`
enum: attrib* enum-decl

union-field: ident `:` type `;`
union-decl: `union` `{` union-field* `}`
union: attrib* union-decl

funcptr-args: type (`,` type)*
funcptr: attrib* `def` `(` funcptr-args? `)` `->` type

variant-field: ident (`:` expr)? `=>` type `;`
variant: attrib* `variant` (`:` type)? `{` variant-field* `}`

type:
    builtin | pointer | const |
    array | typename | struct |
    enum | union | funcptr |
    variant

type-decl: `type` ident `:=` type `;`


func-args-body: ident `:` type (`,` func-args-body)?
func-args: `(` func-args-body? `)`

func-ret: `->` type

func-body: `:=` expr | stmt-list

func: `def` ident func-args? func-ret? func-body
func-decl: attrib* func

var-decl: `var` ident (`:` type)? `:=` expr `;`

body-decl: type-decl | func-decl | var-decl

subscript: expr `[` expr `]`

deref: `*` expr

ref: `&` expr

access: expr `.` ident

indirect: expr `->` ident

expr: literal | subscript | deref | ref | access | indirect

stmt-list: `{` stmt* `}`

return-stmt: `return` expr `;`

stmt: var-decl | stmt-list | return-stmt

unit: import-decl* body-decl*