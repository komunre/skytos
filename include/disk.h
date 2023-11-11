#pragma once

#include "stdint.h"
#include "x86.h"

typedef struct {
    uint8_t id;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
} DISK;

bool _cdecl DISK_Initialize(DISK* disk, uint8_t driveNumber);
bool _cdecl DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, void far* dataOut);