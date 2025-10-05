//
// Created by brego on 05/10/2025.
//

#ifndef VIDEOCARD_H
#define VIDEOCARD_H

#include <stdint.h>
#include "raylib.h"

#define PAGE_SIZE 0x400
#define RGB_WIDTH 6

typedef struct VideoCard {
    // stuff exposed to the cpu
    uint8_t *VRAMpage;
    uint8_t indexRegister;
    uint8_t modeRegister;
    uint8_t palleteIndexRegister;
    uint8_t palleteDataRegister;
    uint8_t statusRegister;
    uint8_t controlRegister;

    // internal hardware crap
    uint8_t VRAM[8][PAGE_SIZE]; // pages * page_size = total memory
    uint8_t palleteRAM[64 * RGB_WIDTH];
    uint8_t horizontalCounter; // frame counters
    uint8_t verticalCounter;
    int width, height;
    int hBlank, vBlank;
} VideoCard;

int videoCardInit(VideoCard *videoCard);

int videoCardTick(VideoCard *videoCard);

#endif //VIDEOCARD_H
