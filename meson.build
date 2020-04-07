project('ctu', ['cpp'],
    default_options : [
        'cpp_str=gnu++17'
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