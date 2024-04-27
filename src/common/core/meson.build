version = meson.project_version()
parts = version.split('.')

config_cdata = configuration_data()

config_cdata.set('CTU_VERSION_MAJOR', parts[0].to_int())
config_cdata.set('CTU_VERSION_MINOR', parts[1].to_int())
config_cdata.set('CTU_VERSION_PATCH', parts[2].to_int())

config_cdata.set10('CTU_DEBUG', is_debug)
config_cdata.set10('CTU_ASSERTS', opt_asserts.allowed())
config_cdata.set10('CTU_PARANOID', opt_paranoid.allowed())

if default_library == 'shared'
    config_cdata.set('CTU_BUILDTYPE', 'CT_BUILDTYPE_SHARED')
else
    config_cdata.set('CTU_BUILDTYPE', 'CT_BUILDTYPE_STATIC')
endif

config_cdata.set10('CTU_TRACE_MEMORY', trace_memory.allowed())
config_cdata.set10('CTU_STB_SPRINTF', opt_stb_sprintf.allowed())

# config_cdata.set10('CTU_MONOLITHIC', monolithic)

ctu_config_header = configure_file(
    output : 'ctu_core_config.h',
    configuration : config_cdata
)

inc = include_directories('.')

core = declare_dependency(
    include_directories : [ 'include', inc ]
)

# use files() as this file is referenced in other places and we need to preserve the path properly
api_header = files('data/module_api.h.in')

install_headers(ctu_config_header)
install_subdir('include', install_dir : 'include', strip_directory : true)
