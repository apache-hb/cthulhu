#pragma once

#include <cstddef>
#include <string>

using size_t = std::size_t;

struct Lexer;

struct Range {
    Lexer* lexer;
    size_t offset;
    size_t length;
};

struct Location {
    size_t line;
    size_t column;
};

#define TYPE_CORE (1 << 8)
#define TYPE_ASM (2 << 8)
#define OP(it) (it)
#define HEIGHT(it) (it << 16)

// operator precedence sorted by decending order
#define INVALID_OP HEIGHT(0) // nothing may have a precedence of 0
#define MULTIPLICATIVE_OP HEIGHT(1) // * / %
#define MATHEMATICAL_OP HEIGHT(2) // + -
#define BITSHIFT_OP HEIGHT(3) // << >>
#define BITWISE_OP HEIGHT(4) // & | ^
#define COMPARISON_OP HEIGHT(5) // < <= > >=
#define EQUALITY_OP HEIGHT(6) // != ==
#define LOGICAL_OP HEIGHT(7) // && ||

// extract the precedence of the operator
#define PRECEDENCE(key) ((key & 0x00FF0000) >> 16)

// extract the type of the keyword
#define TYPE(key) (key & 0x0000FF00)

// 0x00_00_00_00
//   ^^ ^^ ^^ ^^
//   |  |  |  unique per category
//   |  |  category of the keyword
//   |  precendence when needed
//   unused
//
enum Key : int {
    INVALID = 0,

    // core keywords
    RECORD = TYPE_CORE | OP(1),
    VARIANT = TYPE_CORE | OP(2),
    TRUE = TYPE_CORE | OP(3),
    FALSE = TYPE_CORE | OP(4),
    ASM = TYPE_CORE | OP(5),
    IF = TYPE_CORE | OP(6),
    ELSE = TYPE_CORE | OP(7),
    WHILE = TYPE_CORE | OP(8),
    FOR = TYPE_CORE | OP(9),
    BREAK = TYPE_CORE | OP(10),
    CONTINUE = TYPE_CORE | OP(11),
    RETURN = TYPE_CORE | OP(12),
    SWITCH = TYPE_CORE | OP(13),
    CASE = TYPE_CORE | OP(14),
    VAR = TYPE_CORE | OP(16),
    LET, // reserved
    UNION = TYPE_CORE | OP(18),
    FINAL, // reserved
    USING = TYPE_CORE | OP(20),

    COLON,
    COLON2,
    LBRACE,
    RBRACE,
    LPAREN,
    RPAREN,
    LSQUARE,
    RSQUARE,
    SEMI,
    DOT,
    DOT2,
    DOT3,
    ASSIGN,
    COMMA,
    AT,
    ARROW,

    BITNOT, // ~

    // template operators
    BEGIN = TYPE_CORE | OP(64),
    END = TYPE_CORE | OP(65),

    // core unary operators
    NOT = TYPE_CORE | OP(66), // !
    TERNARY = TYPE_CORE | OP(67), // ?

    // core binary operators
    MUL = TYPE_CORE | MULTIPLICATIVE_OP | OP(0),
    MOD = TYPE_CORE | MULTIPLICATIVE_OP | OP(1),
    DIV = TYPE_CORE | MULTIPLICATIVE_OP | OP(2),

    ADD = TYPE_CORE | MATHEMATICAL_OP | OP(1),
    SUB = TYPE_CORE | MATHEMATICAL_OP | OP(2),

    SHL = TYPE_CORE | BITSHIFT_OP | OP(1),
    SHR = TYPE_CORE | BITSHIFT_OP | OP(2),

    BITAND = TYPE_CORE | BITWISE_OP | OP(1),
    BITOR = TYPE_CORE | BITWISE_OP | OP(2),
    BITXOR = TYPE_CORE | BITWISE_OP | OP(3),

    GT = TYPE_CORE | COMPARISON_OP | OP(1),
    GTE = TYPE_CORE | COMPARISON_OP | OP(2),
    LT = TYPE_CORE | COMPARISON_OP | OP(3),
    LTE = TYPE_CORE | COMPARISON_OP | OP(4),

    EQ = TYPE_CORE | EQUALITY_OP | OP(1),
    NEQ = TYPE_CORE | EQUALITY_OP | OP(2),

    AND = TYPE_CORE | LOGICAL_OP | OP(1),
    OR = TYPE_CORE | LOGICAL_OP | OP(2),

    // core statement operators
    MULEQ = TYPE_CORE | INVALID_OP | OP(128),
    MODEQ = TYPE_CORE | INVALID_OP | OP(129),
    DIVEQ = TYPE_CORE | INVALID_OP | OP(130),

    ADDEQ = TYPE_CORE | INVALID_OP | OP(131),
    SUBEQ = TYPE_CORE | INVALID_OP | OP(132),

    SHLEQ = TYPE_CORE | INVALID_OP | OP(133),
    SHREQ = TYPE_CORE | INVALID_OP | OP(134),

    BITANDEQ = TYPE_CORE | INVALID_OP | OP(135),
    BITOREQ = TYPE_CORE | INVALID_OP | OP(136),
    BITXOREQ = TYPE_CORE | INVALID_OP | OP(137),

    // asm operators
    _ADD = TYPE_ASM | OP(0),
    _XOR = TYPE_ASM | OP(1)
};

std::string to_repr(Key key);
const char* to_string(Key key);

struct Int {
    size_t number;
    const std::string* suffix;
};

struct Token {
    enum Type {
        IDENT, // identifier
        KEY, // keyword
        STRING, // string literal
        CHAR, // char literal
        INT, // integer literal
        END, // end of file

        MONOSTATE, // error handling state, this token isnt real

        ERROR,
        ERROR_STRING_EOF, // string wasnt terminated
        ERROR_STRING_LINE, // newline found in string
        ERROR_INVALID_ESCAPE, // invalid escaped character in string
        ERROR_LEADING_ZERO, // an integer literal started with a 0
        ERROR_INT_OVERFLOW, // integer literal was too large
        ERROR_UNRECOGNIZED_CHAR, // unrecognized character in stream
        ERROR_CHAR_OVERFLOW, // character literal was too large
    };

    union Data {
        const std::string* ident;
        Key key;
        const std::string* string;
        const std::string* letters;
        Int number;
    };

    Token()
        : range({})
        , type(MONOSTATE)
    { }

    Token(Range range)
        : range(range)
        , type(END)
    { }

    Token(Range range, Type type, Data data = {})
        : range(range)
        , type(type)
        , data(data)
    { }

    // the internal representation of this token
    std::string repr();

    // the original text of this token
    std::string text();

    // pretty print the source location
    // @param name include the file name
    // @param head the start of the token
    // @param tail the end of the token
    std::string where(
        bool name, 
        Location* head = nullptr, 
        Location* tail = nullptr
    );

    // underline the text and handle multiple lines
    std::string pretty(bool underline = true, const std::string& message = "");

    // is this token an error token
    bool error() const;

    Range range;
    Type type;
    Data data;
};
