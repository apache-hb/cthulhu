path: ident (`::` ident)*

include: `include` path (`->` ident)?

type-list: type (`,` type)*

const: `const` `<` type `>`
pointer: type `*`
reference: type `&`
array: type `[` expr `]`
funcptr: `def` `(` type-list? `)` `->` type

struct-field: ident `:` type
struct: `struct` `{` struct-field* `}`

union-field: ident `:` type
union: `union` `{` union-field* `}`

enum-field: ident `:=` expr
enum: `enum` (`:` type)? `{` enum-field* `}`

any-field: ident `:` type
any: `any` `{` any-field* `}`

type: const | pointer | reference | array | path | struct | union | enum | any | funcptr

typedef: `type` ident `:=` type

arg-val-list: ident `:` type `:=` expr (`,` arg-val-list)?
arg-list: ident `:` type (`,` arg-list)? | arg-val-list
func-args: `(` arg-list? `)`

func-return: `->` type


branch-stmt: `if` expr stmt (`else` `if` expr stmt)* (`else` stmt)?

var-stmt: `var` ident `:` type (`:=` expr)? |
          `var` ident `:=` expr

for-stmt: `for` ident `:` expr stmt

while-stmt: `while` expr stmt

stmt: expr | `{` stmt* `}` | var-stmt | branch-stmt | for-stmt | while-stmt

func-body: `{` stmt* `}` | `:=` expr

funcdef: `def` ident func-args? func-return? func-body

body: typedef | funcdef

program: include* body*
