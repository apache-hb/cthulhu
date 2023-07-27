src = [
    'src/main.c'
]

obr = library('obr', src,
    dependencies : [ mediator, interop, location, hlir ],
    c_args : args,
    include_directories : 'src'
)

langs += {
    'obr': {
        'dep': declare_dependency(link_with : obr),
        'mod': 'kOberonModule'
    }
}
