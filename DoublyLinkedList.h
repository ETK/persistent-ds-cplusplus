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


#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H
#include "Node.h"

#include <iostream>
#include <utility>
#include <vector>

class DoublyLinkedList {

public:
  std::size_t version;

  DoublyLinkedList ();
  virtual ~ DoublyLinkedList ();

  std::pair < std::size_t, Node * >insert (Node & new_node);
  std::pair < std::size_t, Node * >set_field (Node & node,
                                              field_name_t
                                              field_name, void *value);

  const std::vector < std::pair < std::size_t, Node * >>&get_heads ();

  void print_at_version (std::size_t v);
  void print_dot_graph (std::size_t v);
private:
    std::pair < std::size_t, Node * >modify_field (Node &, field_name_t,
                                                   void *);
    Node & copy_live_node (Node & node);

    std::vector < std::pair < std::size_t, Node * >>heads;
};

#endif // DOUBLYLINKEDLIST_H
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
