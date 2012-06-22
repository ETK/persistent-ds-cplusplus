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

// #include <iostream>

#include "DoublyLinkedList.h"

using namespace std;

namespace rollback_naive {
  DoublyLinkedList::DoublyLinkedList () {
    ephemeral_current = ephemeral::DoublyLinkedList ();
    next_record_index = 0ul;
    records = vector < record_t > ();
  };

  void DoublyLinkedList::insert (size_t node_data) {
    if (records.size () > 0) {
      while (next_record_index < records.size () - 1) {
        rollforward ();
      }
    }

    record_t record;
    record.operation = INSERT;
    record.index = 0;
    record.data = node_data;
    records.push_back (record);
    rollforward ();
  }

  void DoublyLinkedList::insert (size_t node_data, std::size_t index) {
    record_t record;
    record.operation = INSERT;
    record.index = index;
    record.data = node_data;
    records.push_back (record);
    rollforward ();
  }

  void DoublyLinkedList::modify_data (std::size_t index, std::size_t value) {
    ephemeral::Node * node = ephemeral_current.head;
    for (size_t i = 0; i < index; ++i) {
      if (node->next) {
        node = node->next;
      } else {
        throw "Index too large!";
      }
    }
    record_t record;
    record.operation = MODIFY;
    record.old_data = node->data;
    record.data = value;
    record.index = index;
    records.push_back (record);
    rollforward ();
  }

  void DoublyLinkedList::remove (std::size_t index) {
    ephemeral::Node * node = ephemeral_current.head;
    for (size_t i = 0; i < index; ++i) {
      if (node->next) {
        node = node->next;
      } else {
        throw "Index too large!";
      }
    }

    record_t record;
    record.operation = REMOVE;
    record.index = index;
    records.push_back (record);
    rollforward ();
  }

  void DoublyLinkedList::rollback () {
    record_t record = records[next_record_index - 1];

    ephemeral::Node * node = ephemeral_current.head;
    for (size_t i = 0; i < record.index; ++i) {
      if (node->next) {
        node = node->next;
      } else {
        throw "Index too large!";
      }
    }

    switch (record.operation) {
    case INSERT:
      ephemeral_current.remove (*node);
      delete node;
      break;
    case REMOVE:{
        ephemeral::Node * new_node = new ephemeral::Node ();
        new_node->data = record.data;
        ephemeral_current.insert (*new_node, record.index);
        break;
      }
    case MODIFY:
      node->data = record.old_data;
      break;
    }
    next_record_index--;
  }

  void DoublyLinkedList::rollforward () {
    record_t record = records[next_record_index];

    ephemeral::Node * node = ephemeral_current.head;
    for (size_t i = 0; i < record.index; ++i) {
      if (node->next) {
        node = node->next;
      }
    }

    switch (record.operation) {
    case INSERT:{
        ephemeral::Node * new_node = new ephemeral::Node ();
        new_node->data = record.data;
        ephemeral_current.insert (*new_node, record.index);
        break;
      }
    case REMOVE:
      ephemeral_current.remove (*node);
      delete node;
      break;
    case MODIFY:
      node->data = record.data;
      break;
    }
    next_record_index++;
  }

  void DoublyLinkedList::print_at_version (std::size_t v) {
    while (v > next_record_index) {
      rollforward ();
    }

    while (v < next_record_index) {
      rollback ();
    }

    ephemeral_current.print ();
  }

}

// kate: indent-mode cstyle; indent-width 2; replace-tabs on; ;
