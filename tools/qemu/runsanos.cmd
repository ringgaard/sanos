qemu -hda ..\..\img\sanos.vmdk -boot c -redir tcp:2323::23 -redir tcp:8080::80 -L . -no-kqemu
