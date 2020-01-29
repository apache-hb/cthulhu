#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdint.h>

// all keywords
// if you update this make sure to update lexer.c to reflect new keywords
typedef enum {
    kw_for = 0,
    kw_if = 1,
    kw_else = 2,
    kw_def = 3,
    kw_while = 4,
    kw_switch = 5,
    kw_case = 6,
} keyword_t;

typedef enum {
    string = 0,
    letter = 1,
    integer = 2,
    floating = 3,
    keyword = 4,
    eof = 5
} token_type_t;

typedef struct {
    token_type_t type;
    uint64_t row;
    uint64_t col;
    uint64_t cursor;

    union
    {
        keyword_t key;
        char* str;
    };
} token_t;

void token_free(token_t tok);

// type that defines a file, allows in memory files
typedef struct {
    // pointer to a block of data
    void* data;
    
    // function that gets the next character
    char(*next)(void*);

    // function that peeks the next character
    char(*peek)(void*);
    
    // function that closes the file/frees the data
    void(*close)(void*);

    // function to seek in the file
    void(*seek)(void*, uint64_t);

    // function to tell current offset
    uint64_t(*tell)(void*);
} file_t;

typedef struct {
    // pointer to code source
    file_t* file;

    // current row
    uint64_t row;
    // current column
    uint64_t col;

    // cursor
    uint64_t cursor;

    uint32_t buf_cursor;
    // internal buffer for lexer use
    char* buf;

    // last token parsed, used for peek
    token_t tok;
} lexer_t;

lexer_t* lexer_alloc(file_t* file);
void lexer_free(lexer_t* self);

token_t lexer_next(lexer_t* self);
token_t lexer_peek(lexer_t* self);

#endif // COMMON_H
