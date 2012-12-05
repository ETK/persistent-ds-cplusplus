project(msc)

cmake_minimum_required(VERSION 2.8)
SET(CMAKE_CXX_FLAGS "-std=c++0x")

add_executable(test_dot_graph node_copying/DoublyLinkedList.cpp node_copying/Node.cpp test_dot_graph.cpp)
add_executable(msc AbstractDoublyLinkedList.cpp rollback/AbstractRollbackDoublyLinkedList.cpp rollback/eliminate_reorder/DoublyLinkedList.cpp rollback/reorder/DoublyLinkedList.cpp rollback/blackbox/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp node_copying/DoublyLinkedList.cpp node_copying/Node.cpp main.cpp)
target_link_libraries(msc rt sqlite3)

add_executable(test_rollback_blackbox AbstractDoublyLinkedList.cpp rollback/AbstractRollbackDoublyLinkedList.cpp rollback/blackbox/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp test_rollback_blackbox.cpp)

add_executable(test_rollback_reorder AbstractDoublyLinkedList.cpp rollback/AbstractRollbackDoublyLinkedList.cpp rollback/reorder/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp test_rollback_reorder.cpp)

add_executable(test_rollback_eliminate_reorder AbstractDoublyLinkedList.cpp rollback/AbstractRollbackDoublyLinkedList.cpp rollback/eliminate_reorder/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp test_rollback_eliminate_reorder.cpp)

#add_subdirectory(node_copying)
#add_subdirectory(ephemeral)


#add_subdirectory(rollback_reorder_only_lazy)
#add_subdirectory(rollback)