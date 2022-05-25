project('ct-base', 'c')

src = [
    'src/base.c'
]

args = []

if get_option('tuning').enabled()
    args += [ '-DENABLE_TUNING=1' ]
endif

libbase = library('base', src,
    include_directories : 'include',
    c_args : args
)

base = declare_dependency(
    link_with : libbase,
    include_directories : 'include',
    compile_args : args
)
