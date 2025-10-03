//
// Created by Bernardo on 03/10/2025.
//

#include "../include/floppyDiskController.h"

#define CMD_NONE 0
#define CMD_READ 1
#define CMD_WRITE 2
#define CMD_RECALIBRATE 3
#define CMD_SEEK 4

#define MSR_BUSY 0b10000000
#define MSR_DRQ 0b00100000

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
            if (controller->phase == PHASE_CMD) {
                // interpret as command input
                controller->current_command = value;
                controller->MSR |= MSR_BUSY;
                controller->fifo_len = controller->fifo_pos = 0;
                controller->phase = PHASE_ARGS;
                if (value == CMD_READ || value == CMD_WRITE) {
                    controller->args_expected = 4;
                } else if (value == CMD_SEEK) {
                    controller->args_expected = 2;
                } else {
                    controller->args_expected = 0;
                }
            } else if (controller->phase == PHASE_ARGS && controller->args_received < controller->args_expected) {
                if (controller->current_command == CMD_READ) {
                    if (controller->args_received == 0) controller->current_drive = value;  // drive
                    if (controller->args_received == 1) controller->current_head = value;   // head
                    if (controller->args_received == 2) { // track
                        if (controller->current_track != value) {
                            controller->seek_ticks = 100; // placeholder
                        }
                        controller->current_track = value;
                    }
                    if (controller->args_received == 3) { // sector
                        controller->current_sector = value;
                        controller->MSR |= MSR_BUSY;
                        controller->current_offset = ((controller->current_track *
                            controller->floppy_disks[controller->current_drive]->heads + controller->current_head)
                            * controller->floppy_disks[controller->current_drive]->sectors + (controller->current_sector-1)) * 512;
                        controller->byte_timer = controller->byte_ticks = 11;
                        controller->ticks_remaining = controller->byte_ticks * 512; // hardcoded val for now
                        controller->phase = PHASE_EXEC;
                    }
                    controller->args_received += 1;
                } else if (controller->current_command == CMD_WRITE) {
                    if (controller->args_received == 0) controller->current_drive = value;  // drive
                    if (controller->args_received == 1) controller->current_head = value;   // head
                    if (controller->args_received == 2) { // track
                        if (controller->current_track != value) {
                            controller->seek_ticks = 100; // placeholder
                        }
                        controller->current_track = value;
                    }
                    if (controller->args_received == 3) { // sector
                        controller->current_sector = value;
                        controller->MSR |= MSR_BUSY;
                        controller->current_offset = ((controller->current_track *
                            controller->floppy_disks[controller->current_drive]->heads + controller->current_head)
                            * controller->floppy_disks[controller->current_drive]->sectors + (controller->current_sector-1)) * 512;
                        controller->byte_timer = controller->byte_ticks = 4;
                        controller->ticks_remaining = controller->byte_ticks * 512; // hardcoded val for now
                        controller->phase = PHASE_EXEC;
                    }
                    controller->args_received += 1;
                }
            } else if (controller->phase == PHASE_EXEC) {
                if (controller->fifo_len >= sizeof(controller->FIFO)) {
                    return;
                }

                int write_pos = (controller->fifo_pos + controller->fifo_len) % 16;
                controller->FIFO[write_pos] = value;
                controller->fifo_len++;

                controller->MSR |= MSR_DRQ;
            }
        break;
    }
}

uint8_t read_FDC(FDC *controller, int port) {
    switch (port) {
        case 0: // MSR
            return controller->MSR;
        case 1: // FIFO
            uint8_t value = controller->FIFO[controller->fifo_pos];
            controller->fifo_pos = (controller->fifo_pos + 1) % 16;
            controller->fifo_len--;
            if (controller->fifo_len == 0) controller->MSR &= ~MSR_DRQ;
            return value;
    }
}

void tick_fdc(FDC *controller) {

    if (controller->seek_ticks > 0) {
        if (--controller->seek_ticks == 0) controller->MSR &= ~MSR_BUSY;
        return;
    }

    if (controller->motor_ticks > 0) {
        if (--controller->motor_ticks == 0) {
            // TODO figure out what happens
        }
        return;
    }

    controller->rotation_pos = (controller->rotation_pos + 1)
    % (controller->floppy_disks[controller->current_drive]->sectors
        * controller->floppy_disks[controller->current_drive]->tracks);

    if (controller->phase == PHASE_EXEC && controller->ticks_remaining > 0) {
        controller->ticks_remaining--;

        if (--controller->byte_timer == 0) {
            controller->byte_timer = controller->byte_ticks;

            if (controller->fifo_len < 16) {
                if (controller->current_command == CMD_READ) {
                    // Push one byte from disk into FIFO
                    int write_pos = (controller->fifo_pos + controller->fifo_len) % 16;

                    controller->FIFO[write_pos] =
                        controller->floppy_disks[controller->current_drive]->data[
                            controller->current_offset++];
                    controller->fifo_len++;
                    controller->MSR |= MSR_DRQ; // signal CPU
                } else if (controller->current_command == CMD_WRITE) {
                    // For write, wait for CPU to fill FIFO before consuming
                    if (controller->fifo_len > 0) {
                        controller->floppy_disks[controller->current_drive]->data[
                            controller->current_offset++] =
                            controller->FIFO[controller->fifo_pos++];
                        controller->fifo_len--;
                        if (controller->fifo_len < sizeof(controller->FIFO)) controller->MSR |= MSR_DRQ;
                    }
                }
            }
        }

        if (controller->ticks_remaining == 0) {
            controller->phase = PHASE_RESULT;
            controller->MSR &= ~MSR_BUSY;
            controller->set_int = 1;
            // TODO the rest of the thing maybe???
        }
    }
}
