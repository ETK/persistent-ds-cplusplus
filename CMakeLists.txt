project(msc)

add_executable(msc DoublyLinkedList.cpp Node.cpp main.cpp)

## Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=gnu++0x")
endif()