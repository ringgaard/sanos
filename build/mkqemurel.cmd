if not exist %1 mkdir %1
copy img\sanos.flp %1
copy tools\qemu\qemu.exe %1
copy tools\qemu\sdl.dll %1
copy tools\qemu\fmod.dll %1
copy tools\qemu\bios.bin %1
copy tools\qemu\vgabios-cirrus.bin %1
copy tools\qemu\runsanos-rel.cmd %1\runsanos.cmd
