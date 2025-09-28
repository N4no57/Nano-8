cmake_minimum_required(VERSION 3.30)
project(assembler C)

set(CMAKE_C_STANDARD 11)

add_executable(assembler src/main.c
        src/symbolTable.c include/symbolTable.h
        src/instructionDef.c include/instructionDef.h
        src/tokeniser.c include/tokeniser.h
        src/utils.c include/utils.h
        src/objectFileWriter.c include/objectFileWriter.h
        src/segments.c include/segments.h
        src/assembler.c include/assembler.h
)