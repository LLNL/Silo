# Silo Library Developer Notes

### ReadTheDocs and GHPages

We are using `.md` files to generate content on both ReadTheDocs and on GitHub pages.

To use `.md` as source files for ReadTheDocs (which uses Sphinx), we are using the [MyST](https://myst-parser.readthedocs.io/en/latest/index.html).

Both ReadTheDocs and GHPages want to generate an `index.html` from an `index.md` as the *root* of the generated content.

However, MyST supports constructs GitHub Flavored Markdown (GFM), which is used in GHPages page generation, does not.


