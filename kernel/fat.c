#include "fat.h"
#include "stdio.h"
#include "memdef.h"
#include "string.h"
#include "math.h"

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
    uint32_t VolumeId;
    uint8_t VolumeLabel[11];
    uint8_t SystemId[8];
} FAT_BootSector;
#pragma pack(pop)

#define SECTOR_SIZE 512
#define MAX_FILE_HANDLES 8
#define MAX_PATH_SIZE 256
#define ROOT_DIRECTORY_HANDLE -1

typedef struct {
    union {
        FAT_BootSector BootSector;
        uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;
    FAT_File OpenFiles[MAX_FILE_HANDLES];
} FAT_Data;

static FAT_Data far* g_Data;
static uint32_t g_RootDirLba;
static uint32_t g_RootDirSectors;
static uint8_t far* g_Fat = NULL;
static FAT_DirectoryEntry far* g_RootDirectory = NULL;
static FAT_File far* g_FileRootDirectory;
static uint32_t g_RootDirectoryEnd;

FAT_File far* g_OpenFATFiles[MAX_FILE_HANDLES];
bool g_FreeFATHandles[MAX_FILE_HANDLES];

uint32_t FAT_ClusterToLba(uint32_t cluster) {
    return (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster;
}

bool FAT_ReadBootSector(DISK* disk) {
    return DISK_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

bool FAT_ReadFat(DISK* disk) {
    //printf("Reserved sectors - %hu, sectors per fat: %hu - ", g_Data->BS.BootSector.ReservedSectors, g_Data->BS.BootSector.SectorsPerFat);
    return DISK_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors, g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

bool FAT_ReadRootDirectory(DISK* disk) {
    uint32_t lba = g_Data->BS.BootSector.ReservedSectors + g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;
    g_RootDirLba = lba;
    uint32_t size = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntriesCount;
    uint32_t sectors = (size / g_Data->BS.BootSector.BytesPerSector);
    if (size % g_Data->BS.BootSector.BytesPerSector > 0) {
        sectors++;
    }
    g_RootDirSectors = sectors;

    g_RootDirectoryEnd = lba + sectors; 
    return DISK_ReadSectors(disk, lba, sectors, g_RootDirectory);
}

bool FAT_Initialize(DISK* disk) {
    g_Data = (FAT_Data far*)MEMORY_FAT_ADDR;
    if (!FAT_ReadBootSector(disk)) {
        printf("FAT: boot sector read failed\r\n");
        return false;
    }
    //printf("BOOT SECATOR PARAMS:\r\n");
    //printf("Sectors per cluster: %hu\r\n", g_Data->BS.BootSector.SectorsPerCluster);

    //printf("Boot Sector read success - ");

    g_Fat = (uint8_t far*)g_Data + sizeof(FAT_Data);
    uint32_t fatSize = g_Data->BS.BootSector.BytesPerSector * g_Data->BS.BootSector.SectorsPerFat;
    if (sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE) {
        printf("FAT: not enough memory to read FAT\r\n");
        return false;
    }

    //printf("Fat table read start - ");

    if (!FAT_ReadFat(disk)) {
        printf("FAT: FAT read failed\r\n");
        return false;
    }
    //printf("Fat table read success - ");


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
    printf("Root directory read success\r\n");

    for (int i = 0; i < MAX_FILE_HANDLES; i++) {
        g_FreeFATHandles[i] = true;
    }

    g_FileRootDirectory = FAT_Open(disk, "ROOT    KRL");

    return true;
}

FAT_DirectoryEntry* FAT_FindFile(DISK* disk, const char* name) {
    /*for (uint32_t i = 0; i < g_Data->BS.BootSector.DirEntriesCount; i++) {
        printf("%s - ", g_RootDirectory[i].Name);
        if (memcmp(name, g_RootDirectory[i].Name, 11) == 0) {
            return &g_RootDirectory[i];
        }
    }*/
    FAT_DirectoryEntry entry;
    while (FAT_ReadFile(disk, g_FileRootDirectory, sizeof(FAT_DirectoryEntry), &entry)) {
        if (memcmp(name, entry.Name, 11) == 0) {
            return &entry;
        }
    }
    return NULL;
}

FAT_File far* FAT_Open(DISK* disk, const char* path) {
    FAT_DirectoryEntry* entry;
    if (memcmp(path, "ROOT    KRL", 11) == 0) {
        entry = g_RootDirectory;
    }
    else {
        //printf("Searching for file\r\n");
        entry = FAT_FindFile(disk, path);
    }

    if (entry == NULL) {
        printf("Error finding file: %s\r\n", path);
        return NULL;
    } 
    for (int i = 0; i < MAX_FILE_HANDLES; i++) {
        if (g_FreeFATHandles[i]) {
            FAT_File* file = &g_Data->OpenFiles[MAX_FILE_HANDLES];
            file->Handle = i;
            file->FirstCluster = entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
            file->Size = entry->FileSize;
            file->CurrentCluster = file->FirstCluster;
            file->CurrentSectorInCluster = 0;
            file->Position = 0;     
            g_OpenFATFiles[i] = file;
            g_FreeFATHandles[i] = false;

            if (!DISK_ReadSectors(disk, file->CurrentCluster, 1, file->Buffer)) {
                printf("Error reading newly opened file!\r\n");
                return NULL;
            }

            printf("File succesfully opened\r\n");
            return file;
        }
    }
    printf("FAILED TO OPEN %s\r\n", path);
    return NULL;
}

bool FAT_Close(FAT_File far* file) {
    g_OpenFATFiles[file->Handle] = NULL;
    g_FreeFATHandles[file->Handle] = true;

    return true;
}

bool FAT_ReadFile(DISK* disk, FAT_File far* file, uint32_t byteCount, void* outputBuffer) {
    bool ok = true;
    uint32_t currentCluster = file->CurrentCluster;

    uint8_t* u8out = (uint8_t*)outputBuffer;

    while (byteCount > 0) {
        uint32_t inBuffer = SECTOR_SIZE - (file->Position % SECTOR_SIZE);
        uint32_t take = min(byteCount, inBuffer);
        memcpy(u8out, file->Buffer + file->Position % SECTOR_SIZE, take); // Why adding module of sector size? TODO: Test failure cases without addition
        u8out += take;
        file->Position += take;
        byteCount -= take;

        if (inBuffer == take) {
        if (!DISK_ReadSectors(disk, file->CurrentCluster, 1, file->Buffer)) {
            printf("FAT_ReadFile ERROR\r\n");
            ok = false;
            break;
        }
        }
        file->CurrentCluster++;
    }
    return ok;
}