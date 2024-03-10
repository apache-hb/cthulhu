src = [ 'src/draw.cpp' ]
inc = include_directories('include')
deps = [ imgui, implot ]

libdraw = library('draw', src,
    cpp_args : gui_args,
    build_by_default : false,
    install : not meson.is_subproject(),
    include_directories : inc,
    dependencies : deps
)

draw = declare_dependency(
    include_directories : inc,
    link_with : libdraw,
    dependencies : deps
)
