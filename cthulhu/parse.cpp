#include "cthulhu.h"

namespace cthulhu {

using namespace peg;
using namespace peg::udl;

namespace {
    // grammar consumed by cpp-peglib
    auto grammar = R"(
        unit    <- decl+ eof { no_ast_opt }

        # toplevel decls
        decl    <- record / alias / variant / function / variable 

        record  <- RECORD ident lbrace fields rbrace { no_ast_opt }
        alias   <- USING ident assign type semi { no_ast_opt }
        variant <- VARIANT ident (colon type)? lbrace cases rbrace { no_ast_opt }

        cases   <- LIST(case) { no_ast_opt }
        case    <- ident (lparen fields rparen)? { no_ast_opt }

        fields  <- LIST(field) { no_ast_opt }
        field   <- ident colon type { no_ast_opt }

        variable    <- VAR ident colon type assign expr semi { no_ast_opt }

        function    <- DEF ident params? (colon type)? body { no_ast_opt }

        params      <- lparen LIST(param)? rparen { no_ast_opt }
        param       <- ident colon type (assign expr)? { no_ast_opt }
        body        <- semi / compound / assign expr semi { no_ast_opt }

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
        type    <- ident / pointer / array / closure { no_ast_opt }
        pointer <- mul type
        array   <- lsquare type (colon expr)? rsquare { no_ast_opt }
        closure <- lparen types? rparen arrow type { no_ast_opt }
        types   <- LIST(type) { no_ast_opt }

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
        ~arrow  <- '->'

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
        ~VARIANT    <- < 'variant' skip >
        ~DEF        <- < 'def' skip >
        ~RETURN     <- < 'return' skip >
        ~VAR        <- < 'var' skip >

        # reserved keywords
        ~COMPILE <- 'compile' skip
        ~REQUIRES   <- 'requires' skip

        KEYWORD <- USING / RECORD / DEF / RETURN / VARIANT / VAR / COMPILE / REQUIRES

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
        case "alias"_:
            buildAlias(ctx, node);
            break;
        case "variant"_:
            buildVariant(ctx, node);
            break;
        case "function"_:
            buildFunction(ctx, node);
            break;
        default:
            panic("TODO: unknown tag `{}`", node->name);
        }
    }

    for (auto type : ctx->types) {
        type->sema(ctx);
    }
}

void Builder::buildRecord(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "record"_);

    auto name = ast->nodes[0]->token_to_string();
    auto fields = buildFields(ctx, ast->nodes[1]);

    ctx->add(new ast::RecordType(name, fields));
}

void Builder::buildAlias(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "alias"_);

    auto name = ast->nodes[0]->token_to_string();
    auto type = buildType(ctx, ast->nodes[1]);

    ctx->add(new ast::AliasType(name, type));
}

void Builder::buildVariant(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "variant"_);

    auto name = ast->nodes[0]->token_to_string();
    auto parent = ast->nodes.size() > 2 ? buildType(ctx, ast->nodes[1]) : ctx->get("int");
    auto cases = buildCases(ctx, ast->nodes.back());

    ctx->add(new ast::SumType(name, parent, cases));
}

void Builder::buildFunction(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "function"_);

    auto name = ast->nodes[0]->token_to_string();
    //auto params = ast->nodes[1]->tag == "params"_ ? buildParams(ctx, ast->nodes[1]) : {};
    auto result = ast->nodes[ast->nodes.size() - 2]->tag == "type"_ 
        ? buildType(ctx, ast->nodes[ast->nodes.size() - 2]) 
        : ctx->get("void");
    auto last = ast->nodes.back();
    //auto body = buildBody(ctx, ast->nodes.back());

    std::cout << ast_to_s(ast) << std::endl;

    switch (last->choice) {
    case 0: // empty
    case 1: // complex
    case 2: // simple
        ctx->add(new ast::Function(name, {}, result));
        break;
    default:
        panic("unknown function body `{}`", last->name);
    }
}

ast::Cases Builder::buildCases(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "cases"_);

    ast::Cases cases;

    for (auto node : ast->nodes) {
        auto name = node->nodes[0]->token_to_string();

        ast::Fields fields;
        if (node->nodes.size() > 1) {
            fields = buildFields(ctx, node->nodes[1]);
        }

        cases.add(name, fields);
    }

    return cases;
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

ast::Type* Builder::buildType(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "type"_);

    switch (ast->choice) {
    case 0: // ident
        return ctx->get(ast->nodes[0]->token_to_string());
    case 1: // pointer
        return new ast::PointerType(buildType(ctx, ast->nodes[0]));
    case 2: // array
        return buildArray(ctx, ast->nodes[0]);
    case 3: // closure
        return buildClosure(ctx, ast->nodes[0]);
    default:
        std::cout << ast_to_s(ast) << std::endl;
        panic("unknown type ast `{}`", ast->name);
    }
}

ast::ArrayType* Builder::buildArray(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "array"_);

    auto type = buildType(ctx, ast->nodes[0]);
    auto size = ast->nodes.size() > 1 ? buildExpr(ctx, ast->nodes[1]) : nullptr;

    return new ast::ArrayType(type, size);
}

ast::ClosureType* Builder::buildClosure(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ast::Types types;
    auto result = buildType(ctx, ast->nodes.back());

    if (ast->nodes.size() > 1) {
        for (auto node : ast->nodes[0]->nodes) {
            types.push_back(buildType(ctx, node));
        }
    }

    return new ast::ClosureType(types, result);
}

ast::Expr* Builder::buildExpr(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    switch (ast->tag) {
    case "number"_:
        return buildNumber(ctx, ast);
    case "expr"_:
        return buildBinary(ctx, ast);
    default:
        panic("unknown expr `{}`", ast->name);
    }
}

ast::Binary* Builder::buildBinary(Context* ctx, std::shared_ptr<peg::Ast> ast) {
    ASSERT(ast->tag == "expr"_);

    auto lhs = buildExpr(ctx, ast->nodes[0]);
    auto rhs = buildExpr(ctx, ast->nodes[2]);
    //auto op = ast->nodes[1]->token_to_string();

    return new ast::Binary(ast::BinaryOp::ADD, lhs, rhs);
}

ast::IntLiteral* Builder::buildNumber(Context*, std::shared_ptr<peg::Ast> ast) {
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

    return new ast::IntLiteral(number, base, suffix);
}

}