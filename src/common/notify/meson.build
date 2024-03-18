notify_api = configure_file(
    input : api_header,
    output : 'ctu_notify_api.h',
    configuration : { 'MOD': 'NOTIFY' }
)

inc = include_directories('.', 'include')
src = [ 'src/notify.c' ]

libnotify = library('notify', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_NOTIFY_BUILD=1' ],
    dependencies : [ std, arena ],
    include_directories : inc
)

notify = declare_dependency(
    link_with : libnotify,
    dependencies : base,
    include_directories : inc
)

install_headers(notify_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
