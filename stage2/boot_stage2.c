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
    printf("Disk init success\r\n");
    if (!FAT_Initialize(&disk)) {
        printf("FAT INIT ERROR\r\n");
        goto end;
    }
    printf("Fat init success\r\n");

    FAT_File far* fd = FAT_Open(&disk, "TEST    TXT");
    uint8_t data[512];
    if (!FAT_ReadFile(&disk, fd, 512, &data)) {
        //printf("FAILED TO READ TEST FILE\r\n");
        goto end;
    }
    printf("Contents: \r\n");
    int i = 0;
    uint8_t* fmt = &data[i];
    while (*fmt) {
        printf("%hhx ", *fmt);
        fmt++;
    }

    end:
    for (;;);
}
