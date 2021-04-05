#include "cthulhu.h"

#include <fmt/core.h>

namespace cthulhu {

using namespace peg;
using namespace peg::udl;

namespace {
    // grammar consumed by cpp-peglib
    auto grammar = R"(
        unit    <- decl* !. { no_ast_opt }

        decl    <- alias / struct / union / variant
        alias   <- USING ident assign type semi
        struct  <- RECORD ident lbrace LIST(field) rbrace
        union   <- UNION ident lbrace LIST(field) rbrace
        variant <- VARIANT ident (colon type)? lbrace LIST(case) rbrace

        case    <- ident (lparen LIST(field) rparen)?
        field   <- ident colon type

        type    <- pointer / ident
        pointer <- mul type { no_ast_opt }

        # operators
        ~lbrace  <- '{'
        ~rbrace  <- '}'
        ~lparen  <- '('
        ~rparen  <- ')'
        ~assign  <- '='
        ~semi    <- ';'
        ~colon   <- ':'
        ~mul     <- '*'
        ~comma   <- ','

        # keywords
        ~USING   <- 'using'
        ~RECORD  <- 'record'
        ~UNION   <- 'union' 
        ~VARIANT <- 'variant'

        # reserved keywords
        ~COMPILE    <- 'compile'

        keyword <- USING / RECORD / UNION / VARIANT / COMPILE

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

    template<typename T = std::runtime_error, typename... A>
    [[noreturn]] void panic(const char* fmt, const A&... args) {
        throw T(fmt::format(fmt, args...));
    }
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

TypeSize SentinelType::size(Context* ctx) const {
    return ctx->get(name)->size(ctx);
}

TypeSize PointerType::size(Context* ctx) const {
    return ctx->enter(chase(ctx), false, true, [&] {
        return target::PTR;
    });
}

TypeSize BuiltinType::size(Context*) const {
    return self;
}

TypeSize AliasType::size(Context* ctx) const{
    return ctx->enter(this, false, false, [&] {
        return type->size(ctx);
    });
}

TypeSize RecordType::size(Context* ctx) const {
    return ctx->enter(this, true, false, [&] {
        TypeSize length = 0;

        for (const auto& [_, type] : fields) {
            length += type->size(ctx);
        }

        return length;
    });
}

TypeSize UnionType::size(Context* ctx) const {
    return ctx->enter(this, true, false, [&] {
        TypeSize length = 0;

        for (const auto& [_, type] : fields) {
            length = std::max(length, type->size(ctx));
        }

        return length;
    });
}

TypeSize VariantType::size(Context* ctx) const {
    return ctx->enter(this, true, false, [&] {
        TypeSize length = 0;

        for (const auto& [_, fields] : cases) {
            TypeSize sum = 0;
            for (const auto& field : fields) {
                sum += field.type->size(ctx);
            }
            length = std::max(length, sum);
        }

        return length + parent->size(ctx);
    });
}

Context::Context(std::string source) : text(source) {
    if (!parser.parse(text, tree)) {
        panic("failed to parse source");
    }

    tree = parser.optimize_ast(tree);

    std::cout << ast_to_s(tree) << std::endl;
}

void Context::resolve() {
    for (auto node : tree->nodes) {
        switch (node->tag) {
        case "struct"_:
            add(record(node));
            break;
        case "alias"_:
            add(alias(node));
            break;
        case "union"_:
            add(union_(node));
            break;
        default:
            panic("unrecognized node `{}`", node->name);
        }
    }

    for (auto* type : types) {
        if (!type->resolved()) {
            panic("unresolved type `{}`", type->name);
        }

        auto size = type->size(this);

        std::cout << "resolved: " << type->name << " size: " << size << std::endl;
    }
}

RecordType* Context::record(std::shared_ptr<Ast> ast) {
    auto name = ast->nodes[0]->token_to_string();

    TypeFields fields;
    for (auto node : ast->nodes) {
        if (node->tag != "field"_)
            continue;

        fields.push_back(field(node));
    }

    return new RecordType(name, fields);
}

UnionType* Context::union_(std::shared_ptr<peg::Ast> ast) {
    auto name = ast->nodes[0]->token_to_string();

    TypeFields fields;
    for (auto node : ast->nodes) {
        if (node->tag != "field"_)
            continue;

        fields.push_back(field(node));
    }

    return new UnionType(name, fields);
}

AliasType* Context::alias(std::shared_ptr<peg::Ast> ast) {
    auto name = ast->nodes[0]->token_to_string();
    auto other = type(ast->nodes[1]);

    return new AliasType(name, other);
}

Field Context::field(std::shared_ptr<peg::Ast> ast) {
    return { ast->nodes[0]->token_to_string(), type(ast->nodes[1]) };
}

Type* Context::type(std::shared_ptr<peg::Ast> ast) {
    switch (ast->tag) {
    case "pointer"_:
        return new PointerType(type(ast->nodes[0]));
    case "ident"_:
        return get(ast->token_to_string());
    default:
        panic("unknown type `{}`", ast->name);
    }
}



void Context::add(NamedType* other) {
    for (auto& type : types) {
        if (type->name == other->name) {
            if (type->resolved()) {
                panic("multiple definitions of `{}`", type->name);
            }
            type = other;
            return;
        }
    }

    types.push_back(other);
}

NamedType* Context::get(const TypeName& name) {
    for (auto& type : types) {
        if (type->name == name) {
            return type;
        }
    }

    auto* sentinel = new SentinelType(name);
    types.push_back(sentinel);
    return sentinel;
}


// recursion testing

void Context::push(const NamedType* type, bool allow, bool opaque) {
    for (const auto& frame : stack) {
        if (frame.type->name == type->name) {
            if (opaque && frame.nesting)
                break;

            panic("recursive type `{}` detected", type->name);
        }
    }

    stack.push_back({ type, allow });
}

void Context::pop() {
    stack.pop_back();
}

}
