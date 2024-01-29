events_api = configure_file(
    input : api_header,
    output : 'ctu_events_api.h',
    configuration : { 'MOD': 'EVENTS' }
)

events_include = include_directories('.', 'include')

src = [
    'src/events.c'
]

libevents = library('events', src,
    build_by_default : false,
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_EVENTS_BUILD=1' ],
    dependencies : [ memory, std, scan, notify ],
    include_directories : events_include
)

events = declare_dependency(
    link_with : libevents,
    dependencies : [ base, notify ],
    include_directories : events_include
)

install_headers(events_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
