call ..\msvc\vc

set OPTS=-nologo

if %1==PRERELEASE set OPTS=%OPTS% PRERELEASEBUILD=1
if %1==RELEASE set OPTS=%OPTS% RELEASEBUILD=1

nmake %OPTS% %2 %3 %4 %5 %6 %7 %8 %9

