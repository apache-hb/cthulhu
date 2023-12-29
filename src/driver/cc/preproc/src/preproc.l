%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="pp_"

%x COMMENT
%x TEXT
%x DIRECTIVE

%%

"//".* ;

"/*" { BEGIN(COMMENT); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>\n ;
<COMMENT>. ;

. { BEGIN(TEXT); }
<TEXT>\n { BEGIN(INITIAL); }

"#" { BEGIN(DIRECTIVE); return PP_DIRECTIVE; }
<DIRECTIVE>\n { BEGIN(INITIAL); }

%%
