# cthulhu

## lex

import: `import`
type: `type`
def: `def`

colon: `:`
assign: `:=`
comma: `,`
mul: `*`

lparen: `(`
rparen: `)`

lsquare: `[`
rsquare: `]`

big-arrow: `=>`
arrow: `->`

## parse

name: ident (colon ident)*

import-decl: import name (arrow ident)?





expr
    : TODO
    ;







funcdef-args-default-body
    : ident colon type-decl assign expr (comma funcdef-args-default-body)?
    ;

funcdef-args-body
    : ident colon type-decl (comma funcdef-args-body)? 
    | funcdef-args-default-body
    ;

funcdef-args
    : lparen funcdef-args-body? rparen
    ;

funcdef-decl
    : def ident funcdef-args? funcdef-return? funcdef-body
    ;






type-decl
    : type-attrib* type-decl-body (array-decl | ptr-decl)*
    ;

type-attrib
    : attrib
    | packed-attrib
    ;

ptr-decl
    : mul
    ;

array-decl
    : lsquare expr rsquare
    ;

type-decl-body
    : struct-decl
    | tuple-decl
    | enum-decl
    | union-decl
    | variant-decl
    | name-decl
    ;

struct-decl
    : lbrace named-type-list? rbrace
    ;

tuple-decl
    : lparen type-list? rparen
    ;

enum-decl
    : enum lbrace enum-body rbrace
    ;

union-decl
    : union struct-decl
    ;

variant-decl
    : variant lbrace variant-body? rbrace
    ;

name-decl
    : name
    ;


named-type-list
    : ident colon type-decl (comma named-type-list)?
    ;

type-list
    : type-decl (comma type-list)?
    ;

enum-body
    : ident (assign expr)? (comma enum-body)?
    ;

variant-body
    : ident (colon expr)? bigarrow type-decl (comma variant-body)?
    ;


typedef-decl
    : type ident assign type-decl
    ;

body-decl
    : typedef-decl | funcdef-decl | builtin-body
    ;

attrib
    : at deprecated-attrib
    ;


deprecated-attrib
    : `deprecated` lparen string rparen
    ;

packed-attrib
    : `packed` lparen expr rparen
    ;


program
    : import-decl* body-decl*
    ;
