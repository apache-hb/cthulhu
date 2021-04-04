project('cpp-peglib', 'cpp')

args = []

if meson.get_compiler('cpp').get_id() == 'msvc'
    args += [ '/Zc:__cplusplus' ]
endif

dep = declare_dependency(
    include_directories : 'cpp-peglib',
    compile_args : args
)
