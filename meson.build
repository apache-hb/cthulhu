project('cthulhu', 'cpp', default_options : [ 'cpp_std=c++17' ])

libctu = static_library('ctu', [
    'ctu/lexer.cpp',
    'ctu/parser.cpp'
])

cli = executable('ctu', 'cli/cli.cpp', link_with : libctu)