#pragma once

// project init template code

const char *BUILD_PATH = "meson.build";
const char *BUILD_TEMPLATE = 
"project('hello', 'ctu')\n"
"\n"
"src = [ 'src/entry.ct' ]\n"
"\n"
"executable('main', src)\n"
;

const char *SOURCE_DIR = "src";
const char *SOURCE_PATH = "src/entry.ct";
const char *SOURCE_TEMPLATE = 
"import sys::io;\n"
"\n"
"def main(): void {\n"
"\tio::print(\"hello world!\");\n"
"}\n"
;
