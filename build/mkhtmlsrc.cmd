del /Q /S htmlsrc
tools\xref c:\sanos\src -r -html -htmlroot=c:\sanos\htmlsrc\sanos -Ic:\sanos\src\include -htmlcutpath=c:\sanos -htmlnounderline -exactpositionresolve
cd htmlsrc\sanos\src
dir /s /b | c:\sanos\utils\xreffilter\debug\xreffilter
cd ..\..\..
