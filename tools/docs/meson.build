doxygen = find_program('doxygen', required : get_option('doxygen'))

if doxygen.found()
    cwd = meson.project_source_root()
    doxy_srcdirs = [ cwd/'interface', cwd/'common', cwd/'cthulhu', cwd/'driver' ]

    doxy_cdata = configuration_data(config_cdata)
    doxy_cdata.set('CTU_SOURCES', doxy_srcdirs.join(';'))

    doxy_cfg = configure_file(
        input : 'Doxyfile.in',
        output : 'Doxyfile',
        configuration : doxy_cdata
    )

    run_target('docs', command : [ doxygen, doxy_cfg ])
endif
