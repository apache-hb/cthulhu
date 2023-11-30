foreach name, lang_config : langs
    cfg_name = name + '_mod.h'
    afl_header = configure_file(
        input : 'data/driver_mod.h.in',
        output : cfg_name,
        configuration : {
            'DRIVER': lang_config['mod'],
            'AFL_DRIVER': lang_config['mod']
        }
    )

    lang_dep = lang_config['dep']
    executable('fuzzer.' + name, 'main.c', afl_header,
        c_args : [ '-DDRIVER_MOD_HEADER=' + cfg_name ],
        dependencies : [ lang_dep, base, std, emit, mediator ]
    )
endforeach
