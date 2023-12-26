src = [
    'src/common.c',
    'src/display.c',
]

libdisplay = library('display', src,
    include_directories : 'include',
    dependencies : [ memory, io, config, base ]
)

display = declare_dependency(
    link_with : libdisplay,
    include_directories : 'include',
    dependencies : [ core ]
)
