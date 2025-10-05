//
// Created by brego on 03/10/2025.
//

#ifndef PMIO_H
#define PMIO_H

#include "cpu.h"

void port_write(CPU *cpu, int port, int data);
uint8_t port_read(CPU *cpu, int port);

#endif //PMIO_H
