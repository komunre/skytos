#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
} __attribute__((packed)) BootSector;

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
} __attribute__((packed)) DirectoryEntry;


BootSector g_BootSector;
uint8_t* g_Fat;
DirectoryEntry* g_RootDirectory = NULL;
uint32_t g_RootDirectoryEnd = NULL;

uint8_t readBootSector(FILE* disk) {
    return fread(&g_BootSector, sizeof(g_BootSector), 1, disk);
}

uint8_t readSectors(FILE* disk, uint32_t lba, uint32_t count, void* bufferOut) {
    uint8_t ok = 1;
    ok = ok && (fseek(disk, lba * g_BootSector.BytesPerSector, SEEK_SET) == 0);
    ok = ok && (fread(bufferOut, g_BootSector.BytesPerSector, count, disk) == count);
    return ok;
}

uint8_t readFat(FILE* disk) {
    g_Fat = (uint8_t*)malloc(g_BootSector.SectorsPerFat * g_BootSector.BytesPerSector);
    return readSectors(disk, g_BootSector.ReservedSectors, g_BootSector.SectorsPerFat, g_Fat);
}

uint8_t readRootDirectory(FILE* disk) {
    uint32_t lba = g_BootSector.ReservedSectors + g_BootSector.SectorsPerFat * g_BootSector.FatCount;
    uint32_t size = sizeof(DirectoryEntry) * g_BootSector.DirEntriesCount;
    uint32_t sectors = (size / g_BootSector.BytesPerSector);
    if (size % g_BootSector.BytesPerSector > 0) {
        sectors++;
    }

    g_RootDirectoryEnd = lba + sectors; 
    g_RootDirectory = (DirectoryEntry*)malloc(sectors * g_BootSector.BytesPerSector);
    return readSectors(disk, lba, sectors, g_RootDirectory);
}

// DOES NOT COMPARE EXTENSION
DirectoryEntry* findFile(const char* name) {
    for (uint32_t i = 0; i < g_BootSector.DirEntriesCount; i++) {
        printf("Checking the file %s\n", g_RootDirectory[i].Name);
        if (memcmp(name, g_RootDirectory[i].Name, 11) == 0) {
            return &g_RootDirectory[i];
        }
    }

    return NULL;
}

uint8_t readFile(DirectoryEntry* entry, FILE* disk, uint8_t* outputBuffer) {
    uint8_t ok = 1;
    uint16_t currentCluster = entry->FirstClusterLow;

    do {
        uint32_t lba = g_RootDirectoryEnd + (currentCluster-2) * g_BootSector.SectorsPerCluster;
        ok = ok && readSectors(disk, lba, g_BootSector.SectorsPerCluster, outputBuffer);
        outputBuffer += g_BootSector.SectorsPerCluster * g_BootSector.BytesPerSector;
    
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

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("%s", "Incorrect argument count");
        return -1;
    }

    FILE* disk = fopen(argv[1], "rb");
    if(!disk) {
        fprintf(stderr, "Cannot open disk image %s!", argv[1]);
        return -1;
    }

    if (!readBootSector(disk)) {
        fprintf(stderr, "Unable to read bootsector in %s!", argv[1]);
        return -2;
    }

    if (!readFat(disk)) {
        fprintf(stderr, "Unable to read fat sectors in %s", argv[1]);
        free(g_Fat);
        return -3;
    }

    if (!readRootDirectory(disk)) {
        fprintf(stderr, "Unable to read root directory");
        return -4;
    }

    DirectoryEntry* fileEntry = findFile(argv[2]);
    if (!fileEntry) {
        fprintf(stderr, "Unable to find file %s", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        return -4;
    }

    printf("Results of search by %s\n", argv[2]);
    printf("Filename: %s\n", fileEntry->Name);
    uint8_t* buffer = (uint8_t*)malloc(fileEntry->FileSize + g_BootSector.BytesPerSector);
    if (!readFile(fileEntry, disk, buffer)) {
        fprintf(stderr, "Unable to read file %s", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        return -5;
    }
    //printf("Contents:\n%s\n", buffer);
    for (uint32_t i = 0; i < fileEntry->FileSize; i++) {
        if (isprint(buffer[i])) fputc(buffer[i], stdout);
        else printf("<%02x>", buffer[i]);
    }
    printf("\n");

    free(g_Fat);
    free(g_RootDirectory);

    return 0;
}