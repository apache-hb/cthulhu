#include "cthulhu.h"

using namespace std;
using namespace peg;
using namespace peg::udl;

auto grammar = R"(
unit    <- decl+ eof { no_ast_opt }

decl    <- def
def     <- DEF ident args? result? func { no_ast_opt }

func    <- ';' / '=' expr ';' / '{' '}' { no_ast_opt }

result  <- COLON type

args    <- LPAREN LIST(arg)? RPAREN { no_ast_opt }
arg     <- ident COLON type

expr    <- binary '?' expr ':' expr / binary

binary    <- unary (binop unary)* {
    precedence
        L <= < > >=
        L + -
        L * / %
}

binop   <- < '+' / '-' / '*' / '/' / '%' / '<=' / '<' / '>=' / '>' >
unop    <- < '+' / '-' / '*' / '&' / '!' / '~' >

unary   <- unop expr / atom 
atom    <- (number / ident / LPAREN expr RPAREN) call* { no_ast_opt }

call    <- LPAREN LIST(expr)? RPAREN { no_ast_opt }

type    <- ident

# operators
~COLON      <- ':'
~COMMA      <- ','
~LPAREN     <- '('
~RPAREN     <- ')'

# keywords
~DEF        <- < 'def' skip >
~KW     <- DEF

# identifiers cannot be keywords
ident   <- !KW < [a-zA-Z_][a-zA-Z0-9_]* / '$' >

LIST(R) <- R (COMMA R)* { no_ast_opt }

# numbers
number  <- < [0-9]+ >

# cpp-peglib extensions
%whitespace <- space*

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

    std::string ident(A a) {
        TAG("ident"_);
        return a->token_to_string();
    }

    Literal* number(A a) {
        TAG("number"_);

        return new Literal(a->token_to_number<size_t>());
    }

    Type* type(A a) {
        return new Sentinel(ident(a));
    }

    Expr* expr(A a);
    Binary* binary(A a);
    Expr* body(A a);

    Ternary* ternary(A a) {
        auto cond = body(a->nodes[0]);
        auto yes = expr(a->nodes[1]);
        auto no = expr(a->nodes[2]);
        return new Ternary(cond, yes, no);
    }

    Unary* unary(A a) {
        TAG("unary"_);

        auto op = [](auto node) -> Unary::Op {
            if (node->token == "-") {
                return Unary::NEG;
            } else if (node->token == "+") {
                return Unary::POS;
            } else if (node->token == "&") {
                return Unary::REF;
            } else if (node->token == "*") {
                return Unary::DEREF;
            } else if (node->token == "!") {
                return Unary::NOT;
            } else if (node->token == "~") {
                return Unary::FLIP;
            } else {
                panic("unknown unary op `{}`", node->token);
            }
        };

        auto it = expr(a->nodes[1]);

        return new Unary(op(a->nodes[0]), it);
    }

    Binary* binary(A a) {
        TAG("binary"_);

        auto op = [](auto node) -> Binary::Op {
            if (node->token == "+") {
                return Binary::ADD;
            } else if (node->token == "-") {
                return Binary::SUB;
            } else if (node->token == "/") {
                return Binary::DIV;
            } else if (node->token == "*") {
                return Binary::MUL;
            } else if (node->token == "%") {
                return Binary::REM;
            } else if (node->token == "<") {
                return Binary::LT;
            } else if (node->token == "<=") {
                return Binary::LTE;
            } else if (node->token == ">") {
                return Binary::GT;
            } else if (node->token == ">=") {
                return Binary::GTE;
            } else {
                panic("unknown binary op `{}`", node->token);
            }
        };

        auto lhs = body(a->nodes[0]);
        auto rhs = body(a->nodes[2]);
        auto o = op(a->nodes[1]);

        return new Binary(o, lhs, rhs);
    }

    Expr* atom(A a) {
        TAG("atom"_);

        Expr* out = body(a->nodes[0]);

        for (size_t i = 1; i < a->nodes.size(); i++) {
            std::vector<Expr*> args;
            for (auto node : a->nodes[i]->nodes) {
                args.push_back(expr(node));
            }
            out = new Call(out, args);
        }

        return out;
    }

    Name* name(A a) {
        return new Name(ident(a));
    }

    Expr* body(A a) {
        switch (a->tag) {
        case "number"_: return number(a);
        case "unary"_: return unary(a);
        case "binary"_: return binary(a);
        case "atom"_: return atom(a);
        case "ident"_: return name(a);
        default: 
            panic("unknown body `{}`", a->name);
        }
    }

    Expr* expr(A a) {
        switch (a->original_choice) {
        case 0: return ternary(a);
        case 1: return body(a);
        default: 
            panic("unkonwn expr `{}`", a->name);
        }
    }

    Function* def(A a) {
        TAG("def"_);
        auto name = ident(a->nodes[0]);

        auto args = [=](auto node) {
            Params out;

            if (node->tag != "args"_) {
                return out;
            }

            for (auto each : node->nodes) {
                auto id = ident(each->nodes[0]);
                auto it = type(each->nodes[1]);

                out.push_back({ id, it });
            }

            return out;
        }(a->nodes[1]);

        auto result = [=](auto node) -> Type* {
            if (a->nodes.size() == 2) {
                return new Sentinel("void");
            }
            if (node->tag == "type"_ || node->tag == "ident"_) {
                return type(node);
            } else {
                return new Sentinel("void");
            }
        }(a->nodes[a->nodes.size() - 2]);

        auto last = a->nodes.back();

        switch (last->original_choice) {
        case 0: return new EmptyFunction(name, args, result);
        case 1: return new LinearFunction(name, args, result, expr(last->nodes[0]));
        default: panic("unknown function body `{}`", last->name);
        }
    }

    Decl* decl(A a) {
        switch (a->choice) {
        case 0: return def(a);
        default: panic("unknown decl `{}`", a->name);
        }
    }

    void unit(C* c, A a) {
        TAG("unit"_);
        for (auto node : a->nodes) {
            c->add(decl(node));
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
