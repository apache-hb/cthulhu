# TODO items

## Blocking items

* complete wat support
  * how do strings work

* document more stuff
  * other people have to contribute now

* extract most of `generic` into subprojects
  * better seperation of code into more logical blocks
  * makes it easier to generate variations of the compiler
  * required for plugins

* create plugin system
  * this would allow our secret sauce to be kept closed source
  * while the majority of the framework would be open source
    * other people can fix our bugs for us
    * brownie points for being open source
    * means we can use libraries like libgccjit
    * my work will remain available to me even after the summer ends

## Optional items

* add ssa emitter
  * direct asm/wasm output would be preferrable to relying on C/wat2wasm
