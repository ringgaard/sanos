if not exist %1 mkdir %1
if not exist %1\bin mkdir %1\bin
if not exist %1\build mkdir %1\build
if not exist %1\tools mkdir %1\tools

copy README %1
copy COPYING %1
copy CHANGES %1

copy build\mkbootdisk.cmd %1\build
copy build\bootdisk.lst %1\build
copy build\krnl.ini %1\build\krnl.ini
copy build\os.ini %1\build\os.ini
copy build\setup.ini %1\build\setup.ini

xcopy bin %1\bin

copy tools\mkdfs.exe %1\tools
copy tools\mkfloppy.exe %1\tools
