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
        variant     <- VARIANT ident tail? lbrace cases rbrace
        function    <- DEF ident params? tail? body { no_ast_opt }
        body        <- (assign expr)? semi / compound

        params      <- lparen LIST(param)? rparen { no_ast_opt }
        param       <- ident tail

        cases   <- LIST(case) { no_ast_opt }
        case    <- ident (lparen LIST(field) rparen)? { no_ast_opt }
        field   <- ident tail

        tail    <- colon type

        type    <- pointer / closure / ident
        closure <- lparen types rparen arrow type
        pointer <- mul type { no_ast_opt }
        types   <- LIST(type) { no_ast_opt }

        # statements
        stmt        <- compound / expr semi
        compound    <- lbrace LIST(stmt)? rbrace

        # expressions
        expr    <- atom (binop atom)* {
            precedence
                L or and
                L bitor bitand
                L xor
                L eq neq
                L gt gte lt lte
                L shl shr
                L add sub
                L div mod mul
        }

        binop   <- < 
            add / sub / mod / mul / div / 
            shl / shr / 
            gt / gte / lt / lte / 
            eq / neq / xor / bitor / bitand / 
            or / and 
        >

        atom    <- ident

        # operators
        ~lbrace  <- '{' spacing
        ~rbrace  <- '}' spacing
        ~lparen  <- '(' spacing
        ~rparen  <- ')' spacing
        ~assign  <- '=' !'=' spacing
        ~semi    <- ';' spacing
        ~colon   <- ':' spacing
        ~comma   <- ',' spacing
        ~arrow   <- '->' spacing

        ~add    <- '+' spacing
        ~sub    <- '-' spacing
        ~mod    <- '%' spacing
        ~mul    <- '*' spacing
        ~div    <- '/' spacing
        ~shl    <- '<<' spacing
        ~shr    <- '>>' spacing
        ~gt     <- '<' !'=' spacing
        ~gte    <- '<=' spacing
        ~lt     <- '>' !'=' spacing
        ~lte    <- '>=' spacing
        ~eq     <- '==' spacing
        ~neq    <- '!=' spacing
        ~bitor  <- '|' !'|' spacing
        ~bitand <- '&' !'&' spacing
        ~or     <- '||' spacing
        ~and    <- '&&' spacing
        ~xor    <- '^' spacing

        # keywords
        ~USING   <- 'using' spacing
        ~RECORD  <- 'record' spacing
        ~UNION   <- 'union' spacing
        ~VARIANT <- 'variant' spacing
        ~DEF     <- 'def' spacing

        # reserved keywords
        ~COMPILE    <- 'compile' spacing

        keyword <- USING / RECORD / UNION / VARIANT / DEF / COMPILE

        ident   <- !keyword < [a-zA-Z_][a-zA-Z0-9_]* / '$' > spacing*
        ~spacing <- (space / comment)
        space   <- [ \t\r\n]
        comment <- '#' (!newline .)* newline?
        newline <- [\r\n]+

        LIST(R) <- R (comma R)*
    )";

/*
unit    <- decl* eof


# toplevel decls
decl    <- struct / variant / alias 

struct  <- RECORD ident lbrace LIST(field) rbrace
variant <- VARIANT ident lbrace LIST(case) rbrace
alias   <- USING ident assign type

case    <- ident fields?
fields  <- lparen LIST(field) rparen
field   <- ident colon type

# types
type    <- ident / pointer
pointer <- mul type

# symbols
~lbrace <- '{' space
~rbrace <- '}' space
~lparen <- '(' space
~rparen <- ')' space
~colon  <- ':' space
~comma  <- ',' space
~assign <- '=' ![=] space

# operators
~mul    <- '*' space

# keywords
~USING      <- 'using' skip
~RECORD     <- 'record' skip
~VARIANT    <- 'variant' skip

# reserved keywords
~COMPILE <- 'compile' skip

KEYWORD <- USING / RECORD / VARIANT / COMPILE

# an identifier is any sequence of [a-zA-Z_][a-zA-Z0-9_] or a single $
# that is *not* a keyword
ident   <- !KEYWORD < [a-zA-Z_][a-zA-Z0-9_]* / '$' > skip

# macros
LIST(I) <- I (comma I)*

# whitespace handling
~comment <- '#' (!line .)* line?
~line    <- [\r\n]
~blank   <- [ \t]
~space   <- (blank / line / comment)*
~skip    <- ![a-zA-Z_] space
~eof     <- !.
*/

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
    ctx->enter(this, false, true, [&] {
        args.sema(ctx);
        result->sema(ctx);
    });
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

            panic("recursive `{}` type detected", type->str());
        }
    }

    stack.push_back({ type, allow });
}

void Context::pop() {
    stack.pop_back();
}

}
