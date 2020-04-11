dotted-name: ident | ident `::` dotted-name

import-decl: `import` dotted-name | `import` dotted-name `=>` ident

import-decls: import-decl | import-decl import-decls

program: import-decls body-decls | body-decls