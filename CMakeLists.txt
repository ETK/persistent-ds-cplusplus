project(msc)

cmake_minimum_required(VERSION 2.8)
SET(CMAKE_CXX_FLAGS "-std=c++0x")

add_executable(test_dot_graph partiallypersistent/DoublyLinkedList.cpp partiallypersistent/Node.cpp test_dot_graph.cpp)
add_executable(msc rollback_reorder_lazy/DoublyLinkedList.cpp rollback_reorder/DoublyLinkedList.cpp rollback_naive/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp partiallypersistent/DoublyLinkedList.cpp partiallypersistent/Node.cpp main.cpp)
target_link_libraries(msc rt sqlite3)

add_executable(test_rollback_naive rollback_naive/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp test_rollback_naive.cpp)
add_executable(test_rollback_snapshots rollback_naive/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp test_rollback_snapshots.cpp)
add_executable(test_rollback_reorder rollback_reorder_lazy/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp test_rollback_reorder.cpp)
add_executable(test_merge_reorder rollback_reorder/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp test_merge_reorder.cpp)

add_subdirectory(partiallypersistent)
add_subdirectory(ephemeral)
add_subdirectory(rollback_naive)
add_subdirectory(rollback_reorder)

add_subdirectory(rollback_reorder_lazy)