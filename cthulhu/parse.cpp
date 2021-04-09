#include "cthulhu.h"

namespace cthulhu {

using namespace peg;
using namespace peg::udl;

namespace {
    // grammar consumed by cpp-peglib
    auto grammar = R"(
        unit    <- decl+ eof { no_ast_opt }

        # toplevel decls
        decl    <- struct / alias / function / variable 

        struct  <- RECORD ident lbrace LIST(field) rbrace { no_ast_opt }
        alias   <- USING ident assign type semi { no_ast_opt }

        case    <- ident fields? { no_ast_opt }
        fields  <- lparen LIST(field) rparen { no_ast_opt }
        field   <- ident colon type { no_ast_opt }

        variable    <- VAR ident colon type assign expr semi { no_ast_opt }

        function    <- DEF ident params colon type body { no_ast_opt }

        params      <- lparen LIST(field) rparen { no_ast_opt }
        body        <- semi / compound { no_ast_opt }

        # statements

        stmt    <- compound / expr assign expr semi / expr semi / return / variable { no_ast_opt }

        compound    <- lbrace stmt* rbrace { no_ast_opt }
        return      <- RETURN expr? semi { no_ast_opt }

        # expressions

        expr    <- suffix (bin suffix)* {
            precedence
                L * / %
                L + -
                L << >>
                L < <= > >=
                L == !=
                L & | ^
                L && ||
        }

        bin   <- < add / sub / mul / div / mod / shl / shr / lte / gte / lt / gt / eq / neq / and / or / xor / bitand / bitor >
        un   <- < add / sub / bitand / mul / not / flip >

        suffix  <- atom (call / subscript)*

        call    <- lparen LIST(expr)? rparen
        subscript   <- lsquare expr rsquare

        atom    <- number / ident / lparen expr rparen / mul expr / un expr

        # types
        type    <- ident / pointer { no_ast_opt }
        pointer <- mul type { no_ast_opt }

        # symbols
        ~lbrace <- '{'
        ~rbrace <- '}'
        ~lparen <- '('
        ~rparen <- ')'
        ~lsquare    <- '['
        ~rsquare    <- ']'
        ~colon  <- ':'
        ~comma  <- ','
        ~semi   <- ';'
        ~assign <- '='

        # operators
        ~add    <- '+'
        ~sub    <- '-'
        ~mul    <- '*'
        ~div    <- '/'
        ~mod    <- '%'
        ~shl    <- '<<'
        ~shr    <- '>>'
        ~gte    <- '>='
        ~lte    <- '<='
        ~gt     <- '>'
        ~lt     <- '<'
        ~eq     <- '=='
        ~neq    <- '!='
        ~and    <- '&&'
        ~or     <- '||'
        ~xor    <- '^'
        ~bitand <- '&'
        ~bitor  <- '|'
        ~not    <- '!'
        ~flip   <- '~'

        # keywords
        ~USING      <- < 'using' skip >
        ~RECORD     <- < 'record' skip >
        ~DEF        <- < 'def' skip >
        ~RETURN     <- < 'return' skip >
        ~VAR        <- < 'var' skip >

        # reserved keywords
        ~COMPILE <- 'compile' skip

        KEYWORD <- USING / RECORD / DEF / RETURN / VAR / COMPILE

        # an identifier is any sequence of [a-zA-Z_][a-zA-Z0-9_] or a single $
        # that is *not* a keyword
        ident   <- < !KEYWORD [a-zA-Z_][a-zA-Z0-9_]* / '$' > 

        number  <-  (base16 / base2 / base10) ident?

        base2   <- < '0b' [01]+ >
        base16  <- < '0x' [0-9a-fA-F]+ >
        base10  <- < [0-9]+ >

        %whitespace <- space*

        # macros
        LIST(I) <- I (comma I)*

        # whitespace handling
        ~comment <- '#' (!line .)* line?
        ~line    <- [\r\n]
        ~blank   <- [ \t]
        ~space   <- (blank / line / comment)
        ~skip    <- ![a-zA-Z_] space
        ~eof     <- !.
    )";

    // our global parser instance
    peg::parser parser;
}

void init() {
    parser.log = [](auto line, auto col, const auto& msg) {
        fmt::print("{}:{}: {}\n", line, col, msg);
    };

    if (!parser.load_grammar(grammar)) {
        panic("failed to load grammar");
    }

    parser.enable_packrat_parsing();
    parser.enable_ast();
}

Context::Context(std::string source): text(source) {
    if (!parser.parse(text, tree)) {
        panic("failed to parse source");
    }

    tree = parser.optimize_ast(tree);

    std::cout << ast_to_s(tree) << std::endl;
}


}