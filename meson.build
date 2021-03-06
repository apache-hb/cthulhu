project('cthulhu', 'cpp',
    default_options : [
        'werror=true',
        'warning_level=3',
        'default_library=static'
    ]
)

utf8 = subproject('utf8').get_variable('utf8')
fmt = subproject('fmt').get_variable('fmt')

deps = [ utf8, fmt ]
inc = [ 'cthulhu' ]
args = []

if meson.get_compiler('cpp').get_id() == 'msvc'
    args += [ '/wd4244', '/wd4127' ]
endif

src = [ 
    'cthulhu/token.cpp', 
    'cthulhu/lexer.cpp', 
    'cthulhu/stream.cpp',
    'cthulhu/pool.cpp',
    'cthulhu/parser.cpp',
    'cthulhu/ast.cpp'
]

cthulhu = library('ctu', src, 
    dependencies : deps,
    cpp_args : args
)

ctu = declare_dependency(
    include_directories : inc, 
    link_with : cthulhu, 
    dependencies : deps
)

#subdir('tools')
subdir('test')
