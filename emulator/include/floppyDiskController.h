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
    char *filename;
} FloppyDisk;

enum FDCPhase {
    PHASE_IDLE,
    PHASE_CMD,
    PHASE_ARGS,
    PHASE_EXEC,
    PHASE_RESULT
};

typedef struct FDC {
    uint8_t MSR;
    uint8_t FIFO[16];
    uint8_t DOR;
    uint8_t DIR;
    int fifo_len, fifo_pos;
    int ticks_remaining;
    int current_track, current_sector, current_head, current_drive;
    int current_command;
    FloppyDisk *floppy_disks[4];

    int rotation_pos;

    int motor_ticks, seek_ticks, byte_timer, byte_ticks;

    enum FDCPhase phase;
    int args_expected, args_received;

    int current_offset;
    uint8_t set_int;
} FDC;

int mount_floppy_disk(FDC *controller, FloppyDisk *floppy_disk, int drive_number);

void write_FDC(FDC *controller, int value, int port);
uint8_t read_FDC(FDC *controller, int port);

void tick_fdc(FDC *controller);

#endif //FLOPPYDISKCONTROLLER_H
