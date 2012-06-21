project(msc)

cmake_minimum_required(VERSION 2.8)

add_executable(msc ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp partiallypersistent/DoublyLinkedList.cpp partiallypersistent/Node.cpp main.cpp)

add_subdirectory(partiallypersistent)
add_subdirectory(ephemeral)