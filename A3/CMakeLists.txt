cmake_minimum_required(VERSION 3.2)
project(A3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(GECODE_LIBS
        -lgecodeflatzinc
        -lgecodedriver
        -lgecodegist
        -lgecodesearch
        -lgecodeminimodel
        -lgecodeset
        -lgecodefloat
        -lgecodeint
        -lgecodekernel
        -lgecodesupport)

set(SQUARE_FILES
        square.cpp)

add_executable(square ${SQUARE_FILES})
target_link_libraries(square ${GECODE_LIBS})
