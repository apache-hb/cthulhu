#include "cthulhu.h"

using namespace std;
using namespace peg;
using namespace peg::udl;

auto grammar = R"(
unit    <- decl+ eof { no_ast_opt }

decl    <- def
def     <- DEF ident ASSIGN expr SEMI

expr    <- unary (binop unary)* {
    precedence
        L + -
        L * / %
}

binop   <- < '+' / '-' / '*' / '/' / '%' >
unop    <- < '+' / '-' / '*' / '&' / '!' / '~' >

unary   <- atom / unop expr
atom    <- (number / ident / LPAREN expr RPAREN) postfix* { no_ast_opt }

postfix <- call / ternary
ternary <- '?' expr ':' expr
call    <- '(' LIST(expr)? ')' 

# operators
~ASSIGN <- '='
~SEMI   <- ';'
~COMMA  <- ','
~LPAREN <- '('
~RPAREN <- ')'

# keywords
~DEF        <- < 'def' skip >
~KW     <- DEF

# identifiers cannot be keywords
ident   <- !KW < [a-zA-Z_][a-zA-Z0-9_]* / '$' >

LIST(R) <- R (COMMA R)* { no_ast_opt }

# numbers
number  <- < [0-9]+ >

# cpp-peglib extensions
%whitespace <- skip*

# whitespace and comment handling
~skip       <- ![a-zA-Z_] space
~space      <- blank / line / lcomment / mcomment
~blank      <- [ \t]
~line       <- [\r\n]
~lcomment   <- '//' (!line .)* line?
~mcomment   <- '/*' (mcomment / !'*/' .)* '*/'
~eof        <- !.
)";

peg::parser reader;

void ctu::init() {
    reader.log = [](auto line, auto col, const auto& msg) {
        fmt::print("{}:{}: {}\n", line, col, msg);
    };

    if (!reader.load_grammar(grammar)) {
        panic("failed to load grammar");
    }

    reader.enable_packrat_parsing();
    reader.enable_ast();
}

#define TAG(id) if (a->tag != id) { std::cout << ast_to_s(a) << std::endl; panic("expected ast tag " #id " but got `{}` instead", a->name); }

namespace p {
    using namespace ctu;

    using C = ctu::Context;
    using A = std::shared_ptr<peg::Ast>;

    std::string ident(C*, A a) {
        TAG("ident"_);
        return a->token_to_string();
    }

    Literal* number(C*, A a) {
        TAG("number"_);

        return new Literal(a->token_to_number<size_t>());
    }

    Expr* expr(C* c, A a);
    Expr* atom(C* c, A a);

    Binary::Op binop(C*, A a) {
        TAG("binop"_);

        auto tok = a->token;

        if (tok == "+") {
            return Binary::ADD;
        } else if (tok == "-") {
            return Binary::SUB;
        } else if (tok == "/") {
            return Binary::DIV;
        } else if (tok == "*") {
            return Binary::MUL;
        } else if (tok == "%") {
            return Binary::REM;
        }

        panic("unknown binop `{}`", tok);
    }

    Unary::Op unop(C*, A a) {
        TAG("unop"_);

        auto tok = a->token;

        if (tok == "+") {
            return Unary::POS;
        } else if (tok == "-") {
            return Unary::NEG;
        } else if (tok == "*") {
            return Unary::DEREF;
        } else if (tok == "&") {
            return Unary::REF;
        } else if (tok == "!") {
            return Unary::NOT;
        } else if (tok == "~") {
            return Unary::FLIP;
        }

        panic("unknown unop `{}`", tok);
    }

    Binary* binary(C* c, A a) {
        TAG("expr"_);

        auto op = binop(c, a->nodes[1]);
        auto lhs = expr(c, a->nodes[0]);
        auto rhs = expr(c, a->nodes[2]);

        return new Binary(op, lhs, rhs);
    }

    Name* name(C* c, A a) {
        return new Name(ident(c, a));
    }

    Expr* unary(C* c, A a) {
        TAG("unary"_);
        auto op = unop(c, a->nodes[0]);
        auto body = expr(c, a->nodes[1]);
        return new Unary(op, body);
    }

    Expr* atom(C* c, A a) {
        auto front = [c](A a) -> Expr* {
            switch (a->tag) {
            case "number"_: return number(c, a);
            case "ident"_: return name(c, a);
            case "expr"_: return expr(c, a);
            case "unop"_: return unary(c, a);
            default: panic("unknown expr `{}``", a->name);
            }
        };

        auto call = [c](auto body, A a) {
            std::vector<Expr*> args;
            for (auto node : a->nodes) {
                args.push_back(expr(c, node));
            }

            return new Call(body, args);
        };

        auto it = front(a->nodes[0]);

        for (size_t i = 1; i < a->nodes.size(); i++) {
            auto node = a->nodes[i];

            switch (node->original_choice) {
            case 0: 
                it = call(it, node);
                break;
            case 1: 
                it = new Ternary(it, expr(c, node->nodes[0]), expr(c, node->nodes[1]));
                break;
            default:
                panic("unknown postfix `{}`", node->name);
            }
        }

        return it;
    }

    Expr* expr(C* c, A a) {
        switch (a->tag) {
        case "expr"_: return binary(c, a);
        case "unary"_: return unary(c, a);
        default: return atom(c, a);
        }
    }

    Function* def(C* c, A a) {
        TAG("def"_);
        
        auto name = ident(c, a->nodes[0]);
        auto body = expr(c, a->nodes[1]);

        return new Function(name, body);
    }

    Decl* decl(C* c, A a) {
        switch (a->choice) {
        case 0: return def(c, a);
        default: panic("unknown node `{}`", a->name);
        }
    }

    void unit(C* c, A a) {
        TAG("unit"_);
        for (auto node : a->nodes) {
            c->add(decl(c, node));
        }
    }
}

ctu::Context ctu::parse(std::string source) {
    std::shared_ptr<Ast> ast;
    if (!reader.parse(source, ast)) {
        panic("failed to parse source");
    }

    ast = reader.optimize_ast(ast);

    ctu::Context ctx;

    p::unit(&ctx, ast);

    return ctx;
}
