#include "cthulhu.h"

#include <iostream>
#include <fstream>

#include <fmt/format.h>

namespace {
    auto grammar2 = R"(
        unit    <- decl* { no_ast_opt }

        # import syntax
        # import  <- USING LIST(ident, COLON2) items SEMI { no_ast_opt }
        # items   <- LPAREN CSV(ident) RPAREN  { no_ast_opt }

        # toplevel declarations
        decl        <- attribs* (alias / variant / union / record / func / var SEMI) { no_ast_opt }

        attribs     <- AT (attrib / '[' CSV(attrib) ']')
        attrib      <- qualified call?

        alias       <- USING ident '=' type SEMI
        union       <- UNION ident '{' fields? '}' { no_ast_opt }
        record      <- RECORD ident '{' fields? '}' { no_ast_opt }
        variant     <- VARIANT ident result? '{' CSV(case)? '}' { no_ast_opt }
        func        <- DEF ident fparams? result? body { no_ast_opt }
        fparams     <- '(' CSV(param)? ')'
        result      <- COLON type
        param       <- attribs* ident COLON (type / DOT3) init?
        body        <- (ASSIGN expr)? SEMI / compound
        var         <- VAR names init?
        names       <- qualified '(' CSV(ident) ')' / name / '[' CSV(name) ']'
        name        <- ident (COLON type)?
        init        <- ASSIGN expr

        # tagged union syntax
        case    <- ident data? ('=' expr)?
        data    <- '(' LIST(field, ',') ')'

        field       <- attribs* ident ':' type bitfield?
        bitfield    <- '[' LIST(bitrange, ',') ']'
        bitrange    <- expr (DOT2 expr)?
        fields      <- LIST(field, ',') { no_ast_opt }

        # type syntax
        type        <- attribs* (basictype error? / error)

        basictype   <- (pointer / array / closure / qualified / mutable)
        error       <- NOT (type / sum)?
        sum         <- '(' LIST(type, '/') ')'

        mutable     <- 'var' type
        array       <- '[' type (':' expr)? ']'
        pointer     <- '*' type { no_ast_opt }
        closure     <- '(' LIST(type, ',')? ')' '->' type
        qualified   <- LIST(ident, '::')

        # statements

        stmt        <- compound / return / break / continue / while / if / guard / expr ';' / switch / assign / for / asm / raise
        for         <- 'for' range else?
        range       <- '(' names DOT2 expr ')' stmt / names DOT2 expr compound
        while       <- 'while' label? cond else?
        continue    <- 'continue' ';'
        break       <- 'break' ident? ';'
        return      <- 'return' expr? ';'
        compound    <- '{' stmt* '}'
        if          <- 'if' cond elif* else?
        elif        <- 'else' 'if' cond
        else        <- 'else' stmt
        label       <- ':' ident
        cond        <- expr compound / '(' expr ')' stmt / first condition? compound / '(' first ')' stmt
        first       <- 'var' names '=' expr
        guard       <- var compound? ';'
        switch      <- 'switch' match
        match       <- expr '{' branches '}' / '(' expr ')' branches / 'var' names '=' expr condition? '{' branches '}'
        branches    <- branch* default?
        branch      <- expr ':' stmt*
        default     <- 'else' ':' stmt*
        assign      <- expr asop expr ';'
        condition   <- 'when' expr
        raise       <- 'raise' expr ';'

        asop        <- < '=' / '+=' / '-=' / '/=' / '*=' / '%=' / '&=' / '|=' / '^=' / '<<=' / '>>=' >

        asm         <- 'asm' '{' opcode* '}'
        opcode      <- ident (LIST(operand, ',')? ';' / ':')
        operand     <- soperand (':' soperand)?
        soperand    <- expr / '[' expr ']' / DOT expr

        expr <- OP(bexpr)

        # expressions
        bexpr    <- prefix (binop prefix)* {
                precedence
                    L NOT
                    L || &&
                    L & |
                    L ^
                    L EQ NEQ
                    L < <= > >=
                    L << >>
                    L + -
                    L / % *
            }

        binop    <- < '+' !'=' 
                / '-' !'=' 
                / '*' !'=' 
                / '/' !'=' 
                / '%' !'=' 
                / '&&'
                / '||'
                / '&' !('=' / '&')
                / '|' !('=' / '|')
                / NEQ / NOT
                / EQ
                / '<<' !'=' 
                / '>>' !'=' / '<=' / '>=' / '<' / '>'
            >

        unop   <- < '!' / '+' / '-' / '*' / '&' >

        atom <- 'try'? (number / qualified / 'true' / 'false' / string / OP('(') expr OP(')') / lambda) postfix*

        prefix  <- atom / OP(unop) prefix / '{' LIST(arg, ',')? '}'
        postfix <- '[' expr ']' / DOT ident / OP('->') ident / call / ternary / 'as' type
        call    <- OP('(') LIST(arg, ',')? OP(')')
        ternary <- OP('?') expr? OP(':') expr
        arg     <-  (DOT ident ASSIGN)? expr
        lambda  <- '[' CSV(capture)? ']' fparams? lresult? lbody
        capture     <- '&'? qualified
        lresult     <- '->' type
        lbody       <- expr / compound

        # basic blocks
        number  <- < (base2 / base10 / base16) ident? > ~spacing

        base10  <- < [0-9]+ >
        base2   <- < '0b' [01]+ >
        base16  <- < '0x' [0-9a-fA-F]+ >

        string  <- < ['] CHARS([']) ['] / '"""' CHARS(["""]) '"""' / ["] CHARS(["]) ["] >
        CHARS(D)    <- (!D char)*
        char    <- '\\' [nrt'"\[\]\\] / !'\\' .

        # keywords
        ~RECORD     <- 'record'
        ~UNION      <- 'union'
        ~USING      <- 'using'
        ~DEF        <- 'def'
        ~VARIANT    <- 'variant'
        ~VAR        <- 'var'

        # reserved keywords
        ~LET        <- 'let'
        ~FINAL      <- 'final'
        ~COMPILE    <- 'compile'

        KEYWORD <- RECORD / UNION / USING / DEF / VARIANT / VAR
                    / LET / FINAL / COMPILE

        # symbols
        ~SEMI       <- ';'
        ~COMMA      <- ','
        ~COLON      <- ':' !':'
        ~COLON2     <- '::'
        ~DOT        <- '.' !'.'
        ~DOT2       <- '..' !'.'
        ~DOT3       <- '...'
        ~ASSIGN     <- '=' !'='
        ~EQ         <- '=='
        ~NEQ        <- '!='
        ~NOT        <- '!' !'='
        ~AT         <- '@'
        ~LPAREN     <- '('
        ~RPAREN     <- ')'

        ident   <- !KEYWORD < [a-zA-Z_][a-zA-Z0-9_]* / '$' > 

        %whitespace <- spacing
        %word       <- ident

        spacing     <- (comment / space)*
        space       <- [ \t\r\n]
        comment     <- '#' (!line .)* line?
        line        <- [\r\n]+

        OP(I)       <- I ~spacing
        LIST(I, D)  <- I (D I)*
        CSV(I)      <- LIST(I, COMMA)
    )";
}

namespace cthulhu {

using namespace peg;
using namespace peg::udl;

namespace {
    // grammar consumed by cpp-peglib
    auto grammar = R"(
        # toplevel unit
        unit    <- decl+ { no_ast_opt }

        # declarations
        decl    <- using / record
        using   <- USING ident ASSIGN type SEMI
        record  <- RECORD ident LBRACE fields RBRACE { no_ast_opt }

        fields  <- field (COMMA field)* { no_ast_opt }
        field   <- ident COLON type

        # types
        type    <- ident

        # basic blocks

        ident   <- !KEYWORD < [a-zA-Z_][a-zA-Z0-9_]* >

        %whitespace <- [ \t\r\n]*

        # keywords
        ~USING      <- 'using'
        ~RECORD     <- 'record'

        KEYWORD <- USING / RECORD

        # operators
        ~ASSIGN <- '='
        ~SEMI   <- ';'
        ~LBRACE <- '{'
        ~RBRACE <- '}'
        ~COLON  <- ':'
        ~COMMA  <- ','
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
