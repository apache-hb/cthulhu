project('ctulang', ['c', 'cpp'], 
    version : '0.5.0',
    license : 'Apache 2.0',
    default_options : [ 
        'c_std=c99',
        'cpp_std=c++2a',
        'werror=true', 
        'warning_level=2' 
    ]
)

ctu = executable('ctu', 'src/main.c', 
    include_directories : include_directories('src')
)