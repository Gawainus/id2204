cmake_minimum_required(VERSION 3.2)
project(A4)

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
      
set(SQUARE_FILES
  square.cpp)

set(LIFE_FILES
  life.cpp)

add_executable(square ${SQUARE_FILES})
target_link_libraries(square ${GECODE_LIBS})

add_executable(life ${LIFE_FILES})
target_link_libraries(life ${GECODE_LIBS} Qt4::QtGui)
