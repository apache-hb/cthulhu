#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include <string>

typedef struct {
    void* src;
    char(*next)(void*);
} input_t;

char input_next(input_t* self) {
    return self->next(self->src);
}

typedef struct {
    uint64_t dist;
    uint64_t col;
    uint64_t line;
} file_pos_t;

typedef struct {
    file_pos_t pos;
    file_pos_t here;
    input_t input;
    char peek;
} lexer_t;

lexer_t make_lexer(input_t in) {
    lexer_t self;
    memset(&self, 0, sizeof(lexer_t));
    self.input = in;
    self.peek = ' ';
    return self;
}

typedef enum {
    token_type_ident,
    token_type_keyword,
    token_type_invalid
} token_type_e;

typedef enum {
    keyword_type, // `type`
    keyword_enum, // `enum`
    keyword_union, // `union`
    keyword_variant, // `variant`
    keyword_object, // `object`

    keyword_include, // `include`

    keyword_semicolon, // `;`
    keyword_newline, // \n

    keyword_assign, // `:=`
    keyword_colon, // `:`
} keyword_e;

typedef struct {
    token_type_e type;
    file_pos_t pos;

    union {
        char* ident;
        keyword_e key;
    };
} token_t;

char lexer_getc(lexer_t* self) {
    char c = self->peek;
    self->peek = input_next(&self->input);

    self->pos.dist++;
    if(c == '\n') {
        self->pos.col = 0;
        self->pos.line++;
    } else {
        self->pos.col++;
    }

    return c;
}

token_t make_key(file_pos_t pos, keyword_e k) {
    token_t tok;
    tok.type = token_type_keyword;
    tok.key = k;
    tok.pos = pos;
    return tok;
}

token_t make_ident(file_pos_t pos, char* ident) {
    token_t tok;
    tok.type = token_type_ident;
    tok.ident = ident;
    tok.pos = pos;
    return tok;
}

token_t make_invalid(file_pos_t pos) {
    token_t tok;
    tok.type = token_type_invalid;
    tok.pos = pos;
    return tok;
}

char skip_whitespace(lexer_t* self) {
    char c = lexer_getc(self);
    while(isspace(c)) {
        c = lexer_getc(self);
        if(c == '\n') {
            return c;
        }
    }
    return c;
}

token_t skip_comment(lexer_t* self) {
    char c = lexer_getc(self);
    while(c != '\n') {
        c = lexer_getc(self);
    }
    return make_key(self->pos, keyword_newline);
}

int lexer_consume(lexer_t* self, char c) {
    if(self->peek == c) {
        lexer_getc(self);
        return 1;
    }
    return 0;
}

token_t lexer_next(lexer_t* self) {
    char c = skip_whitespace(self);
    if(c == '\n') {
        return make_key(self->pos, keyword_newline);
    } else if(c == '#') {
        return skip_comment(self);
    }

    file_pos_t here = self->pos;

    if(isalpha(c) || c == '_') {
        std::string buf = {c};
        while(isalnum(self->peek) || self->peek == '_') {
            buf += lexer_getc(self);
        }

        if(buf == "type") {
            return make_key(here, keyword_type);
        } else if(buf == "enum") {
            return make_key(here, keyword_enum);
        } else if(buf == "union") {
            return make_key(here, keyword_union);
        } else if(buf == "variant") {
            return make_key(here, keyword_variant);
        } else if(buf == "object") {
            return make_key(here, keyword_object);
        } else if(buf == "include") {
            return make_key(here, keyword_include);
        } else {
            return make_ident(here, strdup(buf.c_str()));
        }
    } else if(isdigit(c)) {

    } else {
        switch(c) {
        case ':': return make_key(here, lexer_consume(self, '=') ? keyword_assign : keyword_colon);
        case '=': 
        case '-':
        case '/':
        case ';': return make_key(here, keyword_semicolon);
        default: return make_invalid(here);
        }
    }

    return make_invalid(here);
}

typedef struct {
    lexer_t lex;
} parser_t;

parser_t make_parser(lexer_t lex) {
    parser_t self;
    self.lex = lex;
    return self;
}

typedef enum {
    node_type_program,
    node_type_type,
    node_type_name,
    node_type_struct,
    node_type_union,
    node_type_enum,
    node_type_variant
} node_type_e;

typedef struct {
    char* name;
    struct node_t* data;
} pair_t;

typedef struct node_t {
    node_type_e type;
    union {
        struct { int nincludes; struct node_t* includes; int count; struct node_t* body; } _program;
        struct { int count; char** parts; char* alias; } _include;
        struct { char* name; struct node_t* type; } _type;
        struct { int count; pair_t* fields; } _struct;
        struct { int count; pair_t* fields; } _union;
        struct { int count; struct node_t* backing; pair_t* fields; } _enum;
        struct { int count; struct node_t* backing; pair_t* values; pair_t* fields; } _variant;
        struct { int count; char** path; } _typename;
    };
} node_t;

int main(int argc, const char** argv) {
    if(argc != 2)
        return 1;

    input_t in;
    in.src = fopen(argv[1], "r");
    in.next = [](void* in) -> char { return fgetc((FILE*)in); };

    lexer_t lex = make_lexer(in);

    token_t tok = lexer_next(&lex);
    printf("type: %d\n", tok.type);
}