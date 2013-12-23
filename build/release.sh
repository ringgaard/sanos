#!/bin/sh
#
# Build Sanos release
#

: ${VERSION:="latest"}
: ${DEPLOY:=0}

TAGNAME=v-${VERSION}
SRCNAME=sanos-src-${VERSION}
BINNAME=sanos-bin-${VERSION}
SDKNAME=sanos-sdk-${VERSION}

BUILDTYPE=DEBUG
if [ "${VERSION}" = "latest" ] ; then
  BUILDTYPE=PRERELEASE
else
  BUILDTYPE=RELEASE
fi

#
# Make source code release
#

mksrcrel() {
  mkdir -p $1 $1/build $1/src $1/sdk $1/sdk/src $1/utils

  cp README COPYING CHANGES Makefile Makefile.linux $1
  cp build/sanos.vcproj build/sanos*.sln $1/build
  cp build/*.inf $1/build
  cp build/bootdisk.lst build/minbootdisk.lst $1/build
  cp build/krnl.ini build/os.ini build/setup.ini build/sanos.dep $1/build

  cp -R src $1
  cp -R sdk/src/as $1/sdk/src
  cp -R sdk/src/cc $1/sdk/src

  cp -R utils/dfs $1/utils
  cp -R utils/dbggw $1/utils
  cp -R utils/mkpart $1/utils
  cp -R utils/mkfloppy $1/utils
  cp -R utils/sow $1/utils
  cp -R utils/mkpkg $1/utils
}

#
# Make binary release
#

mkbinrel() {
  mkdir -p $1 $1/win $1/win/install $1/win/tools $1/win/img
  mkdir -p $1/win/install/bin $1/win/install/boot $1/win/install/dev $1/win/install/etc $1/win/install/proc $1/win/install/usr $1/win/install/tmp 

  cp README COPYING CHANGES $1
  cp build/mkbootdisk.cmd $1
  cp win/install/bin/* $1/win/install/bin
  cp win/install/boot/* $1/win/install/boot
  cp win/install/etc/* $1/win/install/etc
  cp win/tools/mkdfs.exe $1/win/tools
  cp win/tools/mkfloppy.exe $1/win/tools
}

#
# Make SDK release
#

mksdkrel() {
  mkdir -p $1
  cp README COPYING CHANGES win/img/sanos.vmdk $1
  cp tools/qemu/bios.bin tools/qemu/qemu.exe tools/qemu/SDL.dll tools/qemu/fmod.dll tools/qemu/vgabios-cirrus.bin $1
  echo "qemu -hda sanos.vmdk -boot c -redir tcp:2323::23 -redir tcp:8080::80 -L . -no-kqemu" > $1/runsanos.cmd
}

#
# Make QEMU release
#

mkqemurel() {
  mkdir -p $1
  cp win/img/sanos.flp $1
  cp tools/qemu/bios.bin tools/qemu/qemu.exe tools/qemu/SDL.dll tools/qemu/fmod.dll tools/qemu/vgabios-cirrus.bin $1
  echo "qemu -fda sanos.flp -boot a -redir tcp:2323::23 -L . -no-kqemu"  > $1/runsanos.cmd
}

#
# Make ZIP file
#
mkzip() {
  rm -f $1.zip
  cd $1
  zip -X -r -q ../$1.zip *
  cd ..
}

# Build Sanos using Wine and MSVC
echo ==== Build Sanos
wine cmd /C "build\winebuild.cmd" ${BUILDTYPE} dirs tools sanos sdk sdkdisk bootdisk

# Build release directories
mkdir -p release
echo ==== Make source release
mksrcrel release/${SRCNAME}
echo ==== Make binary release
mkbinrel release/${BINNAME}
echo ==== Make sdk release
mksdkrel release/${SDKNAME}
echo ==== Make qemu release
mkqemurel release/sanos-qemu

# Generating source code HTML
echo ==== Generate source code HTML
mkdir -p release/htmlsrc
ctags -R -n --tag-relative -f src/ctags src
wine "win\tools\ctohtml" -d src -c "src\ctags" -o "release\htmlsrc" -l -t "sanos source"
rm src/ctags

# Zip release files
echo ==== Make zip files
cd release
mkzip ${SRCNAME}
mkzip ${BINNAME}
mkzip ${SDKNAME}
mkzip sanos-qemu
cd ..

# Deploy release
if [ "${DEPLOY}" = "1" ] ; then
  echo ==== Deploy release
  TARGET=/mnt/www
  cp release/${SRCNAME}.zip ${TARGET}/downloads
  cp release/${BINNAME}.zip ${TARGET}/downloads
  cp release/${SDKNAME}.zip ${TARGET}/downloads
  cp release/sanos-qemu.zip ${TARGET}/downloads
  cp -r release/htmlsrc/* ${TARGET}/sanos/source/
  cp CHANGES ${TARGET}/downloads
  cp win/img/sanos.flp ${TARGET}/sanos
  cp COPYING ${TARGET}/sanos/copying.txt
fi

echo ==== Done

