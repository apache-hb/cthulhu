project('ctulang', 'cpp', 
    version : '0.5.0',
    license : 'Apache 2.0',
    default_options : [ 
        'cpp_std=c++17',
        'werror=true', 
        'warning_level=3' 
    ]
)

compiler = executable('ctc', 'src/main.cpp')

test('sstream1', executable('sstream1', 'src/tests/sstream.cpp',
    include_directories : include_directories('src')
))