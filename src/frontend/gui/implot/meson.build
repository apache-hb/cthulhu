src = [
    'implot_demo.cpp',
    'implot_items.cpp',
    'implot.cpp'
]

inc = include_directories('.')
args = cpp.get_supported_arguments('/wd4127')

libimplot = library('implot', src,
    cpp_args : args,
    include_directories : inc,
    dependencies : imgui
)

implot = declare_dependency(
    include_directories : inc,
    link_with : libimplot
)
