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


#ifndef EPHEMERAL_DOUBLYLINKEDLIST_H
#define EPHEMERAL_DOUBLYLINKEDLIST_H

#include <utility>

#include "Node.h"

// #define VERIFY_STRICT
#undef VERIFY_STRICT

namespace ephemeral {

  class DoublyLinkedList {

  public:
    DoublyLinkedList ();
    DoublyLinkedList (const DoublyLinkedList& other);
    void insert (Node & new_node);
    void insert (Node & new_node, std::size_t index);
    void remove (Node & to_remove);
    void print ();

    Node *head;
    std::size_t size;
  private:
#ifdef VERIFY_STRICT
    std::size_t real_size();
#endif
  };
}
#endif                          // EPHEMERAL_DOUBLYLINKEDLIST_H
// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
