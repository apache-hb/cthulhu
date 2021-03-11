# Grammar

unit : include* decl* EOF 

## Includes

include : using path include-items? `;` 

include-items : `(` include-list `)`

include-list : `...` | list

## Declarations

decl : decorated-decl | alias-decl | record-decl | variant-decl | union-decl

## Aliases

alias-decl : using ident `=` type `;`

## Records

record-decl : record ident `{` record-field* `}`

record-field : basic-field `;`

## Unions

union-decl : union ident `{` union-field* `}`

union-field : basic-field `;`

## Variants

variant-decl : variant ident (`:` qualified)? `{` variant-case* `}`

variant-case : case ident variant-case-body? `;`

variant-case-body : `(` variant-field (`,` variant-field)* `)`

variant-field : basic-field

## Generic fields

basic-field : ident `:` type bitrange?

<!-- TODO: bitrange syntax needs work -->

bitrange : `[` expr `..` expr `]`

## Statements

stmt : assign-stmt | branch-stmt | compound-stmt | while-stmt

while-stmt : while `(` condition `)` stmt else-stmt?

assign-stmt : expr `=` expr `;`

compound-stmt : `{` stmt* `}`

branch-stmt : if-stmt elif-stmt* else-stmt?

if-stmt : if `(` condition `)` stmt

elif-stmt : else if `(` condition `)` stmt

else-stmt : else stmt
 
condition : expr | var-decl-body (`;` expr)?

## Attributes

decorated-decl : attribute decl

attribute : `@` attribute-body

attribute-body : `[` attribute-items `]` | attribute-item

attribute-items : attribute-item (`,` attribute-item)*

attribute-item : path attribute-args?

attribute-args : `(` function-args `)`

## Expressions

expr : ternary-expr

ternary-expr : logic-expr (`?` ternary-expr `:` ternary-expr)*

logic-expr : equality-expr ((`&&` | `||`) logic-expr)*

equality-expr : compare-expr ((`==` | `!=`) equality-expr)*

compare-expr : bitwise-expr ((`<` | `<=` | `>` | `>=`) compare-expr)*

bitwise-expr : bitshift-expr ((`^` | `&` | `|`) bitwise-expr)*

bitshift-expr : math-expr ((`<<` | `>>`) bitshift-expr)*

math-expr : mul-expr ((`+` | `-`) math-expr)*

mul-expr : unary-expr ((`*` | `/` | `%`) mul-expr)*

unary-expr : (`+` | `-` | `~` | `!` | `&` | `*`)? postfix-expr

primary-expr : `(` expr `)` | int | char | string | coerce-expr

postfix-expr : primary-expr | postfix-expr `[` expr `]` | postfix-expr `(` function-args? `)` | postfix-expr `.` ident | postfix-expr `->` ident | primary-expr

coerce-expr : coerce `!<` type `>` `(` expr `)`

function-args : function-arg (`,` function-args) | named-function-args

function-arg : expr

named-function-args : named-function-arg (`,` named-function-arg)*

named-function-arg : `.` ident `=` expr

## Types

type : pointer | closure | array | qualified | mutable

mutable : var type

pointer : `*` type

closure : `(` types? `)` `->` type

array : `[` type (`:` expr)? `]`

qualified : name (`::` name)*

name : ident

types : type (`,` type)* 

## Basic

ident : [a-zA-Z_][a-zA-Z0-9_]* 

int : base2 | base10 | base16

base2 : `0b` [01]+

base10 : [0-9]+

base16 : `0x` [0-9a-fA-F]+

char : `'` letter `'`

string : `"` letter* `"`

letter : `\` ['"ntv0\\] | ~[\\\r\n]

## Keywords

var : `var`

using : `using`

coerce : `coerce`

record : `record`

union : `union`

variant : `variant`

trait : `trait`

extend : `extend`

with : `with`

requires : `requires`

case : `case`

if : `if`

else : `else`

while : `while`
