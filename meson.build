project('ctu', [ 'c', 'cpp' ],
    default_options : [
        'c_std=gnu99',
        'cpp_std=gnu++11',
        'warning_level=2',
        'werror=true'
    ]
)

lex = custom_target('lex',
    command : [ find_program('flex'), '@INPUT@' ],
    input : 'src/cthulhu.l',
    output : [ 'lex.yy.c' ]
)

parse = custom_target('parse',
    command : [ find_program('bison'), '-d', '@INPUT@' ],
    input : 'src/cthulhu.y',
    output : [ 'cthulhu.tab.c', 'cthulhu.tab.h' ],
    depends : lex
)

executable('cthulhu', parse, lex)
