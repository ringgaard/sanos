cd ..
if not exist img mkdir img
if not exist img\bootdisk mkdir img\bootdisk
if not exist img\bootdisk\dev mkdir img\bootdisk\dev
if not exist img\bootdisk\etc mkdir img\bootdisk\etc
if not exist img\bootdisk\os mkdir img\bootdisk\os
if not exist img\bootdisk\setup mkdir img\bootdisk\setup
if not exist img\bootdisk\target mkdir img\bootdisk\target
copy bin\os.dll img\bootdisk\os > nul
copy bin\sh.exe img\bootdisk\os > nul
copy bin\setup.exe img\bootdisk\os > nul
copy bin\fdisk.exe img\bootdisk\os > nul
copy bin\pcnet32.sys img\bootdisk\os > nul
copy build\krnl.ini img\bootdisk\etc > nul
copy build\os.ini img\bootdisk\etc > nul
copy bin\boot img\bootdisk\setup > nul
copy bin\osldr.dll img\bootdisk\setup > nul
copy bin\krnl.dll img\bootdisk\setup > nul
copy bin\os.dll img\bootdisk\setup > nul
copy bin\sh.exe img\bootdisk\setup > nul
copy bin\pcnet32.sys img\bootdisk\setup > nul
copy build\krnl.ini img\bootdisk\setup > nul
copy build\os.ini img\bootdisk\setup > nul
copy build\setup.ini img\bootdisk\setup > nul
tools\mkdfs -d img\bootdisk.img -b bin\boot -l bin\osldr.dll -k bin\krnl.dll -c 1440 -i -f -S img\bootdisk\
