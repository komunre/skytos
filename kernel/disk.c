#include "disk.h"
#include "x86.h"

bool DISK_Initialize(DISK* disk, uint8_t driveNumber) {
    uint8_t driveType;
    uint16_t cylinders, sectors,heads;

    disk->id = driveNumber;
    if (!x86_Disk_GetDriveParams(disk->id, &driveType, &cylinders, &sectors, &heads)) {
        return false;
    }
    disk->cylinders = cylinders;
    disk->heads = heads;
    disk->sectors = sectors;

    return true;
}

void DISK_LBA2CHS(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* sectorOut, uint16_t* headOut) {
    *sectorOut = lba % disk->sectors + 1;
    *cylinderOut = (lba / disk->sectors) / disk->heads;
    *headOut = (lba / disk->sectors) % disk->heads;
}

bool DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t count, void far* dataOut) {
    uint16_t cylinder, sector, head;

    DISK_LBA2CHS(disk, lba, &cylinder, &sector, &head);

    for (int i = 0; i < 5; i++) {
        if (x86_Disk_Read(disk, cylinder, head, sector, count, dataOut)) return true;
        x86_Disk_Reset(disk->id);
    }

    return false;
}