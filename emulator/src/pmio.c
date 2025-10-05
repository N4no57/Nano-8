//
// Created by brego on 03/10/2025.
//

#include "../include/pmio.h"
#include "../include/floppyDiskController.h"

void port_write(CPU *cpu, int port, int data) {
    if (port < 3 && port > 0) {
        write_FDC(&cpu->floppy_controller, data, port-1);
    } else {
        cpu->ports[port] = data;
    }
}

uint8_t port_read(CPU *cpu, int port) {
    if (port < 3 && port > 0) {
        return read_FDC(&cpu->floppy_controller, port-1);
    }
    return cpu->ports[port];
}