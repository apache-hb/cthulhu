grammar cthulhu;

unit : import_decl* body_decl* ;

import_decl : 'import' path import_spec? NL ;

import_spec : '(' ID (',' ID)* ')' ;

body_decl : alias_decl | struct_decl ;

struct_field : ID ':' type NL ;

struct_decl : 'struct' ID '{' struct_field* '}' ;

alias_decl : 'alias' ID '=' type NL ;

type : '&'? typebody | '*' type ;

typebody : path ;

path : ID ('::' ID)* ;

NL : ';' ;

ID : [a-zA-Z_][a-zA-Z0-9_]* ;
WS : [ \t\r\n]+ -> skip;