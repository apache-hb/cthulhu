#include "cthulhu.h"

namespace cthulhu {

using namespace peg;
using namespace peg::udl;

namespace target {
    IntType* CHAR = new IntType("char");
    IntType* SHORT = new IntType("short");
    IntType* INT = new IntType("int");
    IntType* LONG = new IntType("long");
    IntType* SIZE = new IntType("isize");
    IntType* UCHAR = new IntType("uchar");
    IntType* USHORT = new IntType("ushort");
    IntType* UINT = new IntType("uint");
    IntType* ULONG = new IntType("ulong");
    IntType* USIZE = new IntType("usize");
    BoolType* BOOL = new BoolType();
    VoidType* VOID = new VoidType();
    NamedType* VARIANT = INT;

    std::vector<NamedType*> BUILTINS = {
        CHAR, SHORT, INT, LONG, SIZE,
        UCHAR, USHORT, UINT, ULONG, USIZE,
        VOID, BOOL
    };
}

namespace {
    // grammar consumed by cpp-peglib
    auto grammar = R"(
        unit    <- decl* !. { no_ast_opt }

        decl        <- alias / struct / union / variant / function
        alias       <- USING ident assign type semi
        struct      <- RECORD ident lbrace LIST(field) rbrace
        union       <- UNION ident lbrace LIST(field) rbrace
        variant     <- VARIANT ident (colon type)? lbrace cases rbrace
        function    <- DEF ident colon type semi

        cases   <- LIST(case) { no_ast_opt }
        case    <- ident (lparen LIST(field) rparen)? { no_ast_opt }
        field   <- ident colon type

        type    <- pointer / closure / ident
        closure <- lparen types rparen arrow type
        pointer <- mul type { no_ast_opt }
        types   <- LIST(type) { no_ast_opt }

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
        ~arrow   <- '->' spacing

        # keywords
        ~USING   <- 'using' spacing
        ~RECORD  <- 'record' spacing
        ~UNION   <- 'union' spacing
        ~VARIANT <- 'variant' spacing
        ~DEF     <- 'def' spacing

        # reserved keywords
        ~COMPILE    <- 'compile' spacing

        keyword <- USING / RECORD / UNION / VARIANT / DEF / COMPILE

        ident   <- !keyword < [a-zA-Z_][a-zA-Z0-9_]* / '$' > spacing
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
        panic("failed to load grammar");
    }

    parser.enable_packrat_parsing();
    parser.enable_ast();
}

void Context::sema() {
    for (auto* type : types) {
        type->sema(this);
    }
}

void PointerType::sema(Context* ctx) const {
    ctx->enter(chase(ctx), false, true, [&] { });
}

void ClosureType::sema(Context* ctx) const {
    ctx->enter(this, false, true, [&] { 
        for (auto* arg : args) {
            arg->sema(ctx);
        }
        result->sema(ctx);
    });
}

void SentinelType::sema(Context* ctx) const {
    if (auto self = ctx->get(name); self != this) {
        return self->sema(ctx);
    } else {
        panic("unresolved type `{}`", name);
    }
}

void AliasType::sema(Context* ctx) const {
    return ctx->enter(this, false, false, [&] {
        return type->sema(ctx);
    });
}

void RecordType::sema(Context* ctx) const {
    ctx->enter(this, true, false, [&] {
        fields.sema(ctx);
    });
}

void UnionType::sema(Context* ctx) const {
    ctx->enter(this, true, false, [&] {
        fields.sema(ctx);
    });
}

void VariantType::sema(Context* ctx) const {
    ctx->enter(this, true, false, [&] {
        for (const auto& [_, fields] : cases) {
            fields.sema(ctx);
        }
        parent->sema(ctx);
    });
}

void FunctionType::sema(Context* ctx) const {
    signature->sema(ctx);
}

void TypeFields::sema(Context* ctx) const {
    for (const auto& [name, type] : *this) {
        std::cout << name << std::endl;
        if (type->unit()) {
            panic("field `{}` is a unit type", name);
        }
        type->sema(ctx);
    }
}

Context::Context(std::string source) : text(source) {
    if (!parser.parse(text, tree)) {
        panic("failed to parse source");
    }

    tree = parser.optimize_ast(tree);

    std::cout << ast_to_s(tree) << std::endl;
}

void Context::add(NamedType* other) {
    for (auto* type : target::BUILTINS) {
        if (type->name == other->name) {
            panic("`{}` is a builtin type and may not be redefined", type->name);
        }
    }

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
    for (auto* type : target::BUILTINS) {
        if (type->name == name) {
            return type;
        }
    }

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

void Context::push(const Type* type, bool allow, bool opaque) {
    for (const auto& frame : stack) {
        if (frame.type == type) {
            if (opaque && frame.nesting)
                break;

            panic("recursive type detected");
        }
    }

    stack.push_back({ type, allow });
}

void Context::pop() {
    stack.pop_back();
}

}
