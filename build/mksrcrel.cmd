if not exist %1 mkdir %1
if not exist %1\build mkdir %1\build
if not exist %1\src mkdir %1\src
if not exist %1\tools mkdir %1\tools
if not exist %1\utils mkdir %1\utils
if not exist %1\utils\dfs mkdir %1\utils\dfs
if not exist %1\utils\dbggw mkdir %1\utils\dbggw
if not exist %1\utils\mkpart mkdir %1\utils\mkpart

copy README %1
copy COPYING %1

copy build\*.vcproj %1\build
copy build\sanos.sln %1\build
copy build\mkbootdisk.cmd %1\build
copy build\krnlrel.ini %1\build\krnl.ini
copy build\osrel.ini %1\build\os.ini
copy build\setuprel.ini %1\build\setup.ini

xcopy src %1\src /s /exclude:build\exclrel.lst

copy tools\mkdfs.exe %1\tools
copy tools\dbggw.exe %1\tools
copy tools\mkfloppy.exe %1\tools
copy tools\nasmw.exe %1\tools

xcopy utils\dfs %1\utils\dfs /exclude:build\exclrel.lst
xcopy utils\dbggw %1\utils\dbggw /exclude:build\exclrel.lst
xcopy utils\mkpart %1\utils\mkpart /exclude:build\exclrel.lst
