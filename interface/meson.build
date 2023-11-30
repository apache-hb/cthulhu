subdir('support')

setargv = []
if cc.get_id() == 'msvc'
    setargv = [ 'setargv.obj' ]
endif

foreach interface : get_option('interfaces')
    subdir(interface)
endforeach
