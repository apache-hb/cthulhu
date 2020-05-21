#ifndef KEYWORDS_H
#define KEYWORDS_H

typedef enum {
#define KEY(id, _) id,
#define OP(id, _) id,
#define ASM(id, _) id,
#include "keywords.inc"
} keyword;

const char* keyword_to_string(keyword key);

#endif // KEYWORDS_H
