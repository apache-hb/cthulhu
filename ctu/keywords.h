#ifndef KEYWORDS
#define KEYWORDS

#define KEY(id, _) id,
#define OP(id, _) id,
#define RES(id, _) id,

typedef enum {
    kinvalid = 0,
#include "keywords.inc"
} ctu_keyword;

const char* ctu_keyword_str(ctu_keyword);

#endif /* KEYWORDS */
