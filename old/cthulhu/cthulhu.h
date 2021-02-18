#ifndef CTHULHU_H
#define CTHULHU_H

#include <stddef.h>

#include <string>
#include <unordered_set>

typedef size_t CtHash;
typedef size_t CtChar;
typedef const char *CtStr;
typedef int CtBool;

typedef enum {
    K_INVALID
} CtKeyword;

typedef enum {
    BASE2, BASE10, BASE16
} CtDigitBase;

typedef struct {
    CtDigitBase base;
    size_t num;
} CtDigit;

typedef struct {
    CtStr str;
    size_t len;
} CtString;

typedef struct {
    CtHash hash;
    CtStr str;
} CtIdent;

typedef struct {
    size_t offset;
    size_t line;
    size_t column;
} CtOffset;

typedef struct {
    size_t offset;
    size_t line;
    size_t column;
    size_t length;
} CtRange;

typedef enum {
    TK_IDENT,
    TK_KEY,
    TK_INT,
    TK_STRING,
    TK_CHAR,
    TK_END,

    TK_INVALID
} CtTokenKind;

typedef union {
    CtIdent ident;
    CtDigit num;
    CtString str;
    CtChar letter;
    CtKeyword key;
} CtTokenData;

typedef struct {
    CtTokenKind type;
    CtTokenData data;
    CtRange range;
} CtToken;

typedef enum {
    RT_NONE
} CtReportType;

typedef struct {
    CtReportType type;
} CtReport;

typedef struct CtState {
    CtReport *reports;
    size_t head_report;
    size_t tail_report;
    size_t max_reports;

    CtReport get();
} CtState;

typedef enum {
    ERR_NONE,

    ERR_ALLOC, /* allocation failed */

    ERR_EMPTY, /* list is empty */

    ERR_ENCODING, /* invalid character code */
} CtError;

CtState ctStateNew(size_t reports);
void ctStateFree(CtState *self);


typedef struct CtStream {
    void *handle;
    int(*get)(void*);
    int ahead;

    std::string buffer;
    CtOffset offset;

    char next();
    char peek();
} CtStream;

CtStream ctStreamOpen(void *data, int(*get)(void*));


typedef struct CtLexer {
    CtStream *source;

    std::unordered_set<std::string> pool;

    CtToken next();
} CtLexer;

CtLexer ctLexerOpen(CtStream *stream);

#endif /* CTHULHU_H */
