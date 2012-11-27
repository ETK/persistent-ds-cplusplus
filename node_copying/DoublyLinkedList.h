/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef PARTIALLYPERSISTENT_DOUBLYLINKEDLIST_H
#define PARTIALLYPERSISTENT_DOUBLYLINKEDLIST_H
#include "Node.h"
#include "../AbstractDoublyLinkedList.h"

#include <utility>
#include <vector>
#include <cstdlib>
#include <ostream>

// #define EXTRA_ASSERTS

namespace node_copying
{

  class DoublyLinkedList: public AbstractDoublyLinkedList
  {

  public:
#ifdef MEASURE_SPACE
    std::size_t space = 0UL;
#endif

    struct version_info_t {
      Node* head;
      size_t size;
    };

    std::size_t version;

    DoublyLinkedList ();

    std::pair < std::size_t, Node* >insert (std::size_t data,
                                            std::size_t index);

    /// removes the specified node from the next version
    version_info_t remove (Node* to_remove);

    std::pair < std::size_t, Node* >set_data (Node* node, size_t value);

    const std::vector < version_info_t > &get_versions ();

    Node* head () const;
    Node* head_at (std::size_t v) const;

    std::size_t size () const;
    std::size_t size_at (std::size_t v) const;

    std::size_t print_at_version (std::size_t v);
    void print_dot_graph (std::size_t v, std::ofstream& out);

    const std::size_t a_access (const std::size_t version,
                                const std::size_t index);
    void a_insert (const std::size_t index, const std::size_t value);
    void a_modify (const std::size_t index, const std::size_t value);
    void a_remove (const std::size_t index);
    const std::size_t a_size ();
    const std::size_t a_size_at (const std::size_t version);
    const std::size_t a_num_versions ();
    void a_print_at (std::size_t version);

  private:
    Node* modify_field (node_copying::Node* node,
                        field_name_t field_name,
                        node_copying::Node* value);
    Node* modify_field (node_copying::Node* node,
                        field_name_t field_name,
                        node_copying::Node* value,
                        node_copying::Node * &head);
    void copy_live_node (node_copying::Node* node,
                         node_copying::Node* copy,
                         field_name_t field_name, Node* value,
                         node_copying::Node * &head);

    std::vector < version_info_t > versions;
  };
}
#endif                          // PARTIALLYPERSISTENT_DOUBLYLINKEDLIST_H
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;




