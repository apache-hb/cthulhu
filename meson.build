project('ctulang', 'c',
    default_options : [
        'c_std=c89',
        'werror=true',
        'warning_level=3'
    ]
)

sources = [ 'src/main.c' ]

incs = include_directories('src')

ctu = executable('ctu', sources,
    include_directories : incs
)