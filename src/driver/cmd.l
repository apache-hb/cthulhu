%option 8bit
%option noyywrap noinput nounput
%option never-interactive batch
%option prefix="cmd"

%{
#include "cmd-bison.h"
%}

%%

[ \r\n\t] ;

\-\-[a-zA-Z][a-zA-Z0-9]* { return LONG_OPT; }
\-[a-zA-Z][a-zA-Z0-9]* { return SHORT_OPT; }

"enable" { return ENABLE; }
"disable" { return DISABLE; }

[a-zA-Z][a-zA-Z0-9]* { 
    return IDENT; 
}

[0-9]+ { 
    return NUMBER; 
}

%%
