src = [
    'src/notify.c'
]

libnotify = library('notify', src,
    c_args : user_args,
    dependencies : [ std, memory ],
    include_directories : 'include'
)

notify = declare_dependency(
    link_with : libnotify,
    dependencies : [ core, base ],
    include_directories : 'include'
)
