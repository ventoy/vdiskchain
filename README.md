# vdiskchain
For Ventoy Linux vDisk, please refer: [https://www.ventoy.net/en/plugin_vtoyboot.html](https://www.ventoy.net/en/plugin_vtoyboot.html)

This project is used to chainload Ventoy Linux vDisk file from other bootloaders(e.g. grub4dos/grub2/rEFInd/...)

The vDisk file can be in any disk and any partition only if it is in one of FAT32/NTFS/exFAT/XFS/Ext2/Ext3/Ext4/UDF filesystem.

# usage
## i386-pc
```
grub4dos:
kernel /ipxe.krn vdisk=/MyVdiskDir/Deepin.vdi.vtoy
initrd /vdiskchain

GRUB2:
linux16  /ipxe.krn vdisk=/MyVdiskDir/Deepin.vdi.vtoy
initrd16 /vdiskchain

```

## x86-64-efi
```
grub2:
chainloader /vdiskchain vdisk=/MyVdiskDir/Deepin.vdi.vtoy

rEFInd:
loader /vdiskchain vdisk=/MyVdiskDir/Deepin.vdi.vtoy

Systemd-boot:
efi /vdiskchain vdisk=/MyVdiskDir/Deepin.vdi.vtoy
```

# known issue
1. For i386-pc, the bootloader(grub4dos/grub2) must be booted from disk(not cdrom), or it will chainload fail.

