src = [ 'imfilebrowser.cpp' ]
inc = include_directories('.')

libimfilebrowser = static_library('imfilebrowser', src,
    include_directories : inc,
    dependencies : imgui
)

imfilebrowser = declare_dependency(
    link_with : libimfilebrowser,
    include_directories : inc,
)
