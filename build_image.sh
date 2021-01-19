#!/bin/bash

rm -f vdiskchain
rm -f vdiskchain.img
sync

dd status=none if=/dev/zero of=vdiskchain.img bs=1M count=35

LOOP=$(losetup -f)
losetup -P $LOOP vdiskchain.img

echo LOOP=$LOOP

VENTOY_SECTOR_NUM=65536

sector_num=$(cat /sys/block/${LOOP#/dev/}/size)
part1_start_sector=2048 
part1_end_sector=$(expr $sector_num - $VENTOY_SECTOR_NUM - 1)

part2_start_sector=$(expr $part1_end_sector + 1)
part2_end_sector=$(expr $part2_start_sector + $VENTOY_SECTOR_NUM - 1)

echo "create partition ..."
parted -a none --script $LOOP \
    mklabel msdos \
    unit s \
    mkpart primary fat16 $part1_start_sector $part1_end_sector \
    mkpart primary fat16 $part2_start_sector $part2_end_sector \
    set 1 boot on \
    quit

sync
echo -en '\xEF' | dd status=none of=$LOOP conv=fsync bs=1 count=1 seek=466 > /dev/null 2>&1

udevadm trigger --name-match=$LOOP >/dev/null 2>&1
partprobe >/dev/null 2>&1
sleep 3
echo "Done"

echo "copy partition1 ..."

mkfs.vfat -n Ventoy ${LOOP}p1 > /dev/null 2>&1

[ -d ./tmpmnt ] && rm -rf ./tmpmnt
mkdir ./tmpmnt

if mount ${LOOP}p1 ./tmpmnt; then
    mkdir -p ./tmpmnt/ventoy
    cp -a ./Tool/ventoy.json ./tmpmnt/ventoy/
    cp -a ./Tool/ventoy_grub.cfg ./tmpmnt/ventoy/
    
    if umount ./tmpmnt; then
        rm -rf ./tmpmnt
    else
        echo "umount ${LOOP}p1 failed..."
    fi
else
    echo "mount ${LOOP}p1 failed..."
fi


echo "copy partition2 ..."
mkfs.vfat -F 16 -n VTOYEFI ${LOOP}p2 > /dev/null 2>&1
tmp=$(mktemp -d)

if mount ${LOOP}p2 $tmp; then
    cp -a Image/ventoy/* $tmp/
    
    if umount $tmp; then
        rm -rf $tmp
    else
        echo "umount ${LOOP}p2 failed..."
    fi
else
    echo "mount ${LOOP}p2 failed..."
fi

echo "install ventoy ..."
dd status=none conv=fsync if=./Image/boot.img of=$LOOP bs=1 count=446
xzcat ./Image/core.img.xz | dd status=none conv=fsync of=$LOOP bs=512 count=2047 seek=1
sync

losetup -d $LOOP

echo "compress image ..."
rm -f vdiskchain-tmp
./Tool/TianoCompress -e --uefi -o vdiskchain-tmp vdiskchain.img

cat  ./Tool/magic.bin vdiskchain-tmp > vdiskchain.byte
rm -f vdiskchain-tmp vdiskchain.img

gcc ./Tool/Raw2Code.c -o Raw2Code

./Raw2Code vdiskchain.byte 1.c
cat 1.c > ../EDKII/edk2-edk2-stable201911/MdeModulePkg/Application/VDiskChain/VDiskRawData.c
rm -f ./Raw2Code 1.c 
sync


cd ../EDKII
sh build.sh
cd -

echo "123" > ../EDKII/edk2-edk2-stable201911/MdeModulePkg/Application/VDiskChain/VDiskRawData.c

rm -f vdiskchain.byte
mv ./Tool/vdiskchain_x64.efi vdiskchain

sync

echo -e '\n\n============= SUCCESS =============\n'


