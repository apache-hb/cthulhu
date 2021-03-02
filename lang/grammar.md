# Grammar

unit : include* decl* EOF 

## Includes

include : `using` path include-items? `;` 

include-items : `(` include-list `)`

include-list : `...` | list

## Decls

decl : decorator* template? base-decl 

base-decl : alias-decl | struct-decl | union-decl | enum-decl | function-decl | (single-variable-decl `;`)

## Aliases

alias-decl : `using` ident `=` type `;`


## Structs

struct-decl : `struct` ident inherits? struct-body

struct-body : `{` struct-items? `}`

struct-items : struct-item (`,` struct-item)*

struct-item : ident `:` type (`=` expr)?

inherits : single-inherit | `(` inherit-list `)`

inherit-list : inherit (`,` inherit)*

inherit : ident `=` type

single-inherit : `:` type

## Unions

union-decl : `union` ident inherits? union-body

union-body : `{` union-items? `}`

union-items : union-item (`,` union-item)* 

union-item : ident `:` type

## Enums

enum-decl : `enum` (tagged-enum | simple-enum)

tagged-enum : `union` ident single-inherit? tagged-enum-body

simple-enum : ident single-inherit? simple-enum-body

tagged-enum-body : `{` tagged-enum-items? `}` 

tagged-enum-items : tagged-enum-item (`,` tagged-enum-item)*

tagged-enum-item : ident struct-body? (`=` expr)?

simple-enum-body : `{` simple-enum-items `}`

simple-enum-items : simple-enum-item (`,` simple-enum-item)*

simple-enum-item : ident (`=` expr)?

## Functions

function-decl : `def` function-name function-params? function-result? function-body

function-params : `(` function-param-body `)`

function-param-body : function-param (`,` function-param-body) | default-function-params

default-function-params : default-function-param (`,` default-function-param)*

function-param : ident `:` type

default-function-param : ident `:` type `=` expr

function-result : `:` type

function-body : compound | `=` expr `;` | `;`

## Variables

any-variable-decl : variable-start (decomposing-var-name | simple-var-name) var-init?

single-variable-body : variable-start simple-var-name

decomposing-var-decl : variable-start decomposing-var-name

single-variable-decl : variable-start simple-var-name var-init? `;`

variable-start : `var` | `let`

decomposing-var-name : `[` simple-var-name (`,` simple-var-name)* `]`

simple-var-name : ident (`:` type)? 

var-init : `=` expr 

## Decorators

decorator : `@` decorator-list

decorator-list : decorator-body | `[` decorator-body (`,` decorator-body)* `]`

decorator-body : path decorator-args?

decorator-args : `(` function-args `)`


## Templates

template : `template` `!<` template-items `>`

template-items : template-item (`,` template-item)*

template-item : ident template-limits?

template-limits : qualified (`+` qualified)*

## Statements

stmt : compound | return | any-variable-decl `;` | while | branch | with | for | switch | expr `;` | `break` | `continue`

single-stmt : any-variable-decl

compound : `{` stmt* `}`

return : `return` expr? `;`

branch : if else-if* else?

if : `if` condition stmt

else-if : `else` `if` condition stmt

else : `else` stmt

with : `with` condition stmt

while : `while` condition stmt

for : `for` (for-range | for-loop) stmt

for-range : `(` decomposing-var-decl `..` expr `)`

for-loop : `(` any-variable-decl? `;` expr? `;` expr? `)`

switch : `switch` condition switch-body

switch-body : switch-case* switch-else?

switch-case : `case` case-label stmt

switch-else : `else` `:` stmt

case-label : (expr | expr `..` expr) `:`

condition : `(` condition-body `)`

condition-body : single-stmt `;` expr | single-stmt | expr

## Expressions

expr : ternary-expr

ternary-expr : assign-expr (`?` ternary-expr `:` ternary-expr)*

assign-expr : logic-expr ((`=` | `+=` | `-=` | `/=` | `*=` | `%=` | `<<=` `>>=` | `|=` | `&=` | `^=`) assign-expr)* 

logic-expr : equality-expr ((`&&` | `||`) logic-expr)*

equality-expr : compare-expr ((`==` | `!=`) equality-expr)*

compare-expr : bitwise-expr ((`<` | `<=` | `>` | `>=`) compare-expr)*

bitwise-expr : bitshift-expr ((`^` | `&` | `|`) bitwise-expr)*

bitshift-expr : math-expr ((`<<` | `>>`) bitshift-expr)*

math-expr : mul-expr ((`+` | `-`) math-expr)*

mul-expr : unary-expr ((`*` | `/` | `%`) mul-expr)*

unary-expr : (`+` | `-` | `~` | `!` | `&` | `*`)? postfix-expr

primary : `(` expr `)` | int | char | string | coerce

postfix : primary | postfix `[` expr `]` | postfix `(` function-args? `)` | postfix `.` ident | postfix `->` ident

coerce : `coerce` `!<` type `>` `(` expr `)`

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

types : type (`,` type)* 

name : ident (`!<` types `>`)?

## Basic

ident : [a-zA-Z_][a-zA-Z0-9_]* 

int : base2 | base10 | base16

base2 : `0b` [01]+

base10 : [0-9]+

base16 : `0x` [0-9a-fA-F]+

char : `'` letter `'`

string : `"` letter* `"`

letter : `\` ['"ntv0\\] | ~[\\\r\n]
