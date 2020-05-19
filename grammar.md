include: `include` ident (`::` ident)*

linebreak: `\n` | `;`

struct-field: ident `:` type linebreak
struct: `{` struct-field* `}`

union-field: ident `:` type linebreak
union: `union` `{` union-field* `}`

enum-field: ident `:=` expr linebreak
enum: `enum` (`:` type)? `{` enum-field* `}`

variant-field: ident (`:` expr)? `=>` type
variant: `variant` (`:` type)? `{` variant-field* `}`

type-list: type (`,` type)*
funcptr: `def` `(` type-list* `)` `->` type

name: `ident` (`::` ident)*

type: struct | union | variant | enum | funcptr | name

typedef: `type` ident `:=` type linebreak

arg-list-val: ident `:` type `:=` expr (`,` arg-list-val)
arg-list: ident `:` type (`,` arg-list) | arg-list-val
func-args: `(` arg-list `)`

func-return: `->` type


struct-init: `{` `}`

array-init-body: expr (`,` expr)* 
array-init: `[` array-init-body `]`

struct-init-body: expr (`,` expr)*
named-init-body: ident `:=` expr (`,` ident `:=` expr)*
struct-init: `{` (struct-init-body | named-init-body)? `}`

const-expr: int-expr | char-expr | str-expr | float-expr
expr: const-expr | struct-init | array-init
stmt: expr

func-body: `:=` expr | `{` stmt* `}`

funcdef: `def` ident func-args? func-return? func-body linebreak

body: typedef | funcdef

program: include* body*
