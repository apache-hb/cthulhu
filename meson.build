project('cthulhu', 'c',
    default_options : [
        'c_std=c11'
    ]
)

sources = [
    'common/lexer.c',
    'common/parser.c'
]

common = static_library('ctu', sources)

cli = executable('ctu', 'cli/main.c', 
    link_with : common
)