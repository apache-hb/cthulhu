src = [
    'implot_demo.cpp',
    'implot_items.cpp',
    'implot.cpp'
]

inc = include_directories('.')

libimplot = library('implot', src,
    include_directories : inc,
    dependencies : imgui
)

implot = declare_dependency(
    include_directories : inc,
    link_with : libimplot
)
