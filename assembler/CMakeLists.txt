cmake_minimum_required(VERSION 3.30)
project(assembler C)

set(CMAKE_C_STANDARD 11)

add_executable(assembler src/main.c
        src/symbolTable.c include/symbolTable.h
)