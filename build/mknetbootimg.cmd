if not exist ..\img mkdir ..\img
..\tools\mkdfs -d ..\img\sanos.0 -b ..\bin\netboot -l ..\bin\osldr.dll -k ..\bin\krnl.dll -c 512 -I 8192 -i -f -S .. -F bootnet.lst
