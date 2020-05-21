path: ident (`::` ident)*
type-list: type (`,` type)*
name-type-list: ident `:` type (`,` ident `:` type)*

include: `include` path

const: `const` `<` type `>`
array: type `[` expr `]`
pointer: type `*`
reference: type `&`
closure: `(` type-list? `)` `->` type

struct-field: ident `:` type
struct: `struct` `{` struct-field* `}`

union-field: ident `:` type
union: `union` `{` union-field* `}`

enum-field: ident `:=` expr
enum: `enum` (`:` type)? `{` enum-field* `}`

any-field: ident `=>` type
any: `any` `{` any-field* `}`

type: const | array | pointer | reference | struct | union | enum | any | path | closure

typedef: `type` ident `:=` type

func-args: `(` name-type-list? `)`
func-return: `->` type

cast-expr: expr `as` type

paren-expr: `(` expr `)`

unary-op: `~` | `!` | `+` | `-`
unary-expr: unary-op expr

expr: cast-expr | paren-expr | unary-expr

let-stmt: `let` ident `:=` expr `;` | `let` ident `:` type `;` | `let` ident `:` type `:=` expr `;`
return-stmt: `return` expr `;`

stmt: expr | return-stmt | let-stmt

func-body: `:=` expr | `{` stmt* `}`

funcdef: `def` ident func-args? func-return? func-body

body: funcdef | typedef

program: include* body*
