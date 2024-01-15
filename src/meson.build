subdir('common')
subdir('cthulhu')

subdir('driver')

subdir('support')

subdir('interface')

if driver_tests.allowed()
    subdir('tools/harness')
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

if tool_error.allowed()
    subdir('tools/error')
endif
