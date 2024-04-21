#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only

import argparse
from os import path
import re
import json

from emit.codegen import CodeGen
from emit.config import gen_config
from emit.ast import gen_ast

ap = argparse.ArgumentParser()
ap.add_argument("--source", required=True, help="Path to destination source file", type=str)
ap.add_argument("--header", required=True, help="Path to destination header file", type=str)
ap.add_argument("--inl", required=True, help="Path to destination inline file", type=str)
ap.add_argument("--sourceroot", required=True, help="Path to source root directory", type=str)

# require a single positional argument
ap.add_argument("json", help="Path to JSON definition file", type=str)

# mode of either astgen, cmdgen, yaccgen, or tmgen
modegroup = ap.add_mutually_exclusive_group(required=True)
modegroup.add_argument("--astgen", action="store_true", help="Generate AST definitions")
modegroup.add_argument("--cmdgen", action="store_true", help="Generate Command line flags")
modegroup.add_argument("--grammargen", action="store_true", help="Generate flex+bison files")
modegroup.add_argument("--tmgen", action="store_true", help="Generate vscode textmate grammar")

def generate_grammar(data, cg):
    pass

def generate_tm(data, cg):
    pass

def main():
    args = ap.parse_args()
    with open(args.json, 'r') as f:
        text = f.read()
        # trim all comments from the file before parsing
        # https://stackoverflow.com/a/46317863
        text = re.sub("//.*?\n","",text)
        text = re.sub("/\\*.*?\\*/","",text)
        data = json.loads(text)

    cg = CodeGen(data['info'], args.json, args.source, args.header, args.inl, args.sourceroot)

    if args.astgen:
        cg = gen_ast(data, cg)
    elif args.cmdgen:
        cg = gen_config(data, cg)
    elif args.yaccgen:
        generate_grammar(data, cg)
    elif args.tmgen:
        generate_tm(data, cg)

    cg.gen_public_epilogue()
    cg.gen_private_epilogue()

    cg.finish()

if __name__ == "__main__":
    main()
