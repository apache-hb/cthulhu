import glob

def apply_spdx(ext):
    for filename in glob.iglob(f'**/*.{ext}', recursive=True):
        data = open(filename, 'r').read()
        if 'SPDX-License-Identifier' in data:
            continue
        with open(filename, 'w') as file:
            file.write('// SPDX-License-Identifier: GPL-3.0-only\n\n')
            file.write(data)

if __name__ == '__main__':
    apply_spdx('h')
    apply_spdx('c')
    apply_spdx('cpp')
    apply_spdx('inc')
