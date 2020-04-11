project('ctu', ['cpp'],
    default_options : [
        'cpp_std=c++17'
    ]
)

sources = []
incs = []
args = []

subdir('src')

ctu = executable('ctu', sources,
    include_directories : incs,
    cpp_args : args
)