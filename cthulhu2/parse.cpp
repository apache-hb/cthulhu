#include "ast.h"
#include "cthulhu.h"

#include <peglib.h>

using namespace peg;

auto grammar = R"(
unit    <- decl+ eof { no_ast_opt }

decl    <- def
def     <- DEF ident args result func { no_ast_opt }

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
~KW         <- DEF

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

namespace {
    using A = std::shared_ptr<peg::Ast>;
    using C = ctu::Context;

    void unit(C*, A node) {
        for (auto each : node->nodes) {
            std::cout << ast_to_s(each) << std::endl;
        }
    }
}

namespace ctu {
    peg::parser reader;

    void init() {
        reader.log = [](auto line, auto col, const auto& msg) {
            fmt::print("{}:{}: {}\n", line, col, msg);
        };

        if (!reader.load_grammar(grammar)) {
            panic("failed to load grammar");
        }

        reader.enable_packrat_parsing();
        reader.enable_ast();
    }

    Context parse(const std::string& source) {
        std::shared_ptr<Ast> ast;
        if (!reader.parse(source, ast)) {
            panic("failed to parse source");
        }

        ast = reader.optimize_ast(ast);

        Context ctx;
        unit(&ctx, ast);

        return ctx;
    }
}
