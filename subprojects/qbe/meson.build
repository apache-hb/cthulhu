project('qbe', 'c',
    default_options : [
        'c_std=c99',
        'warning_level=3'
    ]
)

src = [
    'qbe/main.c', 'qbe/util.c', 
    'qbe/parse.c', 'qbe/cfg.c', 
    'qbe/mem.c', 'qbe/alias.c',
    'qbe/load.c', 'qbe/copy.c', 
    'qbe/fold.c', 'qbe/live.c', 
    'qbe/spill.c', 'qbe/rega.c',
    'qbe/gas.c'
]

amd64 = [
    'qbe/amd64/targ.c', 'qbe/amd64/sysv.c', 
    'qbe/amd64/isel.c', 'qbe/amd64/emit.c'
]

config = configuration_data({
    'Defasm': 'Gaself',
    'Deftgt': 'T_amd64_sysv'
})

configure_file(output : 'config.h',
    configuration : config
)

args = [ '-Dasm=assembly' ]

libqbe = library('qbe', src,
    c_args : args,
    include_directories : 'qbe'
)

libamd64 = library('qbe-amd64', amd64,
    include_directories : 'qbe',
    c_args : args,
    link_with : libqbe
)

qbe = declare_dependency(
    link_with : libqbe,
    include_directories : '.'
)

amd64 = declare_dependency(
    dependencies : qbe,
    link_with : libamd64,
    include_directories : '.'
)
