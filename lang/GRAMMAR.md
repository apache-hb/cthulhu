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

unary-op: `+` | `-` | `!` | `~` | `*` | `&`
binary-op: 
    `+` | `+=` | `-` | `-=` |
    `*` | `*=` | `/` | `/=` |
    `%` | `%=` | `&` | `&=` |
    `|` | `|=` | `^` | `^=` |
    `<` | `<=` | `>` | `>=` |
    `==` | `!=` | `<<` | `<<=` |
    `>>` | `>>=` | `||` | `&&`

expr-list: expr (`,` expr)*

struct-init-body: ident `:=` expr (`,` ident `:=` expr)*

unary-expr: unary-op expr
paren-expr: `(` expr `)`
subscript-expr: expr `[` expr `]`
cast-expr: expr `as` type
deref-expr: expr `->` ident
access-expr: expr `.` ident
scope-expr: ident `::` expr
binary-expr: expr binary-op expr
ternary-expr: expr `?` expr `:` expr
call-expr: expr `(` expr-list? `)`
lambda-expr: `(` name-type-list? `)` func-return? func-body
array-init-expr: `[` expr-list? `]`
struct-init-expr: `{` expr-list? `}` | `{` struct-init-body? `}`

expr: 
    unary-expr | paren-expr | subscript-expr | 
    cast-expr | deref-expr | access-expr | 
    scope-expr | binary-expr | ternary-expr | 
    call-expr | lambda-expr | array-init-expr | 
    struct-init-expr

var-name: ident (`:` type)
var-name-list: var-name (`,` var-name)*

let-body: var-name | `(` var-name-list `)`
let-stmt: `let` let-body `:=` expr

assign-stmt: expr `:=` expr
return-stmt: `return` expr
branch-stmt: `if` expr stmt (`else` `if` expr stmt)* (`else` stmt)?

for-stmt: `for` ident `:` expr stmt
while-stmt: `while` expr stmt

break-stmt: `break`
continue-stmt: `continue`

stmt-body: (let-stmt | assign-stmt | return-stmt | branch-stmt | expr | for-stmt | while-stmt | break-stmt | continue-stmt) `;`

stmt: `{` stmt-body* `}`



func-body: `:=` expr | stmt
func-args: `(` name-type-list? `)`
func-return: `->` type

funcdef: `def` ident func-args? func-return? func-body

body: funcdef | typedef

program: include* body*
