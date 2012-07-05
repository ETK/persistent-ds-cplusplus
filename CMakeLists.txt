project(msc)

cmake_minimum_required(VERSION 2.8)

add_executable(test_dot_graph partiallypersistent/DoublyLinkedList.cpp partiallypersistent/Node.cpp test_dot_graph.cpp)
add_executable(msc rollback_naive/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp partiallypersistent/DoublyLinkedList.cpp partiallypersistent/Node.cpp main.cpp)

add_executable(test_rollback_naive rollback_naive/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp test_rollback_naive.cpp)

add_subdirectory(partiallypersistent)
add_subdirectory(ephemeral)
add_subdirectory(rollback_naive)