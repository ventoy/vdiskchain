#!/bin/bash

if [ -f "$1" ]; then
    ventoytgz="$1"
    vttar=$(basename $1)
else
    ventoytgz=$(ls ../INSTALL/ventoy-*.tar.gz)
    vttar=$(basename $ventoytgz)
    echo "use $vttar ..."
fi

tgz=${vttar%-linux.tar.gz}
if [ -z "$tgz" ]; then
    echo "usage: update_image.sh ventoy-1.0.32-linux.tar.gz"
    exit 1
fi

tar xf "$ventoytgz" -C ./

[ -d ./Image ] && rm -rf ./Image
mkdir Image
mkdir Image/ventoy

cp -a $tgz/boot/boot.img Image/
cp -a $tgz/boot/core.img.xz Image/
xzcat $tgz/ventoy/ventoy.disk.img.xz > Image/ventoy.disk.img

rm -rf "$tgz"

tmp=$(mktemp -d)
mount Image/ventoy.disk.img $tmp

mkdir -p Image/ventoy/EFI/BOOT/
mkdir -p Image/ventoy/grub/fonts
mkdir -p Image/ventoy/tool
mkdir -p Image/ventoy/ventoy

cp -a $tmp/EFI/BOOT/grubx64_real.efi Image/ventoy/EFI/BOOT/BOOTX64.EFI
cp -a $tmp/grub/fonts/ascii.pf2 Image/ventoy/grub/fonts/unicode.pf2
cp -a $tmp/grub/x86_64-efi Image/ventoy/grub/
cp -a $tmp/grub/i386-pc Image/ventoy/grub/
cp -a $tmp/grub/*.cfg Image/ventoy/grub/

echo 123 > Image/ventoy/ventoy/ventoy.cpio
echo 123 > Image/ventoy/tool/mount.exfat-fuse_aarch64

for i in ipxe.krn ventoy_x64.efi; do
    cp -a $tmp/ventoy/$i   Image/ventoy/ventoy/
done

umount $tmp
rm -rf $tmp

rm -f Image/ventoy.disk.img

echo -e "\n============ success ==============\n"

