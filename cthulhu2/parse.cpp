#include "ast.h"
#include "cthulhu.h"

#include <peglib.h>

using namespace peg;

auto grammar = R"(
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
atom    <- (number / boolean / ident / LPAREN expr RPAREN) call*

call    <- LPAREN LIST(expr)? RPAREN

type    <- '*' type / ident

# operators
~COMMA      <- ','
~LPAREN     <- '('
~RPAREN     <- ')'

# keywords

# identifiers cannot be keywords
ident   <- < [a-zA-Z_][a-zA-Z0-9_]* / '$' >

LIST(R) <- R (COMMA R)*

# numbers
number  <- < [0-9]+ >
boolean <- < 'true' / 'false' >

# cpp-peglib extensions
%whitespace <- space*

# whitespace and comment handling
~space      <- blank / line / lcomment / mcomment
~blank      <- [ \t]
~line       <- [\r\n]
~lcomment   <- '//' (!line .)* line?
~mcomment   <- '/*' (mcomment / !'*/' .)* '*/'

)";

#define TAG(id) if (node->tag != id) { std::cout << ast_to_s(node) << std::endl; panic("expected ast tag " #id " but got `{}` instead", node->name); }

using namespace std;

namespace ctu {
    peg::parser reader;

    void init() {
        reader.log = [](auto line, auto col, const auto& msg) {
            fmt::print("{}:{}: {}\n", line, col, msg);
        };

        if (!reader.load_grammar(grammar)) {
            panic("failed to load grammar");
        }

        reader["number"] = [](const peg::SemanticValues& vs) -> Expr* {
            fmt::print("number\n");
            return new IntLiteral(vs.token_to_number<size_t>());
        };

        reader["boolean"] = [](const peg::SemanticValues& vs) -> Expr* {
            fmt::print("boolean\n");
            return new BoolLiteral(vs.choice() == 0);
        };

        reader["binop"] = [](const peg::SemanticValues& vs) -> Binary::Op {
            fmt::print("binop\n");
            if (vs.sv() == "+") {
                return Binary::ADD;
            } else if (vs.sv() == "-") {
                return Binary::SUB;
            } else if (vs.sv() == "/") {
                return Binary::DIV;
            } else if (vs.sv() == "*") {
                return Binary::MUL;
            } else if (vs.sv() == "%") {
                return Binary::REM;
            } else {
                panic("unknown binop `{}`", vs.sv());
            }
        };

        reader["binary"] = [](const peg::SemanticValues& vs) -> Expr* {
            fmt::print("binary\n");
            Expr* out = any_cast<Expr*>(vs[0]);
            if (vs.size() > 1) {
                auto op = any_cast<Binary::Op>(vs[1]);
                auto other = any_cast<Expr*>(vs[2]);

                out = new Binary(op, out, other);
            }
            return out;
        };

        reader["unop"] = [](const peg::SemanticValues& vs) -> Unary::Op {
            fmt::print("unop\n");
            if (vs.sv() == "+") {
                return Unary::POS;
            } else if (vs.sv() == "-") {
                return Unary::NEG;
            } else if (vs.sv() == "&") {
                return Unary::REF;
            } else if (vs.sv() == "*") {
                return Unary::DEREF;
            } else if (vs.sv() == "!") {
                return Unary::NOT;
            } else if (vs.sv() == "~") {
                return Unary::FLIP;
            } else {
                panic("unknown unop `{}`", vs.sv());
            }
        };

        reader["unary"] = [](const peg::SemanticValues& sv) -> Expr* {
            fmt::print("unary\n");
            if (sv.choice() == 1) {
                return any_cast<Expr*>(sv[0]);
            } else {
                auto op = any_cast<Unary::Op>(sv[0]);
                auto expr = any_cast<Expr*>(sv[1]);
                return new Unary(op, expr);
            }
        };

        reader["call"] = [](const peg::SemanticValues& vs) -> Args {
            fmt::print("call\n");
            Args out;
            for (auto node : vs) {
                out.push_back(any_cast<Expr*>(node));
            }
            return out;
        };

        reader["atom"] = [](const peg::SemanticValues& vs) -> Expr* {
            fmt::print("atom {}\n", vs.sv());
            Expr* out = any_cast<Expr*>(vs[0]);

            for (size_t i = 1; i < vs.size(); i++) {
                out = new Call(out, any_cast<Args>(vs[i]));
            }

            return out;
        };

        reader.enable_packrat_parsing();
        reader.enable_ast();
    }

    Context parse(const std::string& source, std::vector<Symbol*> symbols) {
        Node* node;
        if (!reader.parse(source, node)) {
            panic("failed to parse source");
        }

        std::cout << node->debug() << std::endl;

        return {};
    }
}
