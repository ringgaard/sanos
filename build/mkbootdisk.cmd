cd ..
copy bin\os.dll img\bootdisk\os > nul
copy bin\sh.exe img\bootdisk\os > nul
copy bin\setup.exe img\bootdisk\os > nul
copy bin\fdisk.exe img\bootdisk\os > nul
copy bin\boot img\bootdisk\setup > nul
copy bin\osldr.dll img\bootdisk\setup > nul
copy bin\krnl.dll img\bootdisk\setup > nul
copy bin\os.dll img\bootdisk\setup > nul
copy bin\sh.exe img\bootdisk\setup > nul
tools\mkdfs -d img\bootdisk.img -b bin\boot -l bin\osldr.dll -k bin\krnl.dll -c 1440 -i -f -S img\bootdisk\
