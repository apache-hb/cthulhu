fs = import('fs')

foreach lang : get_option('languages')
    if fs.is_file(lang / 'meson.build')
        subdir(lang)
        driver_summary += { lang + ' tests': true }
    endif
endforeach
