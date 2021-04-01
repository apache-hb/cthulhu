project('cthulhu', 'cpp',
    default_options : [
        'werror=true',
        'warning_level=3',
        'default_library=static'
    ]
)

fmt = subproject('fmt').get_variable('fmt')

inc = [ 'cthulhu' ]
deps = [ fmt ]
args = []

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

src2 = [
    'cthulhu2/stream.cpp',
    'cthulhu2/lexer.cpp',
    'cthulhu2/token.cpp',
    'cthulhu2/util.cpp',
    'cthulhu2/parse.cpp'
]

cthulhu2 = library('ctu2', src2,
    dependencies : fmt
)

ctu2 = declare_dependency(
    include_directories : 'cthulhu2',
    link_with : cthulhu2,
    dependencies : fmt
)

src3 = [ 'cthulhu3/cthulhu.cpp' ]

peg = subproject('peglib').get_variable('peglib_dep')

args = []

if meson.get_compiler('cpp').get_id() == 'msvc'
    args += [ '/Zc:__cplusplus' ]
endif

cthulhu3 = library('ctu3', src3, 
    dependencies : [ fmt, peg ],
    cpp_args : args
)

ctu3 = declare_dependency(
    include_directories : 'cthulhu3',
    link_with : cthulhu3,
    dependencies : [ fmt, peg ]
)

src4 = [ 'cthulhu4/cthulhu.cpp' ]

cthulhu4 = library('ctu4', src4,
    dependencies : [ fmt, peg ],
    cpp_args : args
)

ctu4 = declare_dependency(
    include_directories : 'cthulhu4',
    link_with : cthulhu4,
    dependencies : [ fmt, peg ]
)

subdir('tools')
subdir('test')
