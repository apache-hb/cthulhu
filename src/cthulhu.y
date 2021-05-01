%{
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <vector>
#include <iostream>

extern "C" int yylex(void);
extern int yyparse();
extern FILE* yyin;
extern int yylineno;

void yyerror(const char*);

struct stmt_t {
    virtual ~stmt_t() = default;
    virtual void apply() = 0;
};

struct compound_t : stmt_t {
    virtual ~compound_t() = default;

    std::vector<stmt_t*> decls;

    compound_t(stmt_t* tail, compound_t* compound)
    { 
        if (!compound) {
            decls = { tail };
        } else {
            decls.push_back(tail);
            for (auto decl : compound->decls)
                decls.push_back(decl);
        }
    }

    virtual void apply() override {
        for (auto* decl : decls) {
            decl->apply();
        }
    }
};

struct expr_t : stmt_t {
    virtual ~expr_t() = default;
    virtual void apply() override { 
        printf("result: %" PRId64 "\n", eval());
    }
    virtual int64_t eval() = 0;
};

struct int_t : expr_t {
    virtual ~int_t() = default;
    int64_t value;

    int_t(int64_t val)
        : value(val)
    { }

    virtual int64_t eval() override { return value; }
};

struct binary_t : expr_t {
    virtual ~binary_t() = default;
    enum op_t { ADD, SUB, DIV, MUL, REM } op;
    expr_t *lhs;
    expr_t *rhs;

    binary_t(op_t op, expr_t *lhs, expr_t *rhs)
        : op(op)
        , lhs(lhs)
        , rhs(rhs)
    { }

    virtual int64_t eval() override { 
        switch (op) {
        case ADD: return lhs->eval() + rhs->eval();
        case SUB: return lhs->eval() - rhs->eval();
        case DIV: return lhs->eval() / rhs->eval();
        case MUL: return lhs->eval() * rhs->eval();
        case REM: return lhs->eval() % rhs->eval();
        default:
            fprintf(stderr, "unknown binary op\n");
            exit(1);
        }
    }
};

struct unary_t : expr_t {
    virtual ~unary_t() = default;
    enum op_t { ABS, NEG } op;
    expr_t *expr;

    unary_t(op_t op, expr_t *expr)
        : op(op)
        , expr(expr)
    { }

    virtual int64_t eval() override {
        switch (op) {
        case ABS: return llabs(expr->eval());
        case NEG: return -expr->eval();
        default:
            fprintf(stderr, "unknown unary op\n");
            exit(1);
        }
    }
};

struct decl_t {
    virtual ~decl_t() = default;
    virtual void apply() = 0;

    decl_t(std::string name)
        : name(name)
    { }

    std::string name;
};

struct func_t : decl_t {
    virtual ~func_t() = default;
    virtual void apply() override {
        std::cout << name << std::endl;
        body->apply();
    }

    compound_t *body;

    func_t(std::string name, compound_t* body)
        : decl_t(name)
        , body(body)
    { }
};

extern std::vector<decl_t*> gdecls;

%}

%require "3.2"
%define parse.error verbose
%locations

%union {
    uint64_t digit;
    const char *ident;
    struct expr_t *expr;
    struct stmt_t *stmt;
    struct decl_t *decl;
    struct compound_t *compound;
}

%token<digit> T_DIGIT
%token<ident> T_IDENT
%token T_DEF
%token T_ADD T_SUB T_MUL T_DIV T_REM T_LPAREN T_RPAREN
%token T_SEMI T_LBRACE T_RBRACE
%left T_ADD T_SUB
%left T_MUL T_DIV T_REM

%type<stmt> stmt
%type<decl> func
%type<compound> compound
%type<expr> expr

%start unit

%%

/* a full compilation unit */
unit: func { gdecls.push_back($1); }
    | unit func { gdecls.push_back($2); }
    ;

func: T_DEF T_IDENT[name] T_LBRACE compound[body] T_RBRACE { $$ = new func_t($name, $body); }
    ;

/* a single statement */
stmt: T_LBRACE compound[it] T_RBRACE { printf("stmt\n"); $$ = $it; }
    | expr T_SEMI { $$ = $1; }
    ;

/* zero or more statements */
compound: %empty { $$ = nullptr; }
    | stmt compound { $$ = new compound_t($1, $2); }
    ;

/* a binary expression */
expr: T_DIGIT[it] { $$ = new int_t($it); }
    | expr[lhs] T_ADD expr[rhs] { $$ = new binary_t(binary_t::ADD, $lhs, $rhs); }
    | expr[lhs] T_SUB expr[rhs] { $$ = new binary_t(binary_t::SUB, $lhs, $rhs); }
    | expr[lhs] T_MUL expr[rhs] { $$ = new binary_t(binary_t::MUL, $lhs, $rhs); }
    | expr[lhs] T_DIV expr[rhs] { $$ = new binary_t(binary_t::DIV, $lhs, $rhs); }
    | expr[lhs] T_REM expr[rhs] { $$ = new binary_t(binary_t::REM, $lhs, $rhs); }
    | T_LPAREN expr[it] T_RPAREN { $$ = $it; }
    | T_IDENT { $$ = new int_t(0); }
    | T_ADD expr[it] { $$ = new unary_t(unary_t::ABS, $it); }
    | T_SUB expr[it] { $$ = new unary_t(unary_t::NEG, $it); }
    ;

%%

std::vector<decl_t*> gdecls = {};

int main(int argc, const char **argv) {
    if (argc < 2) {
        fprintf(stderr, "provide input file\n");
        return 1;
    }
    
    yyin = fopen(argv[1], "r");

    do { 
        yyparse();
    } while (!feof(yyin));

    for (auto decl : gdecls) {
        decl->apply();
    }

    return 0;
}

void yyerror(const char *msg) {
    fprintf(stderr, "error: %s\n", msg);
    exit(1);
}
