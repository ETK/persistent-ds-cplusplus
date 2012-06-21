project(msc)

cmake_minimum_required(VERSION 2.8)

add_executable(msc DoublyLinkedList.cpp Node.cpp main.cpp)

#add_definitions(-pg -g)
#
### Compiler flags
#if(CMAKE_COMPILER_IS_GNUCXX)
#    set(CMAKE_EXE_LINKER_FLAGS "-pg")
##    set(CMAKE_CXX_FLAGS "-std=gnu++0x")
#endif()
