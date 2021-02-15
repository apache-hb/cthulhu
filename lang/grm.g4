grammar grm;

start : unit ;

unit : include* toplevel* ;

include : 'using' includePath includeBody? ';' ;

includePath : Ident ('::' Ident)* ;

includeBody : '(' includeItems ')' ;

includeItems : '...' | Ident (',' Ident)* ;

toplevel : alias ;

alias : 'using' Ident '=' type ';' ;



type : ptrType | refType | qualType | varType ;

refType : '&' nonRefPtrType ;
ptrType : '*' nonRefType ;
varType : 'var' '(' nonVarType ')' ;

nonRefPtrType : qualType | nameType ;
nonRefType : ptrType | qualType | nameType ;
nonVarType : ptrType | refType | nameType | qualType ;
nonRefVarType : ptrType | nameType | qualType ;

qualType : nameType ('::' nameType)* ;

nameType : Ident typeArgs? ;


typeArgs : '!<' type (',' type)* '>' ;

typeParams : '!<' typeParamBody '>' ;

typeParamBody : typeParam (',' typeParamBody)? | defaultTypeParamBody ;

typeParam : Ident ':' typeConstraints ;

defaultTypeParamBody : defaultTypeParam (',' defaultTypeParam)* ;

defaultTypeParam : Ident ':' typeConstraints '=' nonRefVarType ;

typeConstraints : nonRefVarType ('+' nonRefVarType)* ;


Ident : [a-zA-Z_][a-zA-Z0-9_]* ;
