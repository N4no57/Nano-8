//
// Created by Bernardo on 03/10/2025.
//

#include "../include/floppyDiskController.h"

#define CMD_NONE 0
#define CMD_READ 1
#define CMD_WRITE 2
#define CMD_RECALIBRATE 3
#define CMD_SEEK 4

int mount_floppy_disk(FDC *controller, FloppyDisk *floppy_disk, const int drive_number) {
    if (drive_number != -1) {
        if (controller->floppy_disks[drive_number] == NULL) {
            controller->floppy_disks[drive_number] = floppy_disk;
            return 0; // success
        }
        return -2; // drive slot in use
    }

    for (int i = 0; i < 4; i++) {
        if (controller->floppy_disks[i] == NULL) {
            controller->floppy_disks[i] = floppy_disk;
            return 0; // success
        }
    }
    return -1; // no available drive slots
}