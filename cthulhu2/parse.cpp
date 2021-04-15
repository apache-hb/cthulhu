#include "cthulhu.h"

using namespace std;
using namespace peg;
using namespace peg::udl;

auto grammar = R"(
unit    <- decl+ eof { no_ast_opt }

decl    <- def
def     <- DEF ident ASSIGN expr SEMI

expr    <- atom (binop atom)* {
    precedence
        L * / %
        L + -
}

binop   <- < '+' / '-' / '*' / '/' / '%' >

atom    <- number / ident / LPAREN expr RPAREN

# operators
~ASSIGN <- '='
~SEMI   <- ';'
~LPAREN <- '('
~RPAREN <- ')'

# keywords
~DEF        <- < 'def' skip >
~KW     <- DEF

# identifiers cannot be keywords
ident   <- !KW < [a-zA-Z_][a-zA-Z0-9_]* / '$' >

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

#define TAG(id) if (a->tag != id) { panic("expected ast tag " #id " but got `{}` instead", a->name); }

namespace p {
    using namespace ctu;

    using C = ctu::Context;
    using A = std::shared_ptr<peg::Ast>;

    std::string ident(A a) {
        TAG("ident"_);
        return a->token_to_string();
    }

    void def(C*, A a) {
        TAG("def"_);
        std::cout << ast_to_s(a) << std::endl;

        auto name = ident(a->nodes[0]);
    }

    void decl(C* c, A a) {
        switch (a->choice) {
        case 0: def(c, a); break;
        default: panic("unknown node `{}`", a->name);
        }
    }

    void unit(C* c, A a) {
        TAG("unit"_);
        for (auto node : a->nodes) {
            decl(c, node);
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
