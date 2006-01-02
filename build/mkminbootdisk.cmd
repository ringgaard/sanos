if not exist ..\img mkdir ..\img
..\tools\mkdfs -d ..\img\sanos.flp -b ..\bin\boot -l ..\bin\osldr.dll -k ..\bin\krnl.dll -c 1440 -i -f -S .. -F minbootdisk.lst
