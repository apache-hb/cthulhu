# Grammar

unit : include* decl* EOF 

## Includes

include : using path include-items? `;` 

include-items : `(` include-list `)`

include-list : `...` | list

## Declarations

decl : assert-decl

assert-decl : assert (ident | string) `:` expr `;`

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

postfix-expr : primary-expr | postfix-expr `[` expr `]` | postfix-expr `(` function-args? `)` | postfix-expr `.` ident | postfix-expr `->` ident

coerce-expr : coerce `!<` type `>` `(` expr `)`

function-args : function-arg (`,` function-args) | named-function-args

function-arg : expr

named-function-args : named-function-arg (`,` named-function-arg)*

named-function-arg : `.` ident `=` expr

## Types

type : pointer | closure | array | qualified

pointer : `*` type

closure : `(` types? `)` `->` type

array : `[` type (`:` expr)? `]`

qualified : name (`::` name)*

name : ident (`!<` types `>`)?

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

using : `using`

coerce : `coerce`

assert : `assert`
