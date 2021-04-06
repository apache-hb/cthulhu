#include "cthulhu.h"

#include <fmt/core.h>

namespace cthulhu {

using namespace peg;
using namespace peg::udl;

namespace target {
    IntType* CHAR = new IntType("char", 1, true);
    IntType* SHORT = new IntType("short", 2, true);
    IntType* INT = new IntType("int", 4, true);
    IntType* LONG = new IntType("long", 8, true);
    IntType* UCHAR = new IntType("uchar", 1, false);
    IntType* USHORT = new IntType("ushort", 2, false);
    IntType* UINT = new IntType("uint", 4, false);
    IntType* ULONG = new IntType("ulong", 8, false);
    BoolType* BOOL = new BoolType();
    VoidType* VOID = new VoidType();
    NamedType* VARIANT = INT;
}

namespace {
    // grammar consumed by cpp-peglib
    auto grammar = R"(
        unit    <- decl* !. { no_ast_opt }

        decl    <- alias / struct / union / variant
        alias   <- USING ident assign type semi
        struct  <- RECORD ident lbrace LIST(field) rbrace
        union   <- UNION ident lbrace LIST(field) rbrace
        variant <- VARIANT ident (colon type)? lbrace cases rbrace

        cases   <- LIST(case) { no_ast_opt }
        case    <- ident (lparen LIST(field) rparen)? { no_ast_opt }
        field   <- ident colon type

        type    <- pointer / ident
        pointer <- mul type { no_ast_opt }

        # operators
        ~lbrace  <- '{' spacing
        ~rbrace  <- '}' spacing
        ~lparen  <- '(' spacing
        ~rparen  <- ')' spacing
        ~assign  <- '=' spacing
        ~semi    <- ';' spacing
        ~colon   <- ':' spacing
        ~mul     <- '*' spacing
        ~comma   <- ',' spacing

        # keywords
        ~USING   <- 'using' spacing
        ~RECORD  <- 'record' spacing
        ~UNION   <- 'union' spacing
        ~VARIANT <- 'variant' spacing

        # reserved keywords
        ~COMPILE    <- 'compile' spacing

        keyword <- USING / RECORD / UNION / VARIANT / COMPILE

        ident   <- !keyword < [a-zA-Z_][a-zA-Z0-9_]* / '$' > spacing
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

void TypeFields::add(const Field& field) {
    for (auto& elem : *this) {
        if (elem.name == field.name) {
            panic("field `{}` already defined", field.name);
        }
    }

    push_back(field);
}


void VariantCases::add(const VariantCase& field) {
    for (auto& elem : *this) {
        if (elem.name == field.name) {
            panic("variant case `{}` already defined", field.name);
        }
    }

    push_back(field);
}

TypeSize SentinelType::size(Context* ctx) const {
    return ctx->get(name)->size(ctx);
}

TypeSize PointerType::size(Context* ctx) const {
    return ctx->enter(chase(ctx), false, true, [&] {
        return target::PTR;
    });
}

TypeSize IntType::size(Context*) const {
    return self;
}

TypeSize VoidType::size(Context*) const {
    panic("size of type is dependant on void");
}

TypeSize BoolType::size(Context*) const {
    return target::B;
}

TypeSize AliasType::size(Context* ctx) const {
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
        case "variant"_:
            add(variant(node));
            break;
        default:
            panic("unrecognized node `{}`", node->name);
        }
    }

    for (auto* type : types) {
        if (!type->resolved()) {
            panic("unresolved type `{}`", type->name);
        }

        // TODO: segregate builtin types so we dont
        // redundantly check their sizes
        if (type != target::VOID)
            type->size(this);
    }
}

RecordType* Context::record(std::shared_ptr<Ast> ast) {
    auto name = ast->nodes[0]->token_to_string();

    TypeFields fields;
    for (auto node : ast->nodes) {
        if (node->tag != "field"_)
            continue;

        fields.add(field(node));
    }

    return new RecordType(name, fields);
}

UnionType* Context::union_(std::shared_ptr<peg::Ast> ast) {
    auto name = ast->nodes[0]->token_to_string();

    TypeFields fields;
    for (auto node : ast->nodes) {
        if (node->tag != "field"_)
            continue;

        fields.add(field(node));
    }

    return new UnionType(name, fields);
}

AliasType* Context::alias(std::shared_ptr<peg::Ast> ast) {
    auto name = ast->nodes[0]->token_to_string();
    auto other = type(ast->nodes[1]);

    return new AliasType(name, other);
}

VariantType* Context::variant(std::shared_ptr<peg::Ast> ast) {
    auto name = ast->nodes[0]->token_to_string();

    auto parent = ast->nodes.size() > 2
        ? type(ast->nodes[1])
        : target::VARIANT;
    VariantCases cases;

    auto nodes = ast->nodes.back()->nodes;
    for (auto node : nodes) {
        cases.add(vcase(node));
    }

    return new VariantType(name, parent, cases);
}

VariantCase Context::vcase(std::shared_ptr<peg::Ast> ast) {
    auto name = ast->nodes[0]->token_to_string();
    TypeFields fields;

    for (auto node : ast->nodes) {
        if (node->tag != "field"_)
            continue;

        fields.add(field(node));
    }

    return { name, fields };
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

    std::cout << "push: " << type->name << std::endl;

    stack.push_back({ type, allow });
}

void Context::pop() {
    stack.pop_back();
}

}
