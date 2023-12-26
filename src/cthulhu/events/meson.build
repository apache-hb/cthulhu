src = [
    'src/events.c'
]

libevents = library('events', src,
    include_directories : 'include',
    c_args : user_args,
    dependencies : [ memory, std, scan, notify ]
)

events = declare_dependency(
    link_with : libevents,
    include_directories : 'include',
    dependencies : [ base, notify ]
)
