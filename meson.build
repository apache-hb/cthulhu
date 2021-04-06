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
    'cthulhu/cthulhu.cpp',
    'cthulhu/parse.cpp'
]

cthulhu = library('ctu', src, dependencies : deps)

ctu = declare_dependency(
    include_directories : 'cthulhu', 
    link_with : cthulhu,
    dependencies : deps
)

subdir('tools')
subdir('test')
