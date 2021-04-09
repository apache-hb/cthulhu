#include "cthulhu.h"

namespace cthulhu {

using namespace peg;
using namespace peg::udl;

namespace {
    // grammar consumed by cpp-peglib
    auto grammar = R"(
        unit    <- decl+ eof { no_ast_opt }

        # toplevel decls
        decl    <- record / alias / function / variable 

        record  <- RECORD ident lbrace fields rbrace { no_ast_opt }
        alias   <- USING ident assign type semi { no_ast_opt }

        fields  <- LIST(field) { no_ast_opt }
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

        atom    <- number / string / ident / lparen expr rparen / mul expr / un expr

        # types
        type    <- ident / pointer / array { no_ast_opt }
        pointer <- mul type
        array   <- lsquare type (colon expr)? rsquare { no_ast_opt }

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

        number  <-  (base16 / base2 / base10) ident? { no_ast_opt }

        string  <- < '"' (!["] char)* '"' >

        char    <- !'\\' . / '\\' [nrt'"\[\]\\]

        base2   <- '0b' < [01]+ >
        base16  <- '0x' < [0-9a-fA-F]+ >
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

Builder::Builder(std::string source): text(source) {
    if (!parser.parse(text, tree)) {
        panic("failed to parse source");
    }

    tree = parser.optimize_ast(tree);
}

void Builder::build(Context* ctx) {
    for (auto node : tree->nodes) {
        switch (node->tag) {
        case "record"_:
            buildRecord(ctx, node);
            break;
        default:
            panic("TODO: unknown tag `{}`", node->name);
        }
    }
}

void Builder::buildRecord(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "record"_);

    std::cout << ast_to_s(ast) << std::endl;
    
    auto name = ast->nodes[0]->token_to_string();
    auto fields = buildFields(ctx, ast->nodes[1]);

    ctx->add(std::make_shared<ast::RecordType>(name, fields));
}

ast::Fields Builder::buildFields(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "fields"_);

    ast::Fields fields;

    for (auto node : ast->nodes) {
        auto name = node->nodes[0]->token_to_string();
        auto type = buildType(ctx, node->nodes[1]);

        fields.add(name, type);
    }

    return fields;
}

std::shared_ptr<ast::Type> Builder::buildType(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "type"_);

    switch (ast->choice) {
    case 0: // ident
        return ctx->get(ast->nodes[0]->token_to_string());
    case 1: // pointer
        return std::make_shared<ast::PointerType>(buildType(ctx, ast->nodes[0]));
    case 2: // array
        return buildArray(ctx, ast->nodes[0]);
    default:
        panic("unknown type ast `{}`", ast->name);
    }
}

std::shared_ptr<ast::ArrayType> Builder::buildArray(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "array"_);

    auto type = buildType(ctx, ast->nodes[0]);
    auto size = ast->nodes.size() > 1 ? buildExpr(ctx, ast->nodes[1]) : nullptr;

    return std::make_shared<ast::ArrayType>(type, size);
}

std::shared_ptr<ast::Expr> Builder::buildExpr(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    switch (ast->tag) {
    case "number"_:
        return buildNumber(ctx, ast);
    case "expr"_:
        return buildBinary(ctx, ast);
    default:
        panic("unknown expr `{}`", ast->name);
    }
}

std::shared_ptr<ast::Binary> Builder::buildBinary(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "expr"_);

    auto lhs = buildExpr(ctx, ast->nodes[0]);
    auto rhs = buildExpr(ctx, ast->nodes[2]);
    //auto op = ast->nodes[1]->token_to_string();

    return std::make_shared<ast::Binary>(ast::BinaryOp::ADD, lhs, rhs);
}

std::shared_ptr<ast::IntLiteral> Builder::buildNumber(Context*, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "number"_);

    auto suffix = ast->nodes.size() > 1 ? ast->nodes[1]->token_to_string() : "";
    auto number = ast->nodes[0]->token_to_string();
    int base = 0;

    switch (ast->nodes[0]->tag) {
    case "base2"_: base = 2; break;
    case "base10"_: base = 10; break;
    case "base16"_: base = 16; break;
    default:
        panic("unknown number `{}`", ast->nodes[0]->name);
    }

    return std::make_shared<ast::IntLiteral>(number, base, suffix);
}

}