%{
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern int yylex(void);
extern int yyparse();
extern FILE* yyin;

void yyerror(const char*);

typedef struct node_t {
    enum { INT, BINARY } kind;

    union {
        uint64_t value;
        struct {
            char op;
            struct node_t *lhs;
            struct node_t *rhs;
        };
    };
} node_t;

uint64_t eval(node_t*);

static node_t* 
new_int(uint64_t value) 
{
    node_t *out = malloc(sizeof(node_t));
    out->kind = INT;
    out->value = value;
    return out;
}

static node_t*
new_binary(char op, node_t *lhs, node_t *rhs)
{
    node_t *out = malloc(sizeof(node_t));

    out->kind = BINARY;
    out->op = op;
    out->lhs = lhs;
    out->rhs = rhs;

    return out;
}

%}

%require "3.2"
%define parse.error verbose

%union {
    uint64_t digit;
    const char *ident;
    struct node_t *node;
}

%token<digit> DIGIT
%token<ident> IDENT
%token ADD SUB MUL DIV REM LPAREN RPAREN
%token SEMI LBRACE RBRACE
%left ADD SUB
%left MUL DIV REM
%left LPAREN RPAREN

%type<node> expr

%start unit

%%

/* a full compilation unit */
unit:
    | unit stmt
    ;

/* a single statement */
stmt: LBRACE compound RBRACE
    | expr SEMI { printf("result: %lu\n", eval($1)); }
    ;

/* zero or more statements */
compound: 
    | stmt compound
    ;

/* a binary expression */
expr: DIGIT[it] { $$ = new_int($it); }
    | expr[lhs] ADD expr[rhs] { $$ = new_binary('+', $lhs, $rhs); }
    | expr[lhs] SUB expr[rhs] { $$ = new_binary('-', $lhs, $rhs); }
    | expr[lhs] MUL expr[rhs] { $$ = new_binary('*', $lhs, $rhs); }
    | expr[lhs] DIV expr[rhs] { $$ = new_binary('/', $lhs, $rhs); }
    | expr[lhs] REM expr[rhs] { $$ = new_binary('%', $lhs, $rhs); }
    | LPAREN expr[it] RPAREN { $$ = $it; }
    | IDENT { $$ = new_int(0); }
    ;

%%

uint64_t 
eval(node_t *node)
{
    switch (node->kind) {
    case INT: return node->value;
    case BINARY: switch (node->op) {
        case '+': return eval(node->lhs) + eval(node->rhs);
        case '-': return eval(node->lhs) - eval(node->rhs);
        case '*': return eval(node->lhs) * eval(node->rhs);
        case '/': return eval(node->lhs) / eval(node->rhs);
        case '%': return eval(node->lhs) % eval(node->rhs);
        default:
            fprintf(stderr, "unknown op `%c`\n", node->op);
            exit(1);
    }
    default: 
        fprintf(stderr, "unknown node kind\n");
        exit(1);
    }
}

int main(int argc, const char **argv) {
    if (argc < 2) {
        fprintf(stderr, "provide input file\n");
        return 1;
    }
    
    yyin = fopen(argv[1], "r");

    do { 
        yyparse();
    } while (!feof(yyin));

    return 0;
}

void yyerror(const char *msg) {
    fprintf(stderr, "error: %s\n", msg);
    exit(1);
}
