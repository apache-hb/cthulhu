project('cthulhu', 'cpp',
    default_options : [
        'cpp_std=c++17'
    ]
)

libctu = static_library('ctu', 'ctulang/ctulang.cpp')

ctu_dep = declare_dependency(
    link_with : libctu,
    include_directories : include_directories('.')
)

cli = executable('cli', 'cli/cli.cpp',
    dependencies : ctu_dep
)