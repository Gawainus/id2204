cmake_minimum_required(VERSION 3.2)
project(A3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

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

# check version with "qmake --version"
find_package(Qt4)
      
set(NO_OVERLAP_FILES
        no-overlap.cpp)

add_executable(no-overlap ${NO_OVERLAP_FILES})
target_link_libraries(no-overlap ${GECODE_LIBS})

set(SQUARE_FILES
        square.cpp)

add_executable(square ${SQUARE_FILES})
target_link_libraries(square ${GECODE_LIBS})

#add_executable(queens ${QUEENS_FILES})
#target_link_libraries(queens ${GECODE_LIBS} Qt4::QtGui)
