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
    'cthulhu/sema.cpp',
    'cthulhu/parse.cpp',
    'cthulhu/emit.cpp',
    'cthulhu/visit.cpp'
]

cthulhu = library('ctu', src, dependencies : deps)

ctu = declare_dependency(
    include_directories : 'cthulhu', 
    link_with : cthulhu,
    dependencies : deps
)

cthulhu2 = library('ctu2', 'cthulhu2/parse.cpp',
    dependencies : deps
)

ctu2 = declare_dependency(
    include_directories : 'cthulhu2',
    link_with : cthulhu2,
    dependencies : deps
)

subdir('tools')
subdir('test')
