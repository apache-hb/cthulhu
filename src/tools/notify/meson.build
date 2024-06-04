root = meson.global_source_root()
tool_cdata = configuration_data()
tool_cdata.set_quoted('TOOL_ROOT', root.replace('\\', '\\\\'))
cfg = configure_file(output : 'tool_config.h', configuration : tool_cdata)
cfg_include = include_directories('.')

executable('tool_notify', 'main.c', cfg,
    include_directories : [ cfg_include ],
    dependencies : [ notify, memory, scan, io, std, config, setup, argparse, format, backtrace ]
)
