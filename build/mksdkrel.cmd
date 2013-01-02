if not exist %1 mkdir %1

copy README %1
copy COPYING %1
copy CHANGES %1

copy img\sanos.vmdk %1
echo qemu -hda sanos.vmdk -boot c -redir tcp:2323::23 -redir tcp:8080::80 -L . -no-kqemu > %1\runsanos.cmd

copy tools\qemu\bios.bin %1
copy tools\qemu\qemu.exe %1
copy tools\qemu\sdl.dll %1
copy tools\qemu\fmod.dll %1
copy tools\qemu\vgabios-cirrus.bin %1
