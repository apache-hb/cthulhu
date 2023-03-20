import sys
import os
from core import *
import xml.dom.minidom as xml


def get_text(node):
    return ''.join([n.data for n in node.childNodes if n.nodeType == n.TEXT_NODE])

# token parsing


def parse_ignore(dom):
    pattern = dom.getAttribute('pattern')
    return LexIgnore(pattern, get_text(dom), None, None)


def parse_transition(dom):
    pattern = dom.getAttribute('pattern')
    return LexTransition(pattern, get_text(dom), None, None, dom.getAttribute('state'))


def parse_token(dom):
    pattern = dom.getAttribute('pattern')
    return LexToken(pattern,
                    get_text(dom),
                    dom.getAttribute('action'),
                    dom.getAttribute('tag'),
                    dom.getAttribute('type'))


def parse_symbol(dom):
    pattern = dom.getAttribute('pattern')
    return LexSymbol(pattern, get_text(dom), dom.getAttribute('action'), dom.getAttribute('tag'))


def parse_pattern(dom):
    kind = dom.tagName
    match kind:
        case 'ignore':
            return parse_ignore(dom)
        case 'transition':
            return parse_transition(dom)
        case 'token':
            return parse_token(dom)
        case 'symbol':
            return parse_symbol(dom)


def parse_state(dom):
    patterns = [parse_pattern(
        node) for node in dom.childNodes if node.nodeType == node.ELEMENT_NODE]

    return LexState(patterns)


def parse_fields(dom):
    return {get_text(node): node.getAttribute('type') for node in dom.getElementsByTagName('field')}


def parse_tokens(dom):
    initial = dom.getAttribute('initial')
    states = {node.getAttribute('name'): parse_state(node)
              for node in dom.getElementsByTagName('state')}
    fields = parse_fields(dom)

    return Lexer(initial, states, fields)

# grammar parsing


def parse_item(node):
    kind = node.tagName
    match kind:
        case 'token':
            return RuleToken(get_text(node))
        case 'rule':
            return RuleSymbol(get_text(node))
        case 'empty':
            return EmptyRule()


def parse_match(dom):
    action = dom.getAttribute('action')
    pattern = [parse_item(node) for node in dom.childNodes if node.nodeType == node.ELEMENT_NODE]
    return RuleMatch(action, pattern)

def parse_rule(dom):
    name = dom.getAttribute('name')
    result = dom.getAttribute('type')
    matches = [parse_match(node) for node in dom.getElementsByTagName('match')]

    print(name)

    return GrammarRule(name, result, matches)


def parse_grammar(dom):
    # getElementsByTagName turns out to be pretty broken in this case
    entry = dom.getAttribute('entry')
    rules = [parse_rule(node) for node in dom.childNodes if node.nodeType == node.ELEMENT_NODE and node.tagName == 'rule']
    return Grammar(entry, rules)

def main():
    path = sys.argv[1]
    name = sys.argv[2]

    token_dom = xml.parse(f'{path}/tokens.xml')
    tokens = parse_tokens(token_dom.getElementsByTagName('tokens')[0])

    grammar_dom = xml.parse(f'{path}/grammar.xml')
    grammar = parse_grammar(grammar_dom.getElementsByTagName('grammar')[0])

    it = Frontend('pl0', tokens, grammar)

    try:
        os.mkdir(f'{name}-out')
    except:
        pass

    with open(f'{name}-out/{name}.l', 'w') as fd:
        it.write_flex(fd)

    with open(f'{name}-out/{name}.y', 'w') as fd:
        it.write_bison(fd)


if __name__ == "__main__":
    main()
