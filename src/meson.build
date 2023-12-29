subdir('common')
subdir('cthulhu')

foreach lang : get_option('languages')
    subdir('driver' / lang)
endforeach

subdir('support')

foreach interface : get_option('interfaces')
    subdir('interface' / interface)
endforeach

if frontend_gui.allowed()
    subdir('interface/gui')
endif

if driver_tests.allowed()
    subdir('tools/harness')
endif

if tool_stacktrace.allowed()
    subdir('tools/stacktrace')
endif

if tool_notify.allowed()
    subdir('tools/notify')
endif

if tool_diagnostic.allowed()
    subdir('tools/diagnostic')
endif

if tool_display.allowed()
    subdir('tools/display')
endif

if tool_cpreproc.allowed()
    subdir('tools/c_preproc')
endif
