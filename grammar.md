dotted-name: ident [`::` dotted-name]

struct-body: ident `:` typedecl [`,` struct-body]
struct-decl: `{` [struct-body] `}`

tuple-body: typedecl [`,` tuple-body]
tuple-decl: `(` [tuple-body] `)`

union-decl: `union` struct-decl

variant-body: ident `=>` typedecl [`,` variant-body]
variant-decl: `variant` `{` [variant-body] `}`

enum-body: ident `:=` expr [`,` enum-body]
enum-decl: `enum` `{` [enum-body] `}`

array-decl: `[` typedecl `:` expr `]`

ptr-decl: `*` typedecl

funcsig-decl: `&(` [tuple-body] `)` `->` typedecl

builtin-decl: `u8` | `u16` | `u32` | `u64` |
              `i8` | `i16` | `i32` | `i64` |
              `c8` | `bool` | `void`

typename-decl: dotted-name

typedecl: struct-decl  | 
          tuple-decl   |
          union-decl   |
          varaint-decl |
          enum-decl    |
          array-decl   |
          ptr-decl     |
          funcsig-decl |
          builtin-decl |
          typename-decl

typedef: `type` ident `=` typedecl

assign-stmt: expr `:=` expr


match-body: expr `->` expr [`,` match-body]
match-else: `else` `->` expr
match-stmt: `match` expr `{` [match-body] [match-else] `}`

while-stmt: `while` expr stmt

elif-stmt: `else` `if` expr stmt [elif-stmt]
else-stmt: `else` stmt
if-stmt: `if` expr stmt [elif-stmt] [else-stmt]

for-stmt: `for` ident `:` expr stmt

stmt: expr | if-stmt | match-stmt | for-stmt | while-stmt | assign-stmt | `{` [stmt+] `}`

func-body: `{` [stmt+] `}` | `=` expr

funcdef: `def` ident `(` [func-args] `)` `->` typedecl func-body

body: typedef | funcdef

preamble: 

file: [preamble] [body+]