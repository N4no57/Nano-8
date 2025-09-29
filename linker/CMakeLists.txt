cmake_minimum_required(VERSION 3.30)
project(linker C)

set(CMAKE_C_STANDARD 11)

add_executable(nano8-ld src/main.c
        src/objectFileReader.c include/objectFileReader.h
        src/linker.c include/linker.h
)