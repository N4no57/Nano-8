cmake_minimum_required(VERSION 3.30)
project(emulator C)

set(CMAKE_C_STANDARD 11)

include_directories(dependencies/raylib/headers)
link_directories(dependencies/raylib/lib)

add_executable(emulator src/main.c
        include/flags.h
        src/cpu.c include/cpu.h
        src/memory.c include/memory.h
        src/emulatorTests.c include/emulatorTests.h
        src/floppyDiskController.c include/floppyDiskController.h
        src/pmio.c include/pmio.h
        src/videoCard.c include/videoCard.h
)

target_link_libraries(${PROJECT_NAME}
        raylib
        opengl32
        gdi32
        winmm
)