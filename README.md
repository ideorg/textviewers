# textviewers
Text viewer especially for huge files

git clone --recursive https://github.com/ideorg/textviewers.git

How to [BUILD.md](BUILD.md)

Twin panel file managers have built in file viewers. Total Commander for Windows and
Double Commander for Linux have text viewer launched by key F3.
Unlike editors, viewed file is not loaded whole to memory, instead only viewed pieces.
Viewed files can be huge: hundreds of gigabytes of dumped databases.<br>
Text viewer differs from hex viewer in that hex viewer has constant, known positions to proceed 
goto any place of file, whereas text file has lines from one byte char LF to thousands chars in one line,
even whole multi-gigabyte file can be one line.<br>
Non-visual classes are tested with Google Test.
It uses simple Unicode library, from https://github.com/siplasplas/UTF , is needed to 
clone recursive with sub-repositories.

Features that the class will have:
- go to any place of file
- position in file is 64 bit
- file can have empty lines
- empty lines can be at start and at end, chars LF or CRLF on end of file are
treated specially: single break char not caused empty lines, except single break-line
at file end
- lines are searched forward and backward for \n chars
- very long line are forced to break, in some situation whole file (hundreds of
gigabytes) can be one line; it would enforce read too much disk and slow reading;
very long lines are wrapped
- lines can be optionally wrapped to screen width
- wrap optionally can preserve whole words
- file is default treated as is in UTF-8 format; utf8 codes are variable length, from 1 to 4 bytes
- tabulation \t is changed to spaces , but variable length spaces, from 1 to max,
tab spaces intended to equal place of screen
- beside goto with scrollbar, scrolling to small distances: one line down and up, one page down and up 

How was done length limit of lines? It is impractical to divide line in points 10000, 
20000, 30000 from begin line (if segment size was set to 10k Unicode chars).
First - utf8 char lengths are variable, second, more important, that lines are 
searching in both directions. It can't first go to begin whole line, because file 
contains only one line and would have hundreds of gigabytes.
Break point are 10000, 20000, 30000 but from beginning file, not line.
This is simplification. Two issues:for example point 20000 is needed to correct 
to 20001 or 20002 due to
- break point can't be inside utf8 code, add 1 or a few 
- break point can't be inside \r\n sequence, add 1

Second issue is that is not nice divide line near beginning, for example breakpoint 
randomly will at line[15], worse, every line has a chance to be divided, not only long line.
To avoid this, each line skip first breakpoint and divide on further.
