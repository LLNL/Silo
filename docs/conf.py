import os, sys

extensions = []

if os.environ.get('READTHEDOCS'):
    from subprocess import call
    call(['pip', 'install', 'myst_parser'])
    extensions.append('myst_parser')
