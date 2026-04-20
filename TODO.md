===== DONE

0.1
- widget
- example program

0.2
- smart EOL ending file
  (smart eols: last line always hasn't eol, except
  if line has only eol - in this case but last line
  hasn't eol)
- start Google tests

0.3
- max line lens 
- Unicode
- BOM

0.4
- wrap mode

0.5
- horizontal scroll
- tab width
- selection

0.5.1 (refactor)
- replace cpg/utf submodule helpers with ICU (via misc/utf_icu.hpp adapter)
- bump cpg submodule to v1.0.0-3 (drops its nested utf submodule)
- inline ICU calls at call sites and remove misc/utf_icu.hpp adapter

0.6
- autoscroll with timer with mouse selection

0.7
- double click: select word
- triple click: select line

0.8
- search

0.9
- config with font size, tab width

1.0
- keyboard using
- caret position
- shift+keyboard selection

==== TODO

- folding: after collapse, load more lines from below the folded
  region so the viewport fills up (currently the rows inside a
  collapsed fold are just not drawn; bottom of the view becomes
  blank until the user scrolls).  Requires virtualising the view
  index (AbstractView) so hidden rows don't take screen slots.

- folding: hover-only display of expanded (v) markers in the gutter
  so densely foldable files (xml) don't look marker-heavy.  Idea
  noted earlier; make it configurable.

> 1.0
- other code pages than UTF8