#include <peglib.h>

#include <fstream>
#include <chrono>

using namespace peg;
using namespace std;
using namespace chrono;

auto grammar = R"(
    unit    <- import* decl*

    # import syntax
    import  <- 'using' LIST(ident, '::') items? ';'
    items   <- '(' (LIST(ident, ',') / '...') ')'

    # toplevel declarations
    decl        <- attribs* (alias / variant / union / record / func / var ';')

    attribs     <- '@' attrib / '@' '[' LIST(attrib, ',') ']'
    attrib      <- qualified call?

    alias       <- 'using' ident '=' type ';'
    union       <- 'union' ident '{' fields '}'
    record      <- 'record' ident '{' fields '}'
    variant     <- 'variant' ident (':' type)? '{' case* '}'
    func        <- 'def' ident ('(' params? ')')? result? body
    result      <- ':' type
    params      <- field init? (',' params)? 
    body        <- ';' / '=' expr ';' / compound
    var         <- 'var' names init?
    names       <- ident '(' LIST(ident, ',') ')' / name / '[' LIST(name, ',') ']'
    name        <- ident (':' type)?
    init        <- OP('=') expr

    # tagged union syntax
    case    <- 'case' ident data? ('=' expr)? ';'
    data    <- '(' LIST(field, ',') ')'

    field   <- ident ':' type
    fields  <- (field ';')*

    # type syntax
    type        <- attribs* (pointer / array / closure / qualified)

    array       <- '[' type (':' expr)? ']'
    pointer     <- '*' type
    closure     <- '(' LIST(type, ',')? ')' '->' type
    qualified   <- LIST(ident, '::')

    # statements

    stmt        <- compound / return / break / continue / while / if / guard / expr ';' / switch / assign / for / asm / with
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
    branch      <- 'case' expr ':' stmt*
    default     <- 'else' ':' stmt*
    assign      <- expr asop expr ';'
    condition   <- 'when' expr
    with        <- 'with' cond

    asop        <- < '=' / '+=' / '-=' / '/=' / '*=' / '%=' / '&=' / '|=' / '^=' / '<<=' / '>>=' >

    asm     <- 'asm' '{' opcode* '}'
    opcode  <- ident (LIST(operand, ',')? ';' / ':')
    operand <- (expr / '[' expr ']' / '$' '(' expr ')')

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
                / '&' !'='
                / '|' !'='
                / '!=' / '!'
                / '==' / '&&' / '||' 
                / '<<' !'=' 
                / '>>' !'=' / '<=' / '>=' / '<' / '>'
            >
        
    unop   <- < '!' / '+' / '-' / '*' / '&' >

    atom <- (number / qualified / 'true' / 'false' / string/ OP('(') expr OP(')')) postfix*

    prefix  <- atom / OP(unop) prefix / '{' LIST(arg, ',')? '}'
    postfix <- '[' expr ']' / OP('.') ident / OP('->') ident / call / ternary / 'as' type
    call    <- OP('(') LIST(arg, ',')? OP(')')
    ternary <- OP('?') expr? OP(':') expr
    arg     <-  expr / '.' ident '=' expr

    # basic blocks
    number  <- < (base2 / base10 / base16) ident? > ~spacing

    base10  <- < [0-9]+ >
    base2   <- < '0b' [01]+ >
    base16  <- < '0x' [0-9a-fA-F]+ >

    string  <- < ['] (!['] char)* ['] / ["] (!["] char)* ["] >
    char    <- '\\' [nrt'"\[\]\\] / !'\\' .

    ident   <- < [a-zA-Z_][a-zA-Z0-9_]* > ~spacing

    %whitespace <- spacing

    spacing     <- (comment / space)*
    space       <- [ \t\r\n]
    comment     <- '#' (!line .)* line?
    line        <- [\r\n]+

    OP(I)       <- I ~spacing
    LIST(I, D)  <- I (D I)*
)";

int main(int argc, const char** argv) {
    if (argc < 2) {
        cerr << argv[0] << " <file> " << endl;
        return 1;
    }

    ifstream in(argv[1]);
    auto text = string(istreambuf_iterator<char>{in}, {});

    parser parser;

    parser.log = [](auto line, auto col, const auto& msg) {
        cerr << line << ":" << col << ": " << msg << endl;
    };

    auto now = high_resolution_clock::now();

    if (!parser.load_grammar(grammar)) {
        cerr << "failed to load grammar" << endl;
        return 1;
    }

    auto load = high_resolution_clock::now();

    cout << "load: " << duration_cast<milliseconds>(load - now).count() << "ms" << endl;

    parser.enable_packrat_parsing();
    parser.enable_ast();

    shared_ptr<Ast> ast;
    if (!parser.parse(text, ast)) {
        cerr << "failed to parse source" << endl;
        return 1;
    }

    ast = parser.optimize_ast(ast);

    cout << "parse: " << duration_cast<milliseconds>(high_resolution_clock::now() - load).count() << "ms" << endl;

    cout << ast_to_s(ast) << endl;

    auto print = high_resolution_clock::now();
}
