project('ct-platform', 'c')

host = host_machine.system()

ctstd = subproject('ct-std').get_variable('std')

src = [
    'src/error.c',
    'src/file.c',
    'src/library.c',

    'src/platform/target.c'.replace('target', host)
]

libplatform = library('platform', src,
    include_directories : 'include',
    dependencies : ctstd
)

platform = declare_dependency(
    link_with : libplatform,
    include_directories : 'include',
    dependencies : ctstd
)
