//
// Created by Bernardo on 03/10/2025.
//

#include "../include/floppyDiskController.h"

#define CMD_NONE 0
#define CMD_READ 1
#define CMD_WRITE 2
#define CMD_RECALIBRATE 3
#define CMD_SEEK 4

#define BYTES_PER_REV 1

int current_offset = 0;

int mount_floppy_disk(FDC *controller, FloppyDisk *floppy_disk, const int drive_number) {
    if (drive_number != -1 && drive_number < 4) {
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

void write_FDC(FDC *controller, int value, int port) {
    switch (port) {
        case 1: // data FIFO
            if (controller->current_command == CMD_NONE) {
                // interpret as command input
                controller->current_command = value;
                controller->MSR |= 0b00010000;
                controller->fifo_len = controller->fifo_pos = 0;
                controller->phase = PHASE_ARGS;
            } else {
                // command args or data
                if (controller->current_command == CMD_READ && controller->args_recieved == 0) {
                    controller->current_track = value;
                    controller->args_expected = 2;
                    controller->args_recieved = 1;
                } else if (controller->current_command == CMD_READ && controller->args_recieved == 1) {
                    controller->current_sector = value;
                    int sectors_per_track = controller->floppy_disks[controller->current_drive]->sectors;
                    current_offset = (controller->current_track*sectors_per_track + (controller->current_sector-1))*512;
                    controller->ticks_remaining = 10000; // placeholder
                    controller->byte_ticks = 1000; // placeholder
                }
            }
    }
}

uint8_t read_FDC();

void tick_fdc(FDC *controller) {
    if (controller->seek_ticks > 0) {
        if (--controller->seek_ticks == 0) controller->MSR &= ~0b00010000;
        return;
    }

    if (controller->motor_ticks > 0) {
        if (--controller->motor_ticks == 0) controller->current_drive = controller->MSR & 0b00000011;
        return;
    }

    controller->rotation_pos = (controller->rotation_pos + 1)
    % (controller->floppy_disks[controller->current_drive]->sectors
        * controller->floppy_disks[controller->current_drive]->tracks);

    if (controller->MSR & 16 && controller->ticks_remaining > 0) {
        controller->ticks_remaining--;

        if (--controller->byte_timer == 0) {
            controller->byte_timer = controller->byte_ticks;

        }

        if (controller->ticks_remaining == 0) {
            // TODO complete command
        }
    }
}
