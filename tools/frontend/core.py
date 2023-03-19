from typing import Mapping, List, TextIO
from dataclasses import dataclass
import json

@dataclass
class LexPattern:
    pattern: str
    name: str
    action: str
    tag: str

    def write_pattern(self, fd: TextIO):
        raise NotImplementedError()

    def token_name(self) -> str:
        return ' '

    def make_token_decl(self) -> str:
        return '%token'

@dataclass
class LexIgnore(LexPattern):
    def write_pattern(self, fd: TextIO):
        fd.write(f'{self.pattern} ;\n')

@dataclass
class LexTransition(LexPattern):
    state: str

    def write_pattern(self, fd: TextIO):
        fd.write(f'"{self.pattern}" {{ BEGIN({self.state}); }}\n')

@dataclass
class LexToken(LexPattern):
    field: str

    def make_action(self) -> str:
        inner = self.action + '; ' if self.action else ''
        return inner.replace('$type', f'yylval->{self.field}')
    
    def write_pattern(self, fd: TextIO):
        action = self.make_action()
        fd.write(f'{self.pattern} {{ {action}return {self.tag}; }}\n')

    def token_name(self) -> str:
        return f' "{self.name}"'

    def make_token_decl(self) -> str:
        field = f'<{self.field}>' if self.field else ''
        return f'%token{field}'

@dataclass
class LexSymbol(LexPattern):
    def write_pattern(self, fd: TextIO):
        action = self.action + '; ' if self.action else ''
        fd.write(f'"{self.pattern}" {{ {action}return {self.tag}; }}\n')

    def token_name(self) -> str:
        return f' "{self.pattern}"'

@dataclass
class LexState:
    patterns: List[LexPattern]

@dataclass
class Lexer:
    initial: str
    states: Mapping[str, LexState]
    union: Mapping[str, str]

    def write_state(self, fd: TextIO, name: str):
        if name == self.initial:
            return
    
        fd.write(f'<{name}> ')

    def declare_state(self, fd: TextIO, name: str):
        if name == self.initial:
            return

        fd.write(f'%x {name}\n')

@dataclass
class RuleSegment:
    name: str

    def get_part(self):
        return self.name

@dataclass
class RuleToken(RuleSegment):
    pass

@dataclass
class RuleSymbol(RuleSegment):
    pass

class EmptyRule(RuleSegment):
    def __init__(self):
        super().__init__('')

    def get_part(self):
        return '%empty'

@dataclass
class RuleMatch:
    action: str
    parts: List[RuleSegment]

    def make_rule(self) -> str:
        rule = ' '.join([p.get_part() for p in self.parts])
        return f'{rule} {{ {self.action} }}'

@dataclass
class GrammarRule:
    name: str
    result: str
    matches: List[RuleMatch]

    def write_type(self, fd: TextIO):
        if self.result:
            fd.write(f'%type<{self.result}> {self.name}\n')

    def write_rule(self, fd: TextIO):
        fd.write(f'{self.name} : ')
        fd.write(' | '.join([m.make_rule() for m in self.matches]))
        fd.write(';\n')

@dataclass
class Grammar:
    entry: str
    rules: List[GrammarRule]

    def write_token(self, fd: TextIO, pattern: LexPattern):
        if isinstance(pattern, (LexIgnore, LexTransition)):
            return
        
        fd.write(f'{pattern.make_token_decl()} {pattern.tag}{pattern.token_name()}\n')

def flex_preamble(name):
    return f'''\
%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="{name}"

%{{
#include "{name}-bison.h"
#include "report/report-ext.h"
#include "interop/flex.h"
%}}
'''

def bison_preamble(name):
    return f'''\
%define parse.error verbose
%define api.pure full
%lex-param {{ void *scan }}
%parse-param {{ void *scan }} {{ scan_t *x }}
%locations
%expect 0
%define api.prefix {{{name}}}

%code top {{
    #include "interop/flex.h"
}}

%{{
int {name}lex();
void {name}error(where_t *where, void *state, scan_t *scan, const char *msg);
%}}
'''

@dataclass
class Frontend:
    name: str
    lexer: Lexer
    grammar: Grammar

    def write_tmlang(self, fd: TextIO):
        result = {
	        "$schema": "https://raw.githubusercontent.com/martinring/tmlanguage/master/tmlanguage.json",
            "name": self.name,
            "patterns": [],
            "repository": {},
            "scopeName": f"source.{self.name}"
        }

        # TODO: emit tmlang properly

        fd.write(json.dumps(result, indent = 4))

    def write_flex(self, fd: TextIO):
        fd.write(flex_preamble(self.name))
        fd.write('\n')
        for name in self.lexer.states.keys():
            self.lexer.declare_state(fd, name)

        fd.write('\n%%\n\n')

        for name, state in self.lexer.states.items():
            for pattern in state.patterns:
                self.lexer.write_state(fd, name)
                pattern.write_pattern(fd)
            fd.write('\n')

        fd.write('. { report_unknown_character(scan_reports(yyextra), node_new(yyextra, *yylloc), yytext); }\n')

        fd.write('\n%%\n')
        fd.write(f'\nFLEX_MEMORY({self.name}alloc, {self.name}realloc, {self.name}free)\n')

    def write_bison(self, fd: TextIO):
        fd.write(bison_preamble(self.name))
        fd.write('\n')
        
        fd.write('%union {\n')
        for name, kind in self.lexer.union.items():
            fd.write(f'    {kind} {name};\n')
        fd.write('}\n\n')

        for name, state in self.lexer.states.items():
            for pattern in state.patterns:
                self.grammar.write_token(fd, pattern)

        for rule in self.grammar.rules:
            rule.write_type(fd)

        fd.write(f'\n%start {self.grammar.entry}\n\n')

        fd.write('%%\n\n')

        for rule in self.grammar.rules:
            rule.write_rule(fd)

        fd.write('\n%%\n')
