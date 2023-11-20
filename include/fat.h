#pragma once 

#include "stdint.h"
#include "disk.h"

#pragma pack(push, 1)

typedef struct 
{
    uint8_t Name[11]; // 3 last bytes are extention
    uint8_t Attributes;
    uint8_t _Reserved;
    uint8_t CreateTime;
    uint16_t CreateTimeExtended;
    uint16_t CreateDate;
    uint16_t AccessDate;
    uint16_t FirstClusterHigh;
    uint16_t LastModifiedTime;
    uint16_t LastModifiedDate;
    uint16_t FirstClusterLow;
    uint32_t FileSize;
} FAT_DirectoryEntry;

#pragma pack(pop)

typedef struct {
    uint8_t Buffer[512];
    int Handle;
    bool IsDirectory;
    uint32_t FirstCluster;
    uint32_t CurrentCluster;
    uint32_t CurrentSectorInCluster;

    uint32_t Position;
    uint32_t Size;
} FAT_File;

bool FAT_Initialize(DISK* disk);
FAT_File far* FAT_Open(DISK* disk, const char* path);
bool FAT_Close(FAT_File far* file);
bool FAT_ReadFile(DISK* disk, FAT_File far* file, uint32_t byteCount, void* outputBuffer);
//uint32_t FAT_Read(DISK* disk, FAT_File far* file, uint32_t byteCount, void* dataOut);
//bool FAT_ReadEntry(DISK* disk, FAT_File far* file, FAT_DirectoryEntry* dataOut);