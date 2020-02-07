# basic constructs
dotted-name: ident [`::` dotted-name]

name-list: ident [`,` name-list]

name-val-list: ident `:` expr [`,` name-val-list]

backing: `:` type

# enums

name-enum-body: `(` [name-list] `)`

val-enum-body: `{` [name-val-list] `}`

enum-body: val-enum-body | [backing] name-enum-body

enum: `enum` enum-body


# unions

name-val-type-list: ident `:` expr `->` type [`,` name-val-type-list]

name-type-union-list: ident `->` type [`,` name-type-union-list]

typesafe-union-body: `{` [name-type-union-list] `}`

type-val-safe-union-body: `{` [name-val-type-list] `}`

typesafe-union: `enum` (typesafe-union-body | [backing] type-val-safe-union-body)

union-body: tuple | struct

union: `union` (typesafe-union | union-body)


array: `[` type [`:` expr] `]`


tuple: `(` [type-list] `)`

struct: `{` [name-type-list] `}`

typename: dotted-name

signature: `&(` [type-list] `)` `->` type

# functions

func-name: dotted-name

named-call-args: ident `=` expr [`,` named-call-args]

call-args: expr [`,` (call-args | named-call-args)]

func-call: `(` [call-args] `)`

unary-op: `&` | `*` | `-` | `+` | `!` | `~`

bitwise-op: (`|` | `&` | `^` | `<<` | `>>`) [`=`]

logic-op: (`&&` | `||` | `==` | `!=` | `<` | `<=` | `>` | `>=`)

math-op: (`+` | `-` | `/` | `*` | `%`) [`=`]

binary-op: bitwise-op | logic-op | math-op

unary-expr: (`&` | `*` | `-` | `+` | `!` | `~`) expr

binary-expr: expr binary-op expr

func-call-expr: func-name func-call


else-expr: `else` `{` func-body `}`

elif-expr: `else` `if` expr `{` func-body `}` [elif-expr | else-expr]

if-expr: `if` expr `{` func-body `}` [elif-expr]


match-else-body: `else` `->` (expr | `{` func-body `}`)

match-body: expr `->` (expr | `{` func-body `}`) [match-body] 

match-expr: `match` expr `{` [match-body] [match-else-body] `}`



expr: unary-expr | binary-expr | `(` expr `)` | if-expr | match-expr | func-call-expr


return-decl: `return` expr

let-decl: `let` ident (backing [`=` expr] | backing)

var-decl: `var` ident (backing [`=` expr] | backing)

while-decl: `while` expr `{` func-body `}`

for-decl: `for` ident `:` expr `{` func-body `}`


decl: return-decl | let-decl | var-decl | while-decl | for-decl | func

func-body: [expr+]

func: `def` ident `(` [name-type-list] `)` `->` type `{` func-body `}`

# top level declarations
type: enum | union | struct | tuple | array | signature | typename [`*`]

using: `using` ident `=` type

scope: `scope` dotted-name `{` body `}`

module: `module` dotted-name

import: `import` dotted-name

body: (using | scope | func)

toplevel: [module] [import+] [body+]
