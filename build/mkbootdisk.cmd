copy bin\os.dll img\bootdisk\os
copy bin\sh.exe img\bootdisk\os
copy bin\setup.exe img\bootdisk\os
copy bin\fdisk.exe img\bootdisk\os
copy bin\zxlog.exe img\bootdisk\os

copy bin\boot img\bootdisk\setup
copy bin\osldr.dll img\bootdisk\setup
copy bin\krnl.dll img\bootdisk\setup
copy bin\os.dll img\bootdisk\setup
copy bin\sh.exe img\bootdisk\setup
copy bin\zxlog.exe img\bootdisk\setup

utils\dfs\debug\mkdfs -d tools\bochs\bootdisk.img -b bin\boot -l bin\osldr.dll -k bin\krnl.dll -c 1440 -i -f -S img\bootdisk\
