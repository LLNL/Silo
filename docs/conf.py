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

# Files to exclude
exclude_patterns = [ "func-template.md" ]

# MYST specific extensions
myst_enable_extensions = [
    "deflist",
    "dollarmath",
    "fieldlist",
    "substitution",
    "attrs_block",
    "attrs_inline",
    "colon_fence"
]
myst_heading_anchors = 3

myst_substitutions = {
  "EndFunc": "<hr class=\"docutils\" />"+"<br>"*40
}

html_static_path = ['_static']
html_theme = "sphinx_rtd_theme"
html_theme_options = {
    # Toc options
    'navigation_depth': 4
}
