#include "cthulhu.h"

#include <iostream>
#include <fstream>

#include <fmt/format.h>

namespace cthulhu {

using namespace peg;
using namespace peg::udl;

namespace {
    // grammar consumed by cpp-peglib
    auto grammar = R"(
        unit    <- decl* !. { no_ast_opt }

        decl    <- alias / struct
        alias   <- using ident assign type semi
        struct  <- record ident lbrace LIST(field) rbrace

        field   <- ident colon type

        type    <- pointer / ident
        pointer <- mul type

        ~lbrace  <- '{'
        ~rbrace  <- '}'
        ~assign  <- '='
        ~semi    <- ';'
        ~colon   <- ':'
        ~mul     <- '*'
        ~comma   <- ','

        ~using   <- 'using'
        ~record  <- 'record'

        keyword <- using / record

        %whitespace <- spacing

        ident   <- !keyword < [a-zA-Z_][a-zA-Z0-9_]* / '$' >
        ~spacing <- (space / comment)*
        space   <- [ \t\r\n]
        comment <- '#' (!newline .)* newline?
        newline <- [\r\n]+

        LIST(R) <- R (comma R)*
    )";

    // our global parser instance
    peg::parser parser;
}

void init() {
    parser.log = [](auto line, auto col, const auto& msg) {
        fmt::print("{}:{}: {}\n", line, col, msg);
    };

    if (!parser.load_grammar(grammar)) {
        throw std::runtime_error("failed to load grammar");
    }

    parser.enable_packrat_parsing();
    parser.enable_ast();
}

Context::Context(std::string source) : text(source) {
    if (!parser.parse(text, tree)) {
        throw std::runtime_error("failed to parse source");
    }

    tree = parser.optimize_ast(tree);

    std::cout << ast_to_s(tree) << std::endl;
}

}
