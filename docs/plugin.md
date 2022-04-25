# Writing a plugin
a plugin is a shared library loaded by the compiler.

a plugin must export `const plugin_info_t kPluginInfo`

a plugin must be named `%s.plugin.so` or `%s.plugin.dll` where `%s` is the plugin name.

a plugin may provide a function called `void ctPluginInit(void)` that is called upon startup
