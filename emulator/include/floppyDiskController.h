//
// Created by Bernardo on 03/10/2025.
//

#ifndef FLOPPYDISKCONTROLLER_H
#define FLOPPYDISKCONTROLLER_H

#include <stdint.h>

typedef struct FloppyDisk {
    int size;
    int heads;
    int tracks;
    int sectors;
    int sector_size;
    uint8_t *data;
} FloppyDisk;

typedef struct FDC {
    uint8_t MSR;
    uint8_t FIFO[512];
    uint8_t DOR;
    uint8_t DIR;
    int fifo_len, fifo_pos;
    int ticks_remaining;
    int current_track, current_sector, current_head, current_drive;
    FloppyDisk *floppy_disks[4];
} FDC;

int mount_floppy_disk(FDC *controller, FloppyDisk *floppy_disk, int drive_number);

#endif //FLOPPYDISKCONTROLLER_H
