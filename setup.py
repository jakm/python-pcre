#!/usr/bin/env python

from distutils.core import setup, Extension

setup(name='python-pcre',
      version='',
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
            library_dirs=['/usr/local/lib'],
            libraries=['pcre'],
            extra_compile_args=['-g', '-Wall', '-std=gnu99'])]
)
