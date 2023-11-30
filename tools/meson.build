afl_fuzzing = get_option('afl_fuzzing').disable_auto_if(meson.is_subproject())
afl = find_program('afl-fuzz', required : afl_fuzzing.enabled())
afl_fuzzing = afl_fuzzing.require(afl.found())

if afl_fuzzing.allowed()
    subdir('afl_fuzzing')
endif
