cd ..
if not exist img mkdir img
tools\mkdfs -d %2 -b %1\bin\boot -l %1\bin\osldr.dll -k %1\bin\krnl.dll -c 1440 -i -f -S %1 -F build\bootdisk.lst -K silent
