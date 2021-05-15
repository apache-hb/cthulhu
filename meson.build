project('ctu', 'c',
    default_options : [
        'c_std=gnu99',
        'warning_level=2',
        'werror=true'
    ]
)

lex = custom_target('lex',
    command : [ find_program('flex'), '@INPUT@' ],
    input : 'src/cthulhu.l',
    output : [ 'flex.c', 'flex.h' ]
)

parse = custom_target('parse',
    command : [ find_program('bison'), '-d', '@INPUT@', '-Wdeprecated', '-v' ],
    input : 'src/cthulhu.y',
    output : [ 'bison.c', 'bison.h' ],
    depends : lex
)

src = [
    'src/main.c',
    'src/ast.c',
    'src/ir.c'
]

ctc = executable('cthulhu', parse, lex, src,
    include_directories : 'src'
)
