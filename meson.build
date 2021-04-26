project('cthulhu', 'cpp',
    default_options : [
        'werror=true',
        'warning_level=3',
        'default_library=static'
    ]
)

fmt = subproject('fmt').get_variable('dep')
peg = subproject('peglib').get_variable('dep')

deps = [ fmt, peg ]

src = [
    'cthulhu/parse.cpp',
    'cthulhu/sema.cpp',
    'cthulhu/tac.cpp'
]

cthulhu = library('ctu', src,
    dependencies : deps
)

ctu = declare_dependency(
    include_directories : 'cthulhu',
    link_with : cthulhu,
    dependencies : deps
)

src2 = [
    'cthulhu2/ast.cpp',
    'cthulhu2/parse.cpp'
]

cthulhu2 = library('ctu2', src2, dependencies : deps)

ctu2 = declare_dependency(
    include_directories : 'cthulhu2',
    link_with : cthulhu2,
    dependencies : deps
)

subdir('tools')
#subdir('test')
