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

        type    <- pointer / mutable / closure / ident
        pointer <- mul type
        mutable <- var lparen type rparen
        closure <- lparen LIST(type) rparen arrow type

        ~lbrace  <- '{'
        ~rbrace  <- '}'
        ~lparen  <- '('
        ~rparen  <- ')'
        ~assign  <- '='
        ~semi    <- ';'
        ~colon   <- ':'
        ~mul     <- '*'
        ~arrow   <- '->'
        ~comma   <- ','

        ~using   <- 'using'
        ~record  <- 'record'
        ~var     <- 'var'

        keyword <- using / record / var

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
}

void Context::resolve() {
    auto decl = [&](std::shared_ptr<Ast> ast) {
        // all fields in this type decl
        auto fields = ast->nodes[1]->nodes;

        for (auto field : fields) {
            // the name and type of the field
            auto fname = field->nodes[0]->token_to_string();
            auto ftype = field->nodes[1]->token_to_string();

            // defer looking up the type of the field
            // for the second phase
            defer(ftype);
        }
    };

    auto alias = [&](std::shared_ptr<Ast> ast) {
        auto type = ast->nodes[1]->token_to_string();
        defer(type);
    };

    auto parse = [&](std::shared_ptr<Ast> ast) {
        // the name of this type decl
        auto name = ast->nodes[0]->token_to_string();

        if (ast->tag == "using"_) {
            alias(ast);
        } else {
            decl(ast);  
        }

        // define this decl for the second phase
        define(name);
    };

    // add all decls to the symbol table
    for (auto node : tree->nodes) {
        parse(node);
    }

    for (const auto& symbol : symbols) {
        if (symbol.type == SymbolType::DEFER) {
            throw std::runtime_error(fmt::format("failed to resolve symbol `{}`", symbol.name));
        }
    }
}

void Context::define(const std::string& name) {
    // if the symbol has already been deferred then replace it
    for (auto& symbol : symbols) {
        if (symbol.name == name) {
            if (symbol.type == SymbolType::DEFINE) {
                throw std::runtime_error(fmt::format("symbol `{}` was already defined", name));
            }
            
            symbol.type = SymbolType::DEFINE;
            return;
        }
    }

    // otherwise define this symbol
    symbols.push_back({ SymbolType::DEFINE, name });
}

void Context::defer(const std::string& name) {
    if (!lookup(name)) {
        symbols.push_back({ SymbolType::DEFER, name });
    }
}

bool Context::lookup(const std::string& name) {
    for (const auto& symbol : symbols) {
        if (symbol.name == name) {
            return symbol.type == SymbolType::DEFINE;
        }
    }

    return false;
}

}
