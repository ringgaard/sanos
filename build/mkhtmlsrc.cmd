del /Q /S htmlsrc
tools\xref -r -html -htmlroot=c:\sanos\htmlsrc\sanos -Ic:\sanos\src\include -htmlcutpath=c:\sanos -htmlnounderline -exactpositionresolve -csuffixes=c;h;cpp;asm c:\sanos\src
cd htmlsrc\sanos\src
dir /s /b | c:\sanos\utils\xreffilter\debug\xreffilter
cd ..\..\..
