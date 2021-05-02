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
    command : [ find_program('bison'), '-d', '@INPUT@' ],
    input : 'src/cthulhu.y',
    output : [ 'bison.c', 'bison.h' ],
    depends : lex
)

executable('cthulhu', parse, lex, 'src/ast.c', 'src/main.c',
    include_directories : 'src'
)
