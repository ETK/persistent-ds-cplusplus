project(msc)

cmake_minimum_required(VERSION 2.8)
SET(CMAKE_CXX_FLAGS "-std=c++0x")

#add_executable(test_dot_graph partiallypersistent/DoublyLinkedList.cpp partiallypersistent/Node.cpp test_dot_graph.cpp)
add_executable(msc AbstractDoublyLinkedList.cpp rollback/AbstractRollbackDoublyLinkedList.cpp rollback/eliminate_reorder/DoublyLinkedList.cpp rollback/reorder/DoublyLinkedList.cpp rollback/blackbox/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp partiallypersistent/DoublyLinkedList.cpp partiallypersistent/Node.cpp main.cpp)
target_link_libraries(msc rt sqlite3)
#add_executable(rollback_reorder_lazy_worstcase AbstractDoublyLinkedList.cpp rollback_reorder_lazy/DoublyLinkedList.cpp rollback/blackbox/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp partiallypersistent/DoublyLinkedList.cpp partiallypersistent/Node.cpp rollback_reorder_lazy_worstcase.cpp)
#target_link_libraries(rollback_reorder_lazy_worstcase rt sqlite3)

#add_executable(test_rollback_reorder rollback/blackbox/DoublyLinkedList.cpp ephemeral/Node.cpp ephemeral/DoublyLinkedList.cpp test_rollback_reorder.cpp)

#add_subdirectory(partiallypersistent)
#add_subdirectory(ephemeral)


#add_subdirectory(rollback_reorder_only_lazy)
#add_subdirectory(rollback)