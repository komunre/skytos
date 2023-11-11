#include "fat.h"
#include "stdio.h"
#include "memdef.h"
#include "string.h"

#pragma pack(push, 1)
typedef struct 
{
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntriesCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;
    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint8_t VolumeId[4];
    uint8_t VolumeLabel[11];
    uint8_t SystemId[8];
} FAT_BootSector;
#pragma pack(pop)

#define SECTOR_SIZE 512

typedef struct {
    union {
        FAT_BootSector BootSector;
        uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;
    FAT_File OpenFiles[4];
} FAT_Data;

static FAT_Data far* g_Data;
static uint8_t* g_Fat = NULL;
static FAT_DirectoryEntry* g_RootDirectory = NULL;
static uint32_t g_RootDirectoryEnd = NULL;

FAT_DirectoryEntry* g_OpenFATFiles[4];
bool g_FreeFATHandles[4] = {true, true, true, true};

bool FAT_ReadBootSector(DISK* disk) {
    return DISK_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

bool FAT_ReadFat(DISK* disk) {
    return DISK_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors, g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

bool FAT_ReadRootDirectory(DISK* disk) {
    uint32_t lba = g_Data->BS.BootSector.ReservedSectors + g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;
    uint32_t size = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntriesCount;
    uint32_t sectors = (size / g_Data->BS.BootSector.BytesPerSector);
    if (size % g_Data->BS.BootSector.BytesPerSector > 0) {
        sectors++;
    }

    g_RootDirectoryEnd = lba + sectors; 
    return DISK_ReadSectors(disk, lba, sectors, g_RootDirectory);
}

bool FAT_Initialize(DISK* disk) {
    g_Data = (FAT_Data far*)MEMORY_FAT_ADDR;
    if (!FAT_ReadBootSector(disk)) {
        printf("FAT: boot sector read failed\r\n");
        return false;
    }

    g_Fat = (uint8_t far*)g_Data + sizeof(FAT_Data);
    uint32_t fatSize = g_Data->BS.BootSector.BytesPerSector * g_Data->BS.BootSector.SectorsPerFat;
    if (sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE) {
        printf("FAT: not enough memory to read FAT\r\n");
        return false;
    }

    if (!FAT_ReadFat(disk)) {
        printf("FAT: FAT read failed\r\n");
        return false;
    }


    g_RootDirectory = (FAT_DirectoryEntry far*)(g_Fat + fatSize);
    uint32_t rootDirSize = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntriesCount;
    if (sizeof(FAT_Data) + fatSize + rootDirSize >= MEMORY_FAT_SIZE) {
        printf("FAT: not enough memory to read root directory\r\n");
        return false;
    }

    if (!FAT_ReadRootDirectory(disk)) {
        printf("FAT: failed to read root directory");
        return false;
    }

    return true;
}

FAT_DirectoryEntry* FAT_FindFile(const char* name) {
    for (uint32_t i = 0; i < g_Data->BS.BootSector.DirEntriesCount; i++) {
        if (memcmp(name, g_RootDirectory[i].Name, 11) == 0) {
            return &g_RootDirectory[i];
        }
    }
    return NULL;
}

FAT_File far* FAT_Open(const char* path) {
    FAT_DirectoryEntry* entry = FAT_FindFile(path);
    for (int i = 0; i < 4; i++) {
        if (g_FreeFATHandles[i]) {
            FAT_File* file = &g_Data->OpenFiles[4];
            file->Handle = i;
            g_OpenFATFiles[i] = entry;
            g_FreeFATHandles[i] = false;
            return file;
        }
    }
    return NULL;
}

bool FAT_Close(FAT_File far* file) {
    g_OpenFATFiles[file->Handle] = NULL;
    g_FreeFATHandles[file->Handle] = true;

    return true;
}

bool FAT_ReadFile(DISK* disk, FAT_File* file, uint8_t* outputBuffer) {
    bool ok = true;
    uint16_t currentCluster = g_OpenFATFiles[file->Handle]->FirstClusterLow;

    do {
        uint32_t lba = g_RootDirectoryEnd + (currentCluster-2) * g_Data->BS.BootSector.SectorsPerCluster;
        ok = ok && DISK_ReadSectors(disk, lba, g_Data->BS.BootSector.SectorsPerCluster, outputBuffer);
        outputBuffer += g_Data->BS.BootSector.SectorsPerCluster * g_Data->BS.BootSector.BytesPerSector;

        uint32_t fatIndex = currentCluster * 3 / 2;
        if (currentCluster % 2 == 0) {
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) & 0x0FFF;
        }
        else {
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) >> 4;
        }
    } while (ok && currentCluster < 0x0FF8);
    return ok;
}