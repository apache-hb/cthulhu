# Grammar

digit:
    0-9

nodigit:
    a-zA-Z_

ident:
    nodigit
    digit

identifier:
    nodigit ident*

## Literals

decimal-literal-digit:
    0-9

decimal-integer-literal:
    decimal-literal-digit*


octal-literal-digit:
    0-7

octal-integer-literal:
    `0o` octal-literal-digit*


hexadecimal-literal-digit:
    0-9
    a-f
    A-F

hexadecimal-integer-literal:
    `0x` hexadecimal-literal-digit*


binary-literal-digit:
    0-1

binary-integer-literal:
    `0b` binary-literal-digit*

integer-literal-body:
    decimal-integer-literal
    octal-integer-literal
    hexadecimal-integer-literal
    binary-integer-literal

integer-literal-suffix:
    `u`
    `ul`
    `U`
    `UL`
    `s`
    `sl`
    `S`
    `SL`

integer-literal:
    integer-literal-body integer-literal-suffix?


floating-literal-suffix:
    `f`
    `F`
    `d`
    `D`

floating-literal-const:
    digit+ `.` digit*

floating-literal-expr:
    `e` digit+
    `E` digit+

floating-literal:
    floating-literal-const floating-literal-exp? floating-literal-suffix?

s-char:

d-char:

raw-char:

s-char-sequence:
    s-char+

d-char-sequence:
    d-char+

raw-char-sequence:
    raw-char+

raw-string:
    `"` d-char-sequence? `(` raw-char-sequence? `)` d-char-sequence? `"`


string-literal:
    encoding-prefix? `"` s-char-sequence? `"`
    encoding-prefix? `R` raw-string

character-literal:


boolean-literal:
    `true`
    `false`

pointer-literal:
    `null`

literal:
    floating-literal
    integer-literal
    character-literal
    string-literal
    boolean-literal
    pointer-literal