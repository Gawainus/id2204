cmake_minimum_required(VERSION 3.2)
project(send_more_money)

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

set(SEND_MORE_MONEY_FILES
        send-more-money.cpp)
set(SUDOKU_FILES
        sudoku.cpp)

add_executable(send_more_money ${SEND_MORE_MONEY_FILES})
target_link_libraries(send_more_money ${GECODE_LIBS})

add_executable(sudoku ${SUDOKU_FILES})
target_link_libraries(sudoku ${GECODE_LIBS})