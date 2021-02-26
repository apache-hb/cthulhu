project('cthulhu', 'cpp',
    default_options : [
        'werror=true',
        'warning_level=3',
        'cpp_std=c++17',
        'default_library=static'
    ]
)

utf8 = subproject('utf8').get_variable('utf8')

deps = [ utf8 ]
inc = [ 'cthulhu' ]
args = []

if meson.get_compiler('cpp').get_id() == 'msvc'
    args += [ '/wd4244', '/wd4127', '/wd4996' ]
endif

src = [
    'cthulhu/cthulhu.cpp',
    'cthulhu/nodes.cpp',
    'cthulhu/ir.cpp'
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

subdir('tools')
subdir('test')
