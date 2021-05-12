%output "bison.c"
%defines "bison.h"

%define parse.error verbose // these messages are still awful but better than nothing
%define api.pure full
%param { scan_extra_t *x }
%locations
%expect 0 // TODO: resolve dangling else without requiring compound stmts everywhere

%code requires {
    #include <stdbool.h>
    #include "scanner.h"
}

%{
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include "ast.h"

int yylex();
int yyerror();

#define YYLEX_PARAM x->scanner

%}

%union {
    char *text;
    bool cond;
    struct node_t *node;
    struct nodes_t *nodes;
    struct path_t *path;
}

%token<text>
    IDENT "identifier"
    DIGIT "integer literal"
    BDIGIT "binary integer literal"
    XDIGIT "hexadecimal integer literal"
    STRING "string literal"
    MSTRING "raw string literal"
    NIL "null"

%token
    LPAREN "("
    RPAREN ")"
    LBRACE "{"
    RBRACE "}"
    LSQUARE "["
    RSQUARE "]"
    COMMA ","
    ASSIGN "="
    QUESTION "?"
    COLON ":"
    COLON2 "::"
    DOT "."
    ARROW "->"
    SEMI ";"
    AT "@"
    DOT2 ".."

%token
    ADDEQ "+="
    SUBEQ "-="
    DIVEQ "/="
    MULEQ "*="
    REMEQ "%="
    SHLEQ "<<="
    SHREQ ">>="
    ANDEQ "&="
    OREQ "|="
    XOREQ "^="

%token
    ADD "+"
    SUB "-"
    DIV "/"
    MUL "*"
    REM "%"
    NOT "!"
    SHL "<<"
    SHR ">>"
    EQ "=="
    NEQ "!="
    GT ">"
    GTE ">="
    LT "<"
    LTE "<="
    OR "||"
    AND "&&"
    BITAND "&"
    BITOR "|"
    BITXOR "^"
    FLIP "~"

%token
    DEF "def"
    EXPORT "export"
    COMPILE "compile"
    MODULE "module"
    USING "using"
    IMPORT "import"
    RECORD "record"
    UNION "union"
    VARIANT "variant"
    VAR "var"
    FINAL "final"
    AS "as"
    RETURN "return"
    CONTINUE "continue"
    BREAK "break"
    WHILE "while"
    FOR "for"
    IF "if"
    ELSE "else"
    WITH "with"
    ASM "asm"

%type<node>
    number literal expr additive multiplicative unary postfix primary
    init result vardecl type decl basedecl param funcdecl body defaultparam
    array closure aliasdecl record field import union variant
    case module subscript bitshift comparison equality bitwise logical
    stmt compound return assign asm break continue namedarg attribute
    while else names

%type<nodes>
    args call params parambody defaultparams types typelist
    fields cases casedata decls stmts namedargs attribs attributes
    attrib namelist 

%type<path>
    path list items qual

%type<cond>
    mut comptime exported

%type<text>
    digit string label

%start unit

%%

unit: decls { x->ast = $1; dump_nodes($1, true); printf("\n"); }
    ;

decls: decl { $$ = list($1); }
    | decls decl { $$ = list_add($1, $2); }
    ;

decl: import { $$ = $1; }
    | exported comptime basedecl { $$ = comptime(exported($3, $1), $2); }
    | attribs exported comptime basedecl { $$ = attach(comptime(exported($4, $2), $3), $1); }
    ;

exported: %empty { $$ = false; }
    | EXPORT { $$ = true; }
    ;

comptime: %empty { $$ = false; }
    | COMPILE { $$ = true; }
    ;

attribs: attrib { $$ = $1; }
    | attribs attrib { $$ = list_join($1, $2); }
    ;

attrib: AT attribute { $$ = list($2); }
    | AT LSQUARE attributes RSQUARE { $$ = $3; }
    ;

attributes: attribute { $$ = list($1); }
    | attributes COMMA attribute { $$ = list_add($1, $3); }
    ;

attribute: qual { $$ = attribute($1, NULL); }
    | qual call { $$ = attribute($1, $2); }
    ;

import: IMPORT path SEMI { $$ = include($2, NULL); }
    | IMPORT path LPAREN items RPAREN SEMI { $$ = include($2, $4); }
    ;

items: DOT2 { $$ = empty_path(); }
    | list { $$ = $1; }
    ;

list: IDENT { $$ = path($1); }
    | list COMMA IDENT { $$ = path_add($1, $3); }
    ;

path: IDENT { $$ = path($1); }
    | path COLON2 IDENT { $$ = path_add($1, $3); }
    ;

basedecl: vardecl { $$ = $1; }
    | funcdecl { $$ = $1; }
    | aliasdecl { $$ = $1; }
    | union { $$ = $1; }
    | record { $$ = $1; }
    | variant { $$ = $1; }
    | module { $$ = $1; }
    ;

module: MODULE path LBRACE decls RBRACE { $$ = scope($2, $4); }
    ;

record: RECORD IDENT LBRACE fields RBRACE { $$ = record($2, $4); }
    ;

union: UNION IDENT LBRACE fields RBRACE { $$ = uniondecl($2, $4); }
    ;

variant: VARIANT IDENT result LBRACE cases RBRACE { $$ = variant($2, $3, $5); }
    ;

cases: case { $$ = list($1); }
    | cases COMMA case { $$ = list_add($1, $3); }
    ;

case: IDENT casedata init { $$ = variantcase($1, $2, $3); }
    ;

casedata: %empty { $$ = NULL; }
    | LPAREN fields RPAREN { $$ = $2; }
    ;

fields: field { $$ = list($1); }
    | fields COMMA field { $$ = list_add($1, $3); }
    ;

field: IDENT COLON type { $$ = item($1, $3); }
    ;

aliasdecl: USING IDENT ASSIGN type SEMI { $$ = alias($2, $4); }
    ;

funcdecl: DEF IDENT params result body { $$ = func($2, $3, $4, $5); }
    ;

body: SEMI { $$ = NULL; }
    | ASSIGN expr SEMI { $$ = $2; }
    | compound { $$ = $1; }
    | vardecl { $$ = $1; }
    | aliasdecl { $$ = $1; }
    ;

stmt: compound { $$ = $1; }
    | return { $$ = $1; }
    | assign { $$ = $1; }
    | break { $$ = $1; }
    | continue { $$ = $1; }
    | while { $$ = $1; }
    ;

while: WHILE label expr compound else { $$ = nwhile($2, $3, $4, $5); }
    ;

label: %empty { $$ = NULL; } 
    | COLON IDENT { $$ = $2; }
    ;

else: %empty { $$ = NULL; }
    | ELSE compound { $$ = $2; }
    ;

break: BREAK SEMI { $$ = nbreak(NULL); }
    | BREAK IDENT SEMI { $$ = nbreak($2); }
    ;

continue: CONTINUE SEMI { $$ = ncontinue(); }
    ;

return: RETURN SEMI { $$ = result(NULL); }
    | RETURN expr SEMI { $$ = result($2); }
    ;

assign: expr ASSIGN expr SEMI { $$ = assign($1, $3, ASSIGN); }
    | expr ADDEQ expr SEMI { $$ = assign($1, $3, ADDEQ); }
    | expr SUBEQ expr SEMI { $$ = assign($1, $3, SUBEQ); }
    | expr DIVEQ expr SEMI { $$ = assign($1, $3, DIVEQ); }
    | expr MULEQ expr SEMI { $$ = assign($1, $3, MULEQ); }
    | expr REMEQ expr SEMI { $$ = assign($1, $3, REMEQ); }
    | expr ANDEQ expr SEMI { $$ = assign($1, $3, ANDEQ); }
    | expr OREQ expr SEMI { $$ = assign($1, $3, OREQ); }
    | expr XOREQ expr SEMI { $$ = assign($1, $3, XOREQ); }
    | expr SHLEQ expr SEMI { $$ = assign($1, $3, SHLEQ); }
    | expr SHREQ expr SEMI { $$ = assign($1, $3, SHREQ); }
    ;

compound: LBRACE stmts RBRACE { $$ = compound($2); }
    ;

stmts: %empty { $$ = empty_list(); }
    | stmts stmt { $$ = list_add($1, $2); }
    ;

params: LPAREN parambody RPAREN { $$ = $2; }
    | LPAREN RPAREN { $$ = empty_list(); }
    ;

parambody: param { $$ = list($1); }
    | defaultparams { $$ = $1; }
    | param COMMA parambody { $$ = list_push($3, $1); }
    ;

defaultparams: defaultparam { $$ = list($1); }
    | defaultparam COMMA defaultparams { $$ = list_push($3, $1); }
    ;

param: IDENT COLON type { $$ = param($1, $3, NULL); }
    ;

defaultparam: IDENT COLON type ASSIGN expr { $$ = param($1, $3, $5); }
    ;

vardecl: mut names result init SEMI { $$ = var($2, $3, $4, $1); }
    ;

names: IDENT { $$ = name($1); }
    | LSQUARE namelist RSQUARE { $$ = items($2); }
    ;

namelist: IDENT { $$ = list(name($1)); }
    | namelist COMMA IDENT { $$ = list_add($1, name($3)); }
    ;

result: %empty { $$ = NULL; }
    | COLON type { $$ = $2; }
    ;

init: %empty { $$ = NULL; }
    | ASSIGN expr { $$ = $2; }
    ;

type: qual { $$ = qualified($1); }
    | LSQUARE array RSQUARE { $$ = $2; }
    | closure { $$ = $1; }
    | MUL type { $$ = pointer($2); }
    | VAR type { $$ = mut($2); }
    ;

array: type COLON expr { $$ = array($1, $3); }
    | type { $$ = array($1, NULL); }
    ;

closure: LPAREN types RPAREN ARROW type { $$ = closure($2, $5); }
    ;

types: %empty { $$ = empty_list(); }
    | typelist { $$ = $1; }
    ;

typelist: type { $$ = list($1); }
    | typelist COMMA type { $$ = list_add($1, $3); }
    ;

qual: IDENT { $$ = path($1); }
    | qual COLON2 IDENT { $$ = path_add($1, $3); }
    ;

mut: VAR { $$ = true; } | FINAL { $$ = false; } ;

primary: literal { $$ = $1; }
    | qual { $$ = qualified($1); }
    | LPAREN expr RPAREN { $$ = $2; }
    | asm { $$ = $1; }
    ;

asm: ASM { $$ = NULL; }
    ;

postfix: primary { $$ = $1; }
    | postfix DOT IDENT { $$ = field($1, $3, false); }
    | postfix ARROW IDENT { $$ = field($1, $3, true); }
    | postfix call { $$ = apply($1, $2); }
    | postfix subscript { $$ = subscript($1, $2); }
    | postfix AS type { $$ = cast($1, $3); }
    ;

unary: postfix { $$ = $1; }
    | ADD unary { $$ = unary(ADD, $2); }
    | SUB unary { $$ = unary(SUB, $2); }
    | BITAND unary { $$ = unary(BITAND, $2); }
    | MUL unary { $$ = unary(MUL, $2); }
    | NOT unary { $$ = unary(NOT, $2); }
    | FLIP unary { $$ = unary(FLIP, $2); }
    ;

multiplicative: unary { $$ = $1; }
    | multiplicative MUL unary { $$ = binary(MUL, $1, $3); }
    | multiplicative DIV unary { $$ = binary(DIV, $1, $3); }
    | multiplicative REM unary { $$ = binary(REM, $1, $3); }
    ;

additive: multiplicative { $$ = $1; }
    | additive ADD multiplicative { $$ = binary(ADD, $1, $3); }
    | additive SUB multiplicative { $$ = binary(SUB, $1, $3); }
    ;

comparison: additive { $$ = $1; }
    | additive GT comparison { $$ = binary(GT, $1, $3); }
    | additive GTE comparison { $$ = binary(GTE, $1, $3); }
    | additive LT comparison { $$ = binary(LT, $1, $3); }
    | additive LTE comparison { $$ = binary(LTE, $1, $3); }
    ;

equality: comparison { $$ = $1; }
    | comparison EQ equality { $$ = binary(EQ, $1, $3); }
    | comparison NEQ equality { $$ = binary(NEQ, $1, $3); }
    ;

bitshift: equality { $$ = $1; }
    | equality SHL bitshift { $$ = binary(SHL, $1, $3); }
    | equality SHR bitshift { $$ = binary(SHR, $1, $3); }
    ;

bitwise: bitshift { $$ = $1; }
    | bitshift BITAND bitwise { $$ = binary(BITAND, $1, $3); }
    | bitshift BITOR bitwise { $$ = binary(BITOR, $1, $3); }
    | bitshift BITXOR bitwise { $$ = binary(BITXOR, $1, $3); }
    ;

logical: bitwise { $$ = $1; }
    | bitwise AND logical { $$ = binary(AND, $1, $3); }
    | bitwise OR logical { $$ = binary(OR, $1, $3); }
    ;

expr: logical { $$ = $1; }
    | logical QUESTION expr COLON expr { $$ = ternary($1, $3, $5); }
    | logical QUESTION COLON expr { $$ = ternary($1, NULL, $4); }
    ;

subscript: LSQUARE expr RSQUARE { $$ = $2; }
    ;

call: LPAREN RPAREN { $$ = empty_list(); }
    | LPAREN args RPAREN { $$ = $2; }
    ;

args: namedargs { $$ = $1; }
    | expr { $$ = list(arg(NULL, $1)); }
    | expr COMMA args { $$ = list_push($3, arg(NULL, $1)); }
    ;

namedargs: namedarg { $$ = list($1); }
    | namedarg COMMA namedargs { $$ = list_push($3, $1); }
    ;

namedarg: DOT IDENT ASSIGN expr { $$ = arg($2, $4); }
    ;

literal: number { $$ = $1; }
    | string { $$ = string($1); }
    | NIL { $$ = nil(); }
    ;

number: digit { $$ = digit($1, NULL); }
    | digit IDENT { $$ = digit($1, $2); }
    ;

digit: DIGIT { $$ = $1; }
    | XDIGIT { $$ = $1; }
    | BDIGIT { $$ = $1; }
    ;

string: STRING { $$ = $1; }
    | MSTRING { $$ = $1; }
    ;

%%
