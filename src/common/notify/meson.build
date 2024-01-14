src = [
    'src/notify.c'
]

libnotify = library('notify', src,
    build_by_default : false,
    c_args : user_args,
    dependencies : [ std, arena ],
    include_directories : 'include'
)

notify = declare_dependency(
    link_with : libnotify,
    dependencies : [ core, base ],
    include_directories : 'include'
)
