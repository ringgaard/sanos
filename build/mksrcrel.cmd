if not exist %1 mkdir %1
if not exist %1\build mkdir %1\build
if not exist %1\src mkdir %1\src
if not exist %1\sdk mkdir %1\sdk
if not exist %1\sdk\src mkdir %1\sdk\src
if not exist %1\sdk\src\as mkdir %1\sdk\src\as
if not exist %1\sdk\src\cc mkdir %1\sdk\src\cc
if not exist %1\sdk\src\libc mkdir %1\sdk\src\libc
if not exist %1\sdk\src\makedepend mkdir %1\sdk\src\makedepend
if not exist %1\vcwizard mkdir %1\vcwizard
if not exist %1\tools mkdir %1\tools
if not exist %1\utils mkdir %1\utils
if not exist %1\utils\dfs mkdir %1\utils\dfs
if not exist %1\utils\dbggw mkdir %1\utils\dbggw
if not exist %1\utils\mkpart mkdir %1\utils\mkpart
if not exist %1\utils\mkfloppy mkdir %1\utils\mkfloppy
if not exist %1\utils\sow mkdir %1\utils\sow

copy README %1
copy COPYING %1
copy CHANGES %1
copy Makefile %1

copy build\*.vcproj %1\build
copy build\sanos*.sln %1\build
copy build\mkbootdisk.cmd %1\build
copy build\bootdisk.lst %1\build
copy build\bootcd.lst %1\build
copy build\boothd.lst %1\build
copy build\minbootdisk.lst %1\build
copy build\krnl.ini %1\build
copy build\osrel.ini %1\build
copy build\oscd.ini %1\build
copy build\setup.ini %1\build
copy build\sanos.dep %1\build

xcopy src %1\src /s /exclude:build\exclrel.lst

copy sdk\src\as\* %1\sdk\src\as
copy sdk\src\cc\* %1\sdk\src\cc
copy sdk\src\libc\* %1\sdk\src\libc
copy sdk\src\makedepend\* %1\sdk\src\makedepend

xcopy vcwizard %1\vcwizard /s /exclude:build\exclrel.lst

copy tools\mkdfs.exe %1\tools
copy tools\dbggw.exe %1\tools
copy tools\mkfloppy.exe %1\tools
copy tools\nasm.exe %1\tools

xcopy utils\dfs %1\utils\dfs /exclude:build\exclrel.lst
xcopy utils\dbggw %1\utils\dbggw /exclude:build\exclrel.lst
xcopy utils\mkpart %1\utils\mkpart /exclude:build\exclrel.lst
xcopy utils\mkfloppy %1\utils\mkfloppy /exclude:build\exclrel.lst
xcopy utils\sow %1\utils\sow /exclude:build\exclrel.lst
