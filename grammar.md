dotted-name: ident [`::` dotted-name]

struct-body-decl: ident `:` type-decl [`,` struct-body-decl]

struct-decl: `{` [struct-body-decl] `}`

tuple-body-decl: type-decl [`,` tuple-body-decl]

tuple-decl: `(` [tuple-body-decl] `)`

array-decl: `[` type-decl `:` expr `]`

typename-decl: dotted-name

type-decl: struct-decl | tuple-decl | typename-decl | array-decl [`*`]

typedef-decl: `using` ident `=` type-decl

body-decl: func-decl | typedef-decl

body-decls: body-decl [body-decls]

import-decl: `import` dotted-name

import-decls: import-decl [import-decls]

module-decl: `module` dotted-name

toplevel-decl: [module-decl] [import-decls] [body-decls]