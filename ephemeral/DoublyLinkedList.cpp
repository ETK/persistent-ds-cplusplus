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

#ifdef VERIFY_STRICT
#include <cassert>
#endif
#include <iostream>

#include "DoublyLinkedList.h"

using namespace std;

namespace ephemeral {
#ifdef VERIFY_STRICT
  size_t DoublyLinkedList::real_size() {
    size_t result = 0UL;
    Node* node = head;
    while (node) {
      ++result;
      node = node->next;
    }
    return result;
  }
#endif

  DoublyLinkedList::DoublyLinkedList () {
    head = 0;
    size = 0;
  };

  DoublyLinkedList::DoublyLinkedList (const DoublyLinkedList &
                                      other):head (0), size (0) {
    Node *node = other.head;
    while (node && node->next) {
      node = node->next;
    }
    while (node) {
      Node * new_node = new Node();
      new_node->data = node->data;
      insert (*new_node);
      node = node->prev;
    }
#ifdef VERIFY_STRICT
    assert (size == other.size);
#endif
  }

  void DoublyLinkedList::insert (Node & new_node) {
    if (head) {
      head->prev = &new_node;
      new_node.next = head;
    }
    head = &new_node;

    ++size;

#ifdef VERIFY_STRICT
    assert (real_size() == size);
#endif
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
#ifdef VERIFY_STRICT
    assert (real_size() == size);
#endif
  }

  void DoublyLinkedList::remove (Node & to_remove) {
    if (to_remove.prev) {
      to_remove.prev->next = to_remove.next;
    } else {
      head = to_remove.next;
    }
    if (to_remove.next) {
      to_remove.next->prev = to_remove.prev;
    }
    to_remove.next = 0;
    to_remove.prev = 0;
    --size;
#ifdef VERIFY_STRICT
    assert (real_size() == size);
#endif
  }

  void DoublyLinkedList::print () {
    Node *node = head;
    while (node) {
      cout << node->data << " ";
      if (node->next != node) {
        node = node->next;
      } else {
        break;
      }
    }
    cout << endl;
  }

}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
