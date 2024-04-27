from .util import chunk_comment_text

COMMENT_LINE_LIMIT = 80

class FileWriter:
    def __init__(self):
        self.text = ""
        self.depth = 0
        self.define_depth = 0
        self.headers = []

    def add_headers(self, headers):
        self.headers.extend(headers)

    def emit_headers(self):
        self.newline()
        for header in self.headers:
            self.writeln(f'#include "{header}"')
        self.newline()

    def newline(self):
        self.text += "\n"

    def writeln(self, text):
        self.text += f'{"    " * self.depth}{text}\n'

    def writeln_define(self, text):
        # only indent by 3 spaces for the first level of define
        # indentation. indent by 4 spaces for each level after that
        def get_indent(depth: int) -> int:
            result = 0
            if depth > 0:
                result = 3
                depth -= 1
            result += 4 * depth
            return result

        indent = get_indent(self.define_depth)
        self.text += f'#{" " * indent}{text}\n'

    def indent(self):
        self.depth += 1

    def dedent(self):
        self.depth -= 1

    def indent_define(self):
        self.define_depth += 1

    def dedent_define(self):
        self.define_depth -= 1

    def append(self, other):
        self.text += other.text

    def save(self, path):
        with open(path, 'w') as f:
            f.write(self.text)

def block_comment(f: FileWriter, text: str):
    lines = chunk_comment_text(text, COMMENT_LINE_LIMIT)
    f.writeln(f'/**')
    for line in lines:
        f.writeln(f' * {line}')
    f.writeln(f' */')

def doc_comment(f: FileWriter, text: str):
    lines = chunk_comment_text(text, COMMENT_LINE_LIMIT)
    f.writeln(f'/// {lines.pop(0)}')
    for line in lines:
        f.writeln(f'/// {line}')
