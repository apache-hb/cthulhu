src = [
    'src/display.c'
]

libdisplay = library('display', src,
    include_directories : 'include',
    dependencies : [ memory, io, config, base, format ]
)

display = declare_dependency(
    link_with : libdisplay,
    include_directories : 'include',
    dependencies : [ core ]
)
