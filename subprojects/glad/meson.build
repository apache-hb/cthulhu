project('glad', 'c')

src = [ 'glad/glad.c' ]

libglad = library('libglad', src,
    include_directories : [ 'glad' ]
)

glad = declare_dependency(
    link_with : libglad,
    include_directories : '.'
)
