#!/usr/bin/env python

import os.path
import shlex
import sys
import subprocess
from distutils.core import setup, Extension

PCRE_VERSION = 8.30

def get_pcre_info():
    cmd = 'pcre-config --version --prefix'
    args = shlex.split(cmd)
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    retcode = p.wait()
    if retcode != 0:
        # pcre-config wrote message to stderr
        exit(1)
    
    version = p.stdout.readline()
    prefix = p.stdout.readline()
    
    return (version, prefix)

version, prefix = get_pcre_info()

if float(version) < PCRE_VERSION:
    print >>sys.stderr, 'pcre is required in version >=', PCRE_VERSION
    exit(1)

pcre_include_dir = os.path.join(prefix, 'include')
pcre_library_dir = os.path.join(prefix, 'lib')

setup(name='python-pcre',
      version='0.1',
      description=('Python binding of PCRE (Perl Compatible Regular '
                   'Expressions) library that supports JIT compilation of '
                   'patterns.'),
      author='Jakub Matys',
      author_email='matys.jakub@gmail.com',
      url='https://github.com/jakm/python-pcre',
      packages=['pcre'],
      package_dir={'': 'src'},
      ext_modules=[
        Extension('_pcre',
            ['src/_pcre/pcre_match.c', 'src/_pcre/pcre_module.c',
             'src/_pcre/pcre_regex.c'],
            include_dirs=[pcre_include_dir],
            library_dirs=[pcre_library_dir],
            libraries=['pcre'],
            extra_compile_args=['-Wall', '-std=gnu99'])]
)
