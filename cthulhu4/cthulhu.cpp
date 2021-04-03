#include "cthulhu.h"

#include <iostream>
#include <fstream>

#include <fmt/format.h>

namespace {
auto grammar = R"(
    unit    <- import* decl*

    # import syntax
    import  <- 'using' LIST(ident, '::') (ialias? / items?) ';' { no_ast_opt }
    items   <- '(' (LIST(include, ',') / '...') ')' { no_ast_opt }
    include <- ident ialias?
    ialias  <- 'as' ident

    # toplevel declarations
    decl        <- attribs* (alias / variant / union / record / func / var ';') { no_ast_opt }

    attribs     <- '@' attrib / '@' '[' LIST(attrib, ',') ']'
    attrib      <- qualified call?

    alias       <- 'using' ident generics? '=' type ';'
    union       <- 'union' ident generics? '{' fields? '}' { no_ast_opt }
    record      <- 'record' ident generics? '{' fields? '}' { no_ast_opt }
    variant     <- 'variant' ident generics? (':' type)? '{' LIST(case, ',')? '}' { no_ast_opt }
    func        <- 'def' ident generics? fparams? result? body { no_ast_opt }
    fparams     <- '(' params? ')'
    result      <- ':' type
    generics    <- '<' LIST(generic, ',') '>'
    generic     <- ident requires? tparam?
    requires    <- ':' LIST(qualified, '+')
    tparam      <- '=' type
    params      <- param init? (',' params)? 
    param       <- attribs* ident ':' (type / '...')
    body        <- ';' / '=' expr ';' / compound
    var         <- 'var' names init?
    names       <- qualified '(' LIST(ident, ',') ')' / name / '[' LIST(name, ',') ']'
    name        <- ident (':' type)?
    init        <- OP('=') expr

    # tagged union syntax
    case    <- ident data? ('=' expr)?
    data    <- '(' LIST(field, ',') ')'

    field       <- attribs* ident ':' type bitfield?
    bitfield    <- '[' LIST(bitrange, ',') ']'
    bitrange    <- expr ('..' expr)?
    fields      <- LIST(field, ',') { no_ast_opt }

    # type syntax
    type        <- attribs* (basictype error? / error)

    basictype   <- (pointer / array / closure / qualified / mutable)
    error       <- '!' (type / sum)?
    sum         <- '(' LIST(type, '/') ')'

    mutable     <- 'var' type
    array       <- '[' type (':' expr)? ']'
    pointer     <- '*' type { no_ast_opt }
    closure     <- '(' LIST(type, ',')? ')' '->' type
    qualified   <- LIST(ident, '::')

    # statements

    stmt        <- compound / return / break / continue / while / if / guard / expr ';' / switch / assign / for / asm / raise
    for         <- 'for' range else?
    range       <- '(' names '..' expr ')' stmt / names '..' expr compound
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
    soperand    <- expr / '[' expr ']' / '.' expr

    expr <- OP(bexpr)

    # expressions
    bexpr    <- prefix (binop prefix)* {
            precedence
                L !
                L || &&
                L & |
                L ^
                L == !=
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
            / '!=' / '!'
            / '==' 
            / '<<' !'=' 
            / '>>' !'=' / '<=' / '>=' / '<' / '>'
        >

    unop   <- < '!' / '+' / '-' / '*' / '&' >

    atom <- 'try'? (number / qualified / 'true' / 'false' / string / OP('(') expr OP(')') / lambda) postfix*

    prefix  <- atom / OP(unop) prefix / '{' LIST(arg, ',')? '}'
    postfix <- '[' expr ']' / OP('.') ident / OP('->') ident / call / ternary / 'as' type
    call    <- OP('(') LIST(arg, ',')? OP(')')
    ternary <- OP('?') expr? OP(':') expr
    arg     <-  expr / '.' ident '=' expr
    lambda  <- '[' LIST(capture, ',')? ']' fparams? lresult? lbody
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

    ident   <- < 'R' !'"' / [a-zA-Z_][a-zA-Z0-9_]* / '$' > ~spacing

    %whitespace <- spacing
    %word       <- ident

    spacing     <- (comment / space)*
    space       <- [ \t\r\n]
    comment     <- '#' (!line .)* line?
    line        <- [\r\n]+

    OP(I)       <- I ~spacing
    LIST(I, D)  <- I (D I)*
)";
}

namespace cthulhu {

using namespace peg;
using namespace peg::udl;

Handles* handles;
parser parser;

void init(Handles* h) {
    handles = h;
    parser.log = [](auto line, auto col, const auto& msg) {
        fmt::print("{}:{}: {}\n", line, col, msg);
    };

    if (!parser.load_grammar(grammar)) {
        throw std::runtime_error("failed to load grammar");
    }

    parser.enable_packrat_parsing();
    parser.enable_ast();
}

Context::Context(const std::filesystem::path& name) : name(name), source(handles->open(name)) {
    if (!parser.parse(source, tree)) {
        throw std::runtime_error("failed to parse source");
    }

    tree = parser.optimize_ast(tree);
}

void Context::compile() {
    for (auto& node : tree->nodes) {
        if (node->tag != "import"_)
            continue;

        include(node);
    }
}

void Context::include(const std::shared_ptr<peg::Ast> ast) {
    std::filesystem::path path;
    
    for (auto& part : ast->nodes) {
        if (part->tag != "ident"_)
            continue;

        path /= part->token;
    }

    std::cout << ast_to_s(ast) << std::endl;

    auto mod = open(path);

    auto last = ast->nodes.back();
    if (last->tag == "items"_) {
        if (last->tag->nodes.size() == 0) {
            // expose all symbols in this module
        }

        for (auto& item : last->nodes) {
            (void)item;
        }
    }
}

std::shared_ptr<Context> Context::open(const std::filesystem::path& path) {
    for (auto context : includes) {
        if (context->name == path) {
            return context;
        }
    }

    return std::make_shared<Context>(path);
}

}
