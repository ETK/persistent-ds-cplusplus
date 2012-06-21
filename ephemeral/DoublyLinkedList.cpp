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


#include "DoublyLinkedList.h"

namespace ephemeral {
  DoublyLinkedList::DoublyLinkedList () {
    head = 0;
    size = 0;
  }
  
  void DoublyLinkedList::insert (Node & new_node) {
    if (!head) {
      head = &new_node;
    } else {
      head->prev = &new_node;
      new_node.next = head;
    }
    ++size;
  }

  void DoublyLinkedList::insert (Node & new_node, std::size_t index) {
    if (index == 0 || !head) {
      insert (new_node);
    } else {
      Node *before = head;
      for (size_t i = 0; i < index; ++i) {
        if (before->next) {
          before = before->next;
        } else {
          break;
        }
      }
      new_node.prev = before;
      new_node.next = before->next;
      if (before->next) {
        before->next->prev = &new_node;
      }
      before->next = &new_node;
      ++size;
    }
  }

  void DoublyLinkedList::remove (Node & to_remove) {
    if (to_remove.prev) {
      to_remove.prev->next = to_remove.next;
    }
    if (to_remove.next) {
      to_remove.next->prev = to_remove.prev;
    }
    --size;
  }
}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
