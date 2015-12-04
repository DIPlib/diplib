---
title: 'Documentation with Markdown'
author: 'Ronald Ligteringen'
date: '13 November 2015'
...

As the new DIP*lib* and DIP*image* implementation will be build from scratch
this might also provide a good opportunity to decide on which format to use for
documentation. Here we suggest to implement 'Markdown' as the preferred format
for all documentation.

**Note: in this document the term 'documentation' reflects all written material
*except* code-documentation; e.g. manuals, instructions, proposals etc.**

## Considerations

- documentation must be highly accessable on all possible platforms, i.e.
Windows, Linux, MacOS, desktop, tablet or smartphone;
- it must be easily integrated in any version-control mechanism, e.g. git or
svn. This will allow for restoring previous version and annotating
modifications;
- rendering must be possible for multiple output, e.g. HTML, PDF or LaTeX;
- the format must support standard formatting like headers, bold, italics,
lists, links, footnotes etc.;
- **not ready...**

## Tools

### Pandoc
The *pandoc* application renders the md-file (Markdown-file) to the given
format as indicated by the extension of the output file. For example:

```
$ pandoc -s --toc --mathjax DIPdoc.md -H pdf_color_links.tex -o DIPdoc.pdf  # create PDF
$ pandoc -s --toc --mathjax DIPdoc.md -o DIPdoc.html # create HTML
$ pandoc -s DIPproposal_SurfSara.md -o DIPproposal_SurfSara.pdf -H pdf_colored_links.tex -N --variable mainfont="Palatino" --latex-engine=xelatex --toc --listings -H listings_setup.tex
```

**NOTE**: the ```-H pdf_colored_links``` in the PDF creation is needed for the links to have color!
The ```-H listings_setup.tex``` in the PDF creation is needed for highlighting of the code!

### MacDown (for OSX)
Having tried several Markdown editors on OSX I've experienced the best WYSIWYaG
with *MacDown*. It supports many Markdown dialects and understands *Mathjax*
entries.

## Markdown usage

### Metadata
The metadata in Markdown is used to provide the rendering information like
'title', 'author' and 'date'. This block must be formatted as a YAML block. To
avoid confusion this block must be always put on top of the document. For
example:

```
---
title: 'Documentation with Markdown'
author: 'Ronald Ligteringen'
date: '13 November 2015'
...
```
When rendered with *pandoc* this will generate the title, author and date.

### Headers
There are two formats for creating a header: Setext-style (underline) and
Atx-style (hash). Using the 'hash' form allows for easy replacement and smaller
documents (one line per header less). For Example:

```
## Tools [equivalent to <h2>Tools</h2> and \subsection{Tools}]
### Pandoc [equivalent to <h3>Pandoc</h3> and \subsubsection{Pandoc}]
```
