# misc constants and functions for code generation

# convert block strings to a list of lines to be placed
# in a block comment
# converts crlf to lf
# trims all leading whitespace on lines that start with '|'
# always breaks on newlines
# always breaks on COMMENT_LINE_LIMIT
# otherwise splits on whitespace
def chunk_comment_text(text: str, limit: int) -> list[str]:
    lines = text.splitlines()
    result = []
    for line in lines:
        if line.lstrip().startswith('|'):
            line = line.lstrip()[1:]

        while len(line) > limit:
            idx = line.rfind(' ', 0, limit)
            if idx == -1:
                idx = limit
            result.append(line[:idx])
            line = line[idx:].lstrip()

        result.append(line)

    # trim leading and trailing empty lines
    while result and not result[0].strip():
        result.pop(0)
    while result and not result[-1].strip():
        result.pop()

    return result

# best attempt convert an ident to camel case
def camel_case(text: str) -> str:
    return ''.join([word.capitalize() for word in text.split('_')])

# convert a snake case string to a space separated string
def space_name(text: str) -> str:
    return ' '.join(text.split('_'))
