#include "cthulhu.h"

using namespace std;
using namespace peg;
using namespace peg::udl;

auto grammar = R"(
unit    <- decl+ eof { no_ast_opt }

decl    <- def
def     <- DEF ident args? result? func

func    <- ';' / '=' expr ';' / '{' '}'

result  <- COLON type

args    <- LPAREN LIST(arg)? RPAREN { no_ast_opt }
arg     <- ident COLON type

expr    <- body '?' expr ':' expr / body

body    <- unary (binop unary)* {
    precedence
        L <= < > >=
        L + -
        L * / %
}

binop   <- < '+' / '-' / '*' / '/' / '%' / '<=' / '<' / '>=' / '>' >
unop    <- < '+' / '-' / '*' / '&' / '!' / '~' >

unary   <- unop expr / atom
atom    <- (number / ident / LPAREN expr RPAREN) call*

call    <- LPAREN LIST(expr)? RPAREN

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

    Type* type(C*, A a) {
        return new Sentinel(ident(a));
    }

    Expr* expr(C*, A) {
        return nullptr;
    }

    Function* def(C* c, A a) {
        TAG("def"_);
        auto name = ident(a->nodes[0]);

        auto args = [=](auto node) {
            Params out;

            if (node->tag != "args"_) {
                return out;
            }

            for (auto each : node->nodes) {
                auto id = ident(each->nodes[0]);
                auto it = type(c, each->nodes[1]);

                out.push_back({ id, it });
            }

            return out;
        }(a->nodes[1]);

        auto result = [=](auto node) -> Type* {
            if (node->tag == "type"_) {
                return type(c, node);
            } else {
                return new Sentinel("void");
            }
        }(a->nodes[a->nodes.size() - 2]);

        auto last = a->nodes.back();

        switch (last->original_choice) {
        case 0: return new EmptyFunction(name, args, result);
        case 1: return new LinearFunction(name, args, result, expr(c, a));
        default: panic("unknown function body `{}`", last->name);
        }
    }

    Decl* decl(C* c, A a) {
        switch (a->choice) {
        case 0: return def(c, a);
        default: panic("unknown decl `{}`", a->name);
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
