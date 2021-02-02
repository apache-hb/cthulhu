grammar cthulhu;

/* expressions */

expr : 'a' ;

decl : expr ';' ;



/* template stuff */
type_param_decl : '!<' type_param (',' type_param)* '>' ;

type_param : Ident ':' qual_type ;




/* used for declaring a function */

func_decl : 'def' Ident generic_func_body ;

func_impl_decl : 'def' Ident ('::' Ident)* generic_func_body ;

generic_func_body : type_param_decl? func_args? func_result? (func_body_short | func_body) ;

func_args : '(' func_args_body ')' ;

func_args_body : func_arg (',' func_args_body)? | default_func_args_body ;

default_func_args_body : default_func_arg (',' default_func_args_body)? ;

func_arg : Ident ':' type ;

default_func_arg : Ident ':' type '=' expr ;

func_result : '->' type ;

func_body_short : '=' expr ';' ;
func_body : '{' decl* '}' ;




/* used for declaring a variable */

let_decl : 'let' Ident (let_no_init | let_init) ';' ;

let_no_init : ':' type ;

let_init : let_no_init? '=' expr ;


var_decl : 'var' Ident (var_no_init | var_init) ';' ;

var_no_init : ':' non_var_type ;

var_init : var_no_init? '=' expr ;



/* used for declaring a type */

type_decl : struct_decl | union_decl | enum_decl | alias_decl ;

inherit_decl : ':' inherit_body ;

inherit_body : inherit_item | '(' inherit_item_list ')' ;

inherit_item_list : inherit_item (',' inherit_item)? ;

inherit_item : Ident '=' type ;

struct_decl : 'struct' Ident type_param_decl? inherit_decl? '{' struct_body '}' ;

union_decl : 'union' Ident type_param_decl? inherit_decl? '{' union_body '}' ;

enum_decl : 'enum' Ident type_param_decl? inherit_decl? '{' enum_body '}' ;

tagged_enum_decl : 'enum' 'union' Ident type_param_decl? inherit_decl? '{' tagged_enum_body '}' ;

alias_decl : 'using' Ident '=' type ;

struct_body : (let_decl | var_decl | func_impl_decl | alias_decl)* ;
union_body : (let_decl | var_decl | func_impl_decl | alias_decl)* ;
enum_body : enum_fields (func_impl_decl | alias_decl)* ;

tagged_enum_body : tagged_enum_field (',' tagged_enum_body)? | init_tagged_enum_fields ;

tagged_enum_field : Ident tagged_enum_field_body ;

init_tagged_enum_fields : init_tagged_enum_field (',' init_tagged_enum_fields)? ;

init_tagged_enum_field : Ident tagged_enum_field_body '=' expr ;

tagged_enum_field_body : '{' (tagged_enum_item (',' tagged_enum_item)*)? '}' ;

tagged_enum_item : Ident ':' type ;

enum_fields : enum_field (',' enum_fields)? | init_enum_fields ;

init_enum_fields : init_enum_field (',' init_enum_fields)? ;

enum_field : Ident ;

init_enum_field : Ident '=' expr ;



/* used for naming a type */
type : ptr_type | ref_type | name_type | qual_type | var_type ;

non_ref_type : ptr_type | qual_type | name_type ;
non_ref_ptr_type : qual_type | name_type ;
non_var_type : ptr_type | ref_type | name_type | qual_type ;

/* ptrs to refs are banned */
ptr_type : '*' non_ref_type ;

/* refs to refs and refs to ptrs are banned */
ref_type : '&' non_ref_ptr_type ;

/* nested var(var(type)) makes no sense */
var_type : 'var' '(' non_var_type ')' ;

name_type : Ident type_args? ;

qual_type : name_type ('::' qual_type)? ;

type_args : '!<' type_arg_body '>' ;

type_arg_body : type_arg (',' type_arg_body)? | named_type_arg_body ;

type_arg : type ;

named_type_arg_body : named_type_arg (',' named_type_arg_body)? ;

named_type_arg : Ident '=' type ;

Ident : [a-zA-Z_][a-zA-Z0-9_]*;
