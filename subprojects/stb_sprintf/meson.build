project('stb_sprintf', 'c')

src = [ 'stb_sprintf.c' ]
inc = include_directories('.')

libstb_sprintf = static_library('stb_sprintf', src,
    include_directories : inc
)

stb_sprintf = declare_dependency(
    include_directories : inc,
    link_with : libstb_sprintf
)
