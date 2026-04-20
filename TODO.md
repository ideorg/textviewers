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

==== TODO

0.9
- config with font size, tab width

1.0
- keyboard using
- caret position
 
> 1.0
- other code pages than UTF8