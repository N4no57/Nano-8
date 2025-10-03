cmake_minimum_required(VERSION 3.30)
project(emulator C)

set(CMAKE_C_STANDARD 11)

add_executable(emulator src/main.c
        src/cpu.c
        include/cpu.h
        include/flags.h
        src/memory.c
        include/memory.h
        src/emulatorTests.c
        include/emulatorTests.h
        src/floppyDiskController.c
        include/floppyDiskController.h
)