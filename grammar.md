dotted-name-decl: ident [`::` dotted-name-decl]


plain-enum-body: ident [`,` plain-enum-body]

plain-enum-decl: `(` plain-enum-body `)`

valued-enum-body: ident `:` expr [`,` valued-enum-body]

valued-enum-decl: `{` valued-enum-body `}`

enum-decl: `enum` (plain-enum-decl | valued-enum-decl)


alias-decl: dotted-name-decl


variant-body-decl: ident `->` type-decl [`,` variant-body-decl]

variant-decl: `variant` `{` variant-body-decl `}`


struct-body-decl: ident `:` type-decl [`,` struct-body-decl]

struct-decl: `{` [struct-body-decl] `}`


array-decl: `[` type-decl `:` expr `]`


ptr-decl: type-decl `*`

type-decl: struct-decl  |
           variant-decl |
           enum-decl    |
           alias-decl   |
           array-decl   |
           ptr-decl     |
           attribute-decl type-decl

using-decl: `using` ident `=` type-decl

in-expr: expr `in` expr

is-expr: expr `is` type-decl

as-expr: expr `as` type-decl

boolean: `true` | `false`

null: `null`

array-body: expr [`,` array-body]

array: `[` [array-body] `]`

const-expr: float   | 
            integer | 
            string  | 
            boolean | 
            null    |
            array

else-expr: `else` func-body

elif-expr: `else` `if` expr func-body [elif-expr]

branch-expr: `if` expr func-body [`elif-expr`] [`else-expr`]

default-expr: `else` `->` func-body

case-expr: expr `->` func-body

switch-expr: `switch` expr [case-expr+] [default-expr]

unary-expr: unary-op expr

binary-expr: expr binary-op expr

access-expr: expr [`.` ident]

ptr-access-expr: expr [`->` ident]

name-expr: ident

expr: const-expr      |
      branch-expr     |
      switch-expr     |
      as-expr         |
      is-expr         |
      in-expr         |
      unary-expr      |
      binary-expr     |
      access-expr     |
      ptr-access-expr |
      name-expr       |
      `(` expr `)`

while-stmt: `while` expr func-body

for-stmt: `for` ident `in` expr func-body

let-stmt: `let` ident `:` type-decl [`=` expr] |
          `let` ident `=` expr

var-stmt: `var` ident `:` type-decl [`=` expr] |
          `var` ident `=` expr

return-stmt: `return` expr

break-stmt: `break`
continue-stmt: `continue`

stmt: while-stmt    |
      for-stmt      |
      let-stmt      |
      var-stmt      |
      return-stmt   |
      break-stmt    |
      continue-stmt |
      attribute-decl stmt

func-body: expr | 
           stmt | 
           `{` [func-body+] `}`

func-args-default-decl: ident `:` type-decl `=` expr [`,` func-args-default-args]

func-args-decl: ident `:` type-decl [`,` (func-args-decl | func-args-default-decl)]

func-decl: `def` ident `(` [func-args-decl] `)` `->` type-decl func-body

attribute-call-args-named-decl: ident `=` expr [`,` attribute-call-args-named-decl]

attribute-call-args-decl: expr [`,` (attribute-call-args-decl | attribute-call-args-named-decl)]

attribute-args-decl: `(` [attribute-call-args-decl] `)`

attribute-body-decl: dotted-name-decl [attribute-args-decl]

attribute-decl: `[[` attribute-body-decl `]]`

body-decl: using-decl |
           func-decl  |
           attribute-decl func-decl

import-decl: `import` dotted-name-decl

module-decl: `module` dotted-name-decl
ast: [module-decl] [import-decl+] [body-decl+]