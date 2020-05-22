#include "token.h"

#include "utils.h"

#include <stdio.h>

char* token_to_string(token_t tok)
{
    if(tok.type == IDENT)
    {
        return strfmt("IDENT(%s)", tok.data.ident);
    }
    else if(tok.type == KEYWORD)
    {
        return strfmt("KEYWORD(%s)", keyword_to_string(tok.data.key));
    }
    else if(tok.type == ERROR)
    {
        return strfmt("ERROR(%s)", tok.data.error);
    }
    else if(tok.type == END)
    {
        return strfmt("EOF");
    }
    else if(tok.type == INVALID)
    {
        return strfmt("INVALID");
    }
    else
    {
        return strfmt("unknown token");
    }
}