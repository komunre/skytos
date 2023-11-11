#include "stdint.h"
#include "stdio.h"
#include "disk.h"
#include "fat.h"

void _cdecl cstart_(uint16_t bootDrive) {
    printf("SKYTOS\r\n");
    printf("Disk init...\r\n");
    DISK disk;
    if (!DISK_Initialize(&disk, bootDrive)) {
        printf("DISK INIT ERROR\r\n");
        goto end;
    }
    if (!FAT_Initialize(&disk)) {
        printf("FAT INIT ERROR\r\n");
        goto end;
    }

    FAT_File far* fd = FAT_Open("TEST    TXT");
    uint8_t data[256];
    FAT_ReadFile(&disk, fd, &data);
    printf("%s\r\n", data);

    end:
    for (;;);
}
