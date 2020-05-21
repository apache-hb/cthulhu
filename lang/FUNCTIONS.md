# Functions

```md

name-type-list: ident `:` type (`,` ident `:` type)*
func-args: `(` name-type-list? `)`
func-return: `->` type

func-body: `:=` expr | `{` stmt* `}`

funcdef: `def` func-args? func-return? func-body
```