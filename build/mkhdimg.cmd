if not exist ..\img mkdir ..\img
..\tools\mkdfs -d ..\img\sanos.img -b ..\bin\boot -l ..\bin\osldr.dll -k ..\bin\krnl.dll -c 10240 -i -f -S .. -F bootdisk.lst
