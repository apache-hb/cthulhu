project('cthulhu', 'cpp',
    default_options : [
        'cpp_std=c++17'
    ]
)

ctu_dep = declare_dependency(
    include_directories : include_directories('.')
)

cli = executable('cli', 'cli/cli.cpp',
    dependencies : ctu_dep
)