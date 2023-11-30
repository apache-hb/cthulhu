gen_corpus_script = find_program('scripts/gen_corpus.py')

foreach name, lang_config : langs
    if not lang_config.has_key('fuzz_corpus')
        continue
    endif

    cfg_name = name + '_mod.h'
    corpus_dir = lang_config['fuzz_corpus']
    lang_dep = lang_config['dep']
    lang_mod = lang_config['mod']

    llvm_header = configure_file(
        input : 'data/driver_mod.h.in',
        output : cfg_name,
        configuration : {
            'DRIVER': lang_mod,
            'AFL_DRIVER': lang_mod
        }
    )

    fuzz_compile_args = [
        '-DDRIVER_MOD_HEADER="' + cfg_name + '"',
        '-fsanitize=fuzzer'
    ]

    fuzz_exe = executable('fuzzer.' + name, 'main.cpp', llvm_header,
        cpp_args : fuzz_compile_args,
        link_args : [ '-fsanitize=fuzzer' ],
        dependencies : [ lang_dep, base, std, emit, mediator, argparse ]
    )

    fuzz_run_args = [
        # we leak *ALOT* of memory, so disable leak detection
        # enable this later once arenas are implemented
        '-detect_leaks=0',

        '-print_final_stats=1',

        # if we take more than 3 seconds to run, we're probably stuck
        '-timeout=3'
    ]

    gen_corpus_name = name + '_corpus'
    gen_corpus_dir = meson.current_build_dir() / gen_corpus_name

    llvm_gen_corpus = custom_target('llvm_fuzzer.gen_corpus.' + name,
        command : [ gen_corpus_script, fuzz_exe.full_path(), gen_corpus_dir, corpus_dir, 'stub_' + name + '.txt' ],
        output : [ 'stub_' + name + '.txt' ]
    )

    run_target('llvm_fuzzer.run.' + name,
        command : [ fuzz_exe, gen_corpus_dir ] + fuzz_run_args,
        depends : [ llvm_gen_corpus ]
    )
endforeach
