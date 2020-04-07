IDENT: [_a-zA-Z]([_a-zA-Z0-9]*)
COLON2: `::`
COLON: `:`
ARROW: `=>`
ASSIGN: `:=`
COMMA: `,`
QUESTION: `?`

LBRACE: `{`
RBRACE: `}`

LPAREN: `(`
RPAREN: `)`

LSQUARE: `[`
RSQUARE: `]`

IMPORT: `import`
RETURN: `return`
WHILE: `while`
FOR: `for`
IF: `if`
MATCH: `match`
ELSE: `else`
DEF: `def`

TYPE: `type`
UNION: `union`
ENUM: `enum`
VARIANT: `variant`

name-decl:  IDENT | 
            IDENT COLON2 name-decl



import-decl:    IMPORT name-decl | 
                IMPORT name-decl ARROW IDENT

imports-decls:  import-decl | 
                import-decl import-decls



struct-body:    IDENT COLON type-decl |
                IDENT COLON type-decl COMMA struct-body

struct-decl:    LBRACE RBRACE |
                LBRACE struct-body RBRACE



tuple-body: type-decl |
            type-decl COMMA tuple-body

tuple-decl: LPAREN RPAREN |
            LPAREN tuple-body RPAREN




union-decl: UNION LBRACE RBRACE |
            UNION LBRACE struct-body RBRACE


enum-body:  IDENT ASSIGN expr |
            IDENT ASSIGN expr COMMA enum-body

enum-decl:  ENUM LBRACE RBRACE |
            ENUM LBRACE enum-body RBRACE


variant-body:   IDENT ARROW type-decl |
                IDENT ARROW type-decl COMMA variant-body

variant-decl:   VARIANT LBRACE RBRACE |
                VARAINT LBRACE variant-body RBRACE




ptr-decl:   MUL type-decl




type-decl:  struct-decl | 
            tuple-decl | 
            union-decl | 
            enum-decl |
            variant-decl |
            ptr-decl |
            name-decl


typedef-decl: TYPE IDENT ASSIGN type-decl


call-args:  expr |
            expr COMMA call-args

array-expr-body:    expr |
                    expr COMMA array-expr-body

tuple-expr-body:    expr |
                    expr COMMA tuple-expr-body

unary-expr: unary-op expr
binary-expr: expr binary-op expr
ternary-expr: expr QUESTION expr COLON expr
cast-expr: expr AS type-decl
const-expr: CHAR | STRING | HEX | BIN | NUM | TRUE | FALSE | NULL
call-expr: expr LPAREN RPAREN | expr LPAREN call-args RPAREN

struct-expr-body:   IDENT ASSIGN expr |
                    IDENT ASSIGN expr COMMA struct-expr-body

tuple-expr:     LPAREN RPAREN |
                LPAREN tuple-expr-body RPAREN

struct-expr:    LBRACE RBRACE |
                LBRACE struct-expr-body RBRACE

array-expr: LSQUARE RSQUARE |
            LSQAURE array-expr-body RSQUARE

index-expr: expr LSQUARE expr RSQUARE


expr:   unary-expr |
        binary-expr |
        ternary-expr |
        cast-expr |
        const-expr |
        call-expr |
        LBRACE expr RBRACE |
        struct-expr |
        tuple-expr |
        array-expr |
        index-expr |
        name-decl


while-stmt: WHILE expr func-body

assign-stmt: expr ASSIGN expr

return-stmt: RETURN expr

for-stmt: FOR IDENT COLON expr func-body

elif-stmt:  ELSE IF expr func-body |
            ELSE IF expr func-body elif-stmt |
            ELSE func-body

if-stmt:    IF expr func-body |
            IF expr func-body elif-stmt

match-body: expr ARROW func-body |
            expr ARROW func-body match-body
            ELSE ARROW func-body

match-stmt: MATCH expr LPAREN match-body RPAREN

stmt:   if-stmt | while-stmt | for-stmt | match-stmt | return-stmt | assign-stmt | expr


stmts:  stmt |
        stmt stmts

func-body:  LBRACE stmts RBRACE |
            stmt


func-arg-body-default:  IDENT COLON type-decl ASSIGN expr |
                        IDENT COLON type-decl ASSIGN expr COMMA func-arg-body-default

func-arg-body:  IDENT COLON type-decl |
                IDENT COLON type-decl COMMA func-arg-body |
                func-arg-body-default

func-args:  LPAREN RPAREN |
            LPAREN func-arg-body RPAREN


func-return: ARROW type-decl

func-outer-body:    ASSIGN expr |
                    LBRACE func-body RBRACE

func-decl:  DEF IDENT func-args func-return func-outer-body |
            DEF IDENT func-args func-outer-body |
            DEF IDENT func-return func-outer-body |
            DEF IDENT func-outer-body


body-decl:  typedef-decl | 
            func-decl

body-decls: body-decl | 
            body-decl body-decls


program-decl:   import-decls body-decls | 
                body-decls