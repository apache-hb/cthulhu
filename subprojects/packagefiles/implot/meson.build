project('implot', 'cpp')

implot_dep = declare_dependency(
    sources: [ 'implot.cpp', 'implot_items.cpp', 'implot_demo.cpp' ],
    dependencies: dependency('imgui'),
    include_directories: '.',
)

meson.override_dependency('implot', implot_dep)
