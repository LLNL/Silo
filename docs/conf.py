import os, sys

extensions = ['sphinx.ext.mathjax']

if os.environ.get('READTHEDOCS'):
    from subprocess import call
    call(['pip', 'install', 'myst_parser'])
    extensions.append('myst_parser')
