project('ctu', 'c',
    default_options : [
        'c_std=c11', # flex under msvc requires c11
        'warning_level=2', # bison generates code that warns on gcc at level 3
        'default_library=static',
        'werror=true'
    ]
)

windows = build_machine.system() == 'windows'

flex = find_program(
    windows ? 'win_flex' : 'flex', 
    version : '>=2.6'
)

bison = find_program(
    windows ? 'win_bison' : 'bison', 
    version : '>=3.7'
)

# create our flex lexer that bison needs
lex = custom_target('lex',
    command : [ flex ] + (windows ? [ '--wincompat' ] : []) + [ 
        '--outfile=@OUTPUT0@', 
        '--header-file=@OUTPUT1@', 
        '@INPUT@'
    ],
    input : 'ctu/ast/cthulhu.l',
    output : [ 'flex.c', 'flex.h' ]
)

# create our bison parser that our ast needs
parse = custom_target('parse',
    command : [ 
        bison, '-d', '@INPUT@', '-Wcounterexamples', '-Wdeprecated', '-v',
        '--output=@OUTPUT0@',
        '--defines=@OUTPUT1@'
    ],
    input : 'ctu/ast/cthulhu.y',
    output : [ 'bison.c', 'bison.h' ],
    depends : [ lex ]
)

src = [
    'ctu/util/str.c',
    'ctu/util/report.c'
]

libutil = library('util', src)

src = [
    'ctu/ast/compile.c',
    'ctu/ast/scanner.c',
    'ctu/ast/ast.c',
    'ctu/ast/type.c'
]

libast = library('ast', src, lex, parse,
    link_with : libutil
)

src = [
    'ctu/sema/type.c'
]

libsema = library('sema', src,
    link_with : libutil
)

src = [
    'ctu/ir/ir.c',
    'ctu/ir/match.c'
]

libir = library('ir', src,
    link_with : libutil
)

src = [
    'ctu/speed/dead.c',
    'ctu/speed/mem2reg.c',
    'ctu/speed/fold.c'
]

libspeed = library('speed', src,
    link_with : libir
)

src = [
    'ctu/gen/c99.c'
]

libgen = library('gen', src,
    link_with : libir
)

src = [
    'ctu/debug/ast.c',
    'ctu/debug/ir.c',
    'ctu/debug/common.c'
]

libdebug = library('debug', src,
    link_with : libutil
)

driver = executable('ctc', 'ctu/main.c',
    c_args : [ '-D_CRT_SECURE_NO_WARNINGS=1' ],
    link_with : [ libdebug, libgen, libspeed, libir, libsema, libast ]
)
