cd ..
if not exist img mkdir img
if not exist %2 mkdir %2
if not exist %2\dev mkdir %2\dev
if not exist %2\proc mkdir %2\proc
if not exist %2\etc mkdir %2\etc
if not exist %2\bin mkdir %2\bin
if not exist %2\setup mkdir %2\setup
if not exist %2\mnt mkdir %2\mnt
if not exist %2\usr mkdir %2\usr
copy %1\os.dll %2\bin > nul
rem copy %1\ndis.sys %2\bin > nul
copy %1\sh.exe %2\bin > nul
copy %1\setup.exe %2\bin > nul
copy %1\fdisk.exe %2\bin > nul
copy %1\pcnet32.sys %2\bin > nul
copy %1\ne2000.sys %2\bin > nul
copy %1\3c905c.sys %2\bin > nul
copy %1\rtl8139.sys %2\bin > nul
copy %1\eepro100.sys %2\bin > nul
copy %1\msvcrt.dll %2\bin > nul
copy %1\kernel32.dll %2\bin > nul
copy %1\user32.dll %2\bin > nul
copy %1\advapi32.dll %2\bin > nul
copy %1\winmm.dll %2\bin > nul
copy %1\wsock32.dll %2\bin > nul
copy %1\jinit.exe %2\bin > nul
copy build\krnl.ini %2\etc > nul
copy build\os.ini %2\etc > nul
copy %1\boot %2\setup > nul
copy %1\osldr.dll %2\setup > nul
copy build\krnl.ini %2\setup > nul
copy build\os.ini %2\setup > nul
copy build\setup.ini %2\setup > nul
tools\mkdfs -d %3 -b %1\boot -l %1\osldr.dll -k %1\krnl.dll -c 1440 -i -f -S %2\
