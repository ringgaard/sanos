if not exist ..\img mkdir ..\img
if not exist ..\img\usr mkdir ..\img\usr
if not exist ..\img\usr\bin mkdir ..\img\usr\bin

if exist ..\img\sanos.iso del ..\img\sanos.iso

copy ..\bin\sh.exe       ..\img\usr\bin\sh.exe
copy ..\bin\httpd.dll    ..\img\usr\bin\httpd.dll
copy ..\bin\setup.exe    ..\img\usr\bin\setup.exe
copy ..\bin\fdisk.exe    ..\img\usr\bin\fdisk.exe
copy ..\bin\jinit.exe    ..\img\usr\bin\jinit.exe

copy ..\bin\msvcrt.dll   ..\img\usr\bin\msvcrt.dll
copy ..\bin\kernel32.dll ..\img\usr\bin\kernel32.dll
copy ..\bin\user32.dll   ..\img\usr\bin\user32.dll
copy ..\bin\advapi32.dll ..\img\usr\bin\advapi32.dll
copy ..\bin\winmm.dll    ..\img\usr\bin\winmm.dll
copy ..\bin\wsock32.dll  ..\img\usr\bin\wsock32.dll

..\tools\mkdfs -d ..\img\usr\BOOTIMG.BIN -b ..\bin\cdboot -l ..\bin\osldr.dll -k ..\bin\krnl.dll -c 512 -I 8192 -i -f -S .. -F bootcd.lst
..\tools\mkisofs -no-emul-boot -J -c BOOTCAT.BIN -b BOOTIMG.BIN -o ..\img\sanos.iso ..\img\usr
