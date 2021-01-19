#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    int i, Len;
    char *buf;
    FILE *fIn, *fOut;

    fIn = fopen(argv[1], "rb");
    
    fseek(fIn, 0, SEEK_END);
    Len = (int)ftell(fIn);
    fseek(fIn, 0, SEEK_SET);

    buf = (char *)malloc(Len);

    fread(buf, 1, Len, fIn);
    fclose(fIn);

    fOut = fopen(argv[2], "w+");

    fprintf(fOut, "#include <Uefi.h>\n");
    
    fprintf(fOut, "UINT8 gVDiskRawData[] = {\n");
    for (i = 0; i < Len; i++)
    {
        fprintf(fOut, "0x%02x, ", (unsigned char)(buf[i]));
        if ((i + 1) % 16 == 0)
        {
            fprintf(fOut, "\n");
        }
    }
    fprintf(fOut, "};\n\n");
    
    fprintf(fOut, "int vdisk_get_vdisk_raw(UINT8 **buf, UINT32 *size)\n");
    fprintf(fOut, "{\n");
    fprintf(fOut, "    *buf = gVDiskRawData;\n");
    fprintf(fOut, "    *size = (UINT32)sizeof(gVDiskRawData);\n");
    fprintf(fOut, "    return 0;\n");
    fprintf(fOut, "}\n");

    free(buf);
    fclose(fOut);

    return 0;
}

