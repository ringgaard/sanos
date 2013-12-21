if not exist win\img mkdir win\img
win\tools\mkdfs -d win\img\sanos.flp -b win\install\boot\boot -l win\install\boot\osldr.dll -k win\install\boot\krnl.dll -c 1440 -i -f -S win\install -T .
