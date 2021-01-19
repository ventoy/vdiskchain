
FILE_LICENCE ( GPL2_OR_LATER_OR_UBDL );

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <byteswap.h>
#include <errno.h>
#include <assert.h>
#include <ipxe/blockdev.h>
#include <ipxe/io.h>
#include <ipxe/acpi.h>
#include <ipxe/sanboot.h>
#include <ipxe/device.h>
#include <ipxe/pci.h>
#include <ipxe/eltorito.h>
#include <ipxe/timer.h>
#include <ipxe/umalloc.h>
#include <ipxe/image.h>
#include <realmode.h>
#include <bios.h>
#include <biosint.h>
#include <bootsector.h>
#include <int13.h>
#include <ventoy.h>

int g_debug = 0;
int g_bios_disk80 = 0;
char *g_cmdline_copy;
void *g_initrd_addr;
size_t g_initrd_len;

void *g_disk_buf_addr;
size_t g_disk_buf_len;

int ventoy_vdisk_read(struct san_device *sandev, uint64_t lba, unsigned int count, unsigned long buffer)
{
    if (INT13_EXTENDED_READ != sandev->int13_command)
    {
        DBGC(sandev, "invalid cmd %u\n", sandev->int13_command);
        return 0;
    }

    memcpy((char *)buffer, (char *)g_disk_buf_addr + lba * 512, count * 512);
    return 0;
}

int ventoy_find_vdisk_offset(uint8_t *buf)
{
    int i;
    uint8_t magic[32];
    
    for (i = 0; i < 16; i++)
    {
        magic[i] = 0xA0 + i;
        magic[i + 16] = 0xB0 + i;
    }

    for (i = 0; i < 2 * 1024 * 1024; i++)
    {
        if (*(uint32_t *)(buf + i) == 0xA3A2A1A0)
        {
            if (memcmp(buf + i, magic, sizeof(magic)) == 0)
            {
                return i + sizeof(magic);
            }
        }
    }

    return 0;
}

int ventoy_patch_vdisk_path(char *pos)
{
    int j;
    size_t i;
    char *end;
    char *buf = (char *)g_disk_buf_addr;
    
    if (*pos == '\"')
    {
        pos++;
    }

    end = strstr(pos, ".vtoy");
    end += 5;//string length
    
    for (i = 0; i < g_disk_buf_len; i++)
    {
        if (*(uint32_t *)(buf + i) == 0x59595959)
        {
            for (j = 0; j < 300; j++)
            {
                if (buf[i + j] != 'Y')
                {
                    break;
                }
            }

            if (j >= 300)
            {
                break; 
            }
        }
    }

    if (i >= g_disk_buf_len)
    {
        if (g_debug)
        {
            printf("No need to fill vdisk path\n");
        }
        return 0;
    }

    if (g_debug)
    {
        printf("Fill vdisk path at %d\n", i);        
    }

    while (pos != end)
    {
        buf[i++] = *pos++;
    }

    buf[i++] = '\"';
    
    while (buf[i] == 'Y' || buf[i] == '\"')
    {
        buf[i] = ' ';
        i++;
    }   

    return 0;    
}

int ventoy_boot_vdisk(void *data)
{
    int offset;
    uint32_t len;
    uint32_t ScratchSize;
    unsigned int drive;
    void *vdiskpos;
    void *Scratch;
    char *diskbuf;
    struct image *image;
    
    (void)data;

    if (strstr(g_cmdline_copy, "debug"))
    {
        g_debug = 1;
        printf("### ventoy chain boot begin... ###\n");
        printf("cmdline: <%s>\n", g_cmdline_copy);
        ventoy_debug_pause();
    }
    
    if (strstr(g_cmdline_copy, "bios80"))
    {
        g_bios_disk80 = 1;
    }

    vdiskpos = strstr(g_cmdline_copy, "vdisk=");
    if (vdiskpos == NULL || strstr(vdiskpos, ".vtoy") == NULL)
    {
        printf("\n\nError: vdisk parameter not found!\n");
        ventoy_debug_pause();
        return 1;
    }

    image = find_image("<INITRD>");
    if (!image)
    {
        printf("vdiskchain not found\n");
        ventoy_debug_pause();
    }

    offset = ventoy_find_vdisk_offset((uint8_t *)(image->data));
    diskbuf = (char *)(image->data) + offset;

    EfiGetInfo(diskbuf, image->len - offset, &len, &ScratchSize);
    Scratch = malloc(ScratchSize);

    g_disk_buf_len = len;
    g_disk_buf_addr = (void *)umalloc(len);
    EfiDecompress(diskbuf, image->len, g_disk_buf_addr, len, Scratch, ScratchSize);
    free(Scratch);

    if (g_debug)
    {
        printf("vdiskchain orginal len: %lu, offset:%d decompress len:%lu\n",
            (unsigned long)image->len, offset, (unsigned long)len);
    }
    
    ventoy_patch_vdisk_path(vdiskpos + 6);

    drive = ventoy_int13_hook(len);

    if (g_debug)
    {
        printf("### ventoy chain boot before boot image ... ###\n");
        ventoy_debug_pause();    
    }

    g_initrd_addr = (char *)(image->data);
    g_initrd_len = (size_t)len;    

    ventoy_int13_boot(drive);

    if (g_debug)
    {
        printf("!!!!!!!!!! ventoy boot failed !!!!!!!!!!\n");
        ventoy_debug_pause();
    }

    return 0;
}

