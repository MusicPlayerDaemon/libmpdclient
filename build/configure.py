#!/usr/bin/env python3

import os, sys, shutil, subprocess

global_options = ['--werror']

flavors = {
    'debug': {
        'options': [
            '-Ddocumentation=true',
            '-Dtest=true',
        ],
    },

    'asan': {
        'options': [
            '-Db_sanitize=address',
            '-Dtest=true',
        ],
    },

    'release': {
        'options': [
            '--buildtype', 'release',
            '-Db_ndebug=true',
            '-Dtest=true',
        ],
    },

    'musl': {
        'options': [
            '--buildtype', 'minsize',
            '--default-library', 'static',
            '-Db_ndebug=true',
        ],
        'env': {
            'CC': 'musl-gcc',
        },
    },

    'win32': {
        'arch': 'i686-w64-mingw32',
    },

    'win64': {
        'arch': 'x86_64-w64-mingw32',
    },
}

source_root = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]) or '.', '..'))
output_path = os.path.join(source_root, 'output')
cross_path = os.path.join(source_root, 'build', 'cross')
prefix_root = '/usr/local/stow'

for name, data in flavors.items():
    print(name)
    build_root = os.path.join(output_path, name)

    env = os.environ.copy()
    if 'env' in data:
        env.update(data['env'])

    cmdline = [
        'meson', source_root, build_root,
    ] + global_options

    if 'options' in data:
        cmdline.extend(data['options'])

    prefix = os.path.join(prefix_root, 'libmpdclient-' + name)

    if 'arch' in data:
        prefix = os.path.join(prefix, data['arch'])
        cmdline += ('--cross-file', os.path.join(cross_path, name + '.txt'))

        # this is necessary because Meson uses Debian's build machine
        # MultiArch path (e.g. "lib/x86_64-linux-gnu") for cross
        # builds, which is obviously wrong
        cmdline += ('--libdir', 'lib')

    cmdline += ('--prefix', prefix)

    try:
        shutil.rmtree(build_root)
    except:
        pass

    subprocess.check_call(cmdline, env=env)
