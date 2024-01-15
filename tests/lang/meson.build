fs = import('fs')

foreach name, info : langs
    if fs.is_file(name / 'meson.build')
        subdir(name)
        driver_summary += { name + ' tests': true }
    endif
endforeach
