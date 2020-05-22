project('ctulang', ['c', 'cpp'], 
    version : '0.5.0',
    license : 'Apache 2.0',
    default_options : [ 
        'c_std=c11',
        'werror=true', 
        'warning_level=3' 
    ]
)

source = [
    'src/keywords.c',
    'src/stream.c',
    'src/lexer.c',
    'src/utils.c',
    'src/token.c',
    'src/parser.cpp',
    'src/main.cpp'
]

compiler = executable('ctc', source)
