import os, sys

extensions = ['sphinx.ext.mathjax']

if os.environ.get('READTHEDOCS'):
    from subprocess import call
    call(['pip', 'install', 'myst_parser'])
    extensions.append('myst_parser')

# Attempt add_css_file only if the version of Sphinx actually supports it
def setup(app):
    if hasattr(app, 'add_css_file'):
        app.add_css_file('custom.css')
    elif hasattr(app, 'add_stylesheet'):
        app.add_stylesheet('custom.css')
